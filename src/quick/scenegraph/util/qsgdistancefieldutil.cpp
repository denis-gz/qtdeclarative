/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsgdistancefieldutil_p.h"

#include <qmath.h>
#include <private/qsgpathsimplifier_p.h>
#include <private/qsgadaptationlayer_p.h>
#include <QtGui/private/qopenglengineshadersource_p.h>
#include <QtQuick/private/qsgcontext_p.h>

QT_BEGIN_NAMESPACE

static float defaultThresholdFunc(float glyphScale)
{
    static float base = qgetenv("QT_DF_BASE").isEmpty() ? 0.5f : qgetenv("QT_DF_BASE").toFloat();
    static float baseDev = qgetenv("QT_DF_BASEDEVIATION").isEmpty() ? 0.065f : qgetenv("QT_DF_BASEDEVIATION").toFloat();
    static float devScaleMin = qgetenv("QT_DF_SCALEFORMAXDEV").isEmpty() ? 0.15f : qgetenv("QT_DF_SCALEFORMAXDEV").toFloat();
    static float devScaleMax = qgetenv("QT_DF_SCALEFORNODEV").isEmpty() ? 0.3f : qgetenv("QT_DF_SCALEFORNODEV").toFloat();
    return base - ((qBound(devScaleMin, glyphScale, devScaleMax) - devScaleMin) / (devScaleMax - devScaleMin) * -baseDev + baseDev);
}

static float defaultAntialiasingSpreadFunc(float glyphScale)
{
    static float range = qgetenv("QT_DF_RANGE").isEmpty() ? 0.06f : qgetenv("QT_DF_RANGE").toFloat();
    return range / glyphScale;
}

namespace
{
    enum FillHDir
    {
        LeftToRight,
        RightToLeft
    };

    enum FillVDir
    {
        TopDown,
        BottomUp
    };

    enum FillClip
    {
        NoClip,
        Clip
    };
}

template <FillClip clip, FillHDir dir>
inline void fillLine(qint32 *, int, int, int, qint32, qint32)
{
}

template <>
inline void fillLine<Clip, LeftToRight>(qint32 *line, int width, int lx, int rx, qint32 d, qint32 dd)
{
    int fromX = qMax(0, lx >> 8);
    int toX = qMin(width, rx >> 8);
    int x = toX - fromX;
    if (x <= 0)
        return;
    qint32 val = d + (((fromX << 8) + 0xff - lx) * dd >> 8);
    line += fromX;
    do {
        *line = abs(val) < abs(*line) ? val : *line;
        val += dd;
        ++line;
    } while (--x);
}

template <>
inline void fillLine<Clip, RightToLeft>(qint32 *line, int width, int lx, int rx, qint32 d, qint32 dd)
{
    int fromX = qMax(0, lx >> 8);
    int toX = qMin(width, rx >> 8);
    int x = toX - fromX;
    if (x <= 0)
        return;
    qint32 val = d + (((toX << 8) + 0xff - rx) * dd >> 8);
    line += toX;
    do {
        val -= dd;
        --line;
        *line = abs(val) < abs(*line) ? val : *line;
    } while (--x);
}

template <>
inline void fillLine<NoClip, LeftToRight>(qint32 *line, int, int lx, int rx, qint32 d, qint32 dd)
{
    int fromX = lx >> 8;
    int toX = rx >> 8;
    int x = toX - fromX;
    if (x <= 0)
        return;
    qint32 val = d + ((~lx & 0xff) * dd >> 8);
    line += fromX;
    do {
        *line = abs(val) < abs(*line) ? val : *line;
        val += dd;
        ++line;
    } while (--x);
}

template <>
inline void fillLine<NoClip, RightToLeft>(qint32 *line, int, int lx, int rx, qint32 d, qint32 dd)
{
    int fromX = lx >> 8;
    int toX = rx >> 8;
    int x = toX - fromX;
    if (x <= 0)
        return;
    qint32 val = d + ((~rx & 0xff) * dd >> 8);
    line += toX;
    do {
        val -= dd;
        --line;
        *line = abs(val) < abs(*line) ? val : *line;
    } while (--x);
}

template <FillClip clip, FillVDir vDir, FillHDir hDir>
inline void fillLines(qint32 *bits, int width, int height, int upperY, int lowerY,
                      int &lx, int ldx, int &rx, int rdx, qint32 &d, qint32 ddy, qint32 ddx)
{
    Q_UNUSED(height);
    Q_ASSERT(upperY < lowerY);
    int y = lowerY - upperY;
    if (vDir == TopDown) {
        qint32 *line = bits + upperY * width;
        do {
            fillLine<clip, hDir>(line, width, lx, rx, d, ddx);
            lx += ldx;
            d += ddy;
            rx += rdx;
            line += width;
        } while (--y);
    } else {
        qint32 *line = bits + lowerY * width;
        do {
            lx -= ldx;
            d -= ddy;
            rx -= rdx;
            line -= width;
            fillLine<clip, hDir>(line, width, lx, rx, d, ddx);
        } while (--y);
    }
}

template <FillClip clip>
void drawTriangle(qint32 *bits, int width, int height, const QPoint *center,
                  const QPoint *v1, const QPoint *v2, qint32 value)
{
    const int y1 = clip == Clip ? qBound(0, v1->y() >> 8, height) : v1->y() >> 8;
    const int y2 = clip == Clip ? qBound(0, v2->y() >> 8, height) : v2->y() >> 8;
    const int yC = clip == Clip ? qBound(0, center->y() >> 8, height) : center->y() >> 8;

    const int v1Frac = clip == Clip ? (y1 << 8) + 0xff - v1->y() : ~v2->y() & 0xff;
    const int v2Frac = clip == Clip ? (y2 << 8) + 0xff - v2->y() : ~v1->y() & 0xff;
    const int centerFrac = clip == Clip ? (yC << 8) + 0xff - center->y() : ~center->y() & 0xff;

    int dx1 = 0, x1 = 0, dx2 = 0, x2 = 0;
    qint32 dd1, d1, dd2, d2;
    if (v1->y() != center->y()) {
        dx1 = ((v1->x() - center->x()) << 8) / (v1->y() - center->y());
        x1 = center->x() + centerFrac * (v1->x() - center->x()) / (v1->y() - center->y());
    }
    if (v2->y() != center->y()) {
        dx2 = ((v2->x() - center->x()) << 8) / (v2->y() - center->y());
        x2 = center->x() + centerFrac * (v2->x() - center->x()) / (v2->y() - center->y());
    }

    const qint32 div = (v2->x() - center->x()) * (v1->y() - center->y())
                     - (v2->y() - center->y()) * (v1->x() - center->x());
    const qint32 dd = div ? qint32((qint64(value * (v1->y() - v2->y())) << 8) / div) : 0;

    if (y2 < yC) {
        if (y1 < yC) {
            // Center at the bottom.
            if (y2 < y1) {
                // y2 < y1 < yC
                // Long right edge.
                d1 = centerFrac * value / (v1->y() - center->y());
                dd1 = ((value << 8) / (v1->y() - center->y()));
                fillLines<clip, BottomUp, LeftToRight>(bits, width, height, y1, yC, x1, dx1,
                                                       x2, dx2, d1, dd1, dd);
                dx1 = ((v1->x() - v2->x()) << 8) / (v1->y() - v2->y());
                x1 = v1->x() + v1Frac * (v1->x() - v2->x()) / (v1->y() - v2->y());
                fillLines<clip, BottomUp, LeftToRight>(bits, width, height, y2, y1, x1, dx1,
                                                       x2, dx2, value, 0, dd);
            } else {
                // y1 <= y2 < yC
                // Long left edge.
                d2 = centerFrac * value / (v2->y() - center->y());
                dd2 = ((value << 8) / (v2->y() - center->y()));
                fillLines<clip, BottomUp, RightToLeft>(bits, width, height, y2, yC, x1, dx1,
                                                       x2, dx2, d2, dd2, dd);
                if (y1 != y2) {
                    dx2 = ((v1->x() - v2->x()) << 8) / (v1->y() - v2->y());
                    x2 = v2->x() + v2Frac * (v1->x() - v2->x()) / (v1->y() - v2->y());
                    fillLines<clip, BottomUp, RightToLeft>(bits, width, height, y1, y2, x1, dx1,
                                                           x2, dx2, value, 0, dd);
                }
            }
        } else {
            // y2 < yC <= y1
            // Center to the right.
            int dx = ((v1->x() - v2->x()) << 8) / (v1->y() - v2->y());
            int xUp, xDn;
            xUp = xDn = v2->x() + (clip == Clip ? (yC << 8) + 0xff - v2->y()
                                                : (center->y() | 0xff) - v2->y())
                        * (v1->x() - v2->x()) / (v1->y() - v2->y());
            fillLines<clip, BottomUp, LeftToRight>(bits, width, height, y2, yC, xUp, dx,
                                                   x2, dx2, value, 0, dd);
            if (yC != y1)
                fillLines<clip, TopDown, LeftToRight>(bits, width, height, yC, y1, xDn, dx,
                                                      x1, dx1, value, 0, dd);
        }
    } else {
        if (y1 < yC) {
            // y1 < yC <= y2
            // Center to the left.
            int dx = ((v1->x() - v2->x()) << 8) / (v1->y() - v2->y());
            int xUp, xDn;
            xUp = xDn = v1->x() + (clip == Clip ? (yC << 8) + 0xff - v1->y()
                                                : (center->y() | 0xff) - v1->y())
                        * (v1->x() - v2->x()) / (v1->y() - v2->y());
            fillLines<clip, BottomUp, RightToLeft>(bits, width, height, y1, yC, x1, dx1,
                                                   xUp, dx, value, 0, dd);
            if (yC != y2)
                fillLines<clip, TopDown, RightToLeft>(bits, width, height, yC, y2, x2, dx2,
                                                      xDn, dx, value, 0, dd);
        } else {
            // Center at the top.
            if (y2 < y1) {
                // yC <= y2 < y1
                // Long right edge.
                if (yC != y2) {
                    d2 = centerFrac * value / (v2->y() - center->y());
                    dd2 = ((value << 8) / (v2->y() - center->y()));
                    fillLines<clip, TopDown, LeftToRight>(bits, width, height, yC, y2, x2, dx2,
                                                          x1, dx1, d2, dd2, dd);
                }
                dx2 = ((v1->x() - v2->x()) << 8) / (v1->y() - v2->y());
                x2 = v2->x() + v2Frac * (v1->x() - v2->x()) / (v1->y() - v2->y());
                fillLines<clip, TopDown, LeftToRight>(bits, width, height, y2, y1, x2, dx2,
                                                      x1, dx1, value, 0, dd);
            } else {
                // Long left edge.
                // yC <= y1 <= y2
                if (yC != y1) {
                    d1 = centerFrac * value / (v1->y() - center->y());
                    dd1 = ((value << 8) / (v1->y() - center->y()));
                    fillLines<clip, TopDown, RightToLeft>(bits, width, height, yC, y1, x2, dx2,
                                                          x1, dx1, d1, dd1, dd);
                }
                if (y1 != y2) {
                    dx1 = ((v1->x() - v2->x()) << 8) / (v1->y() - v2->y());
                    x1 = v1->x() + v1Frac * (v1->x() - v2->x()) / (v1->y() - v2->y());
                    fillLines<clip, TopDown, RightToLeft>(bits, width, height, y1, y2, x2, dx2,
                                                          x1, dx1, value, 0, dd);
                }
            }
        }
    }
}

template <FillClip clip>
void drawRectangle(qint32 *bits, int width, int height,
                   const QPoint *int1, const QPoint *center1, const QPoint *ext1,
                   const QPoint *int2, const QPoint *center2, const QPoint *ext2,
                   qint32 extValue)
{
    if (center1->y() > center2->y()) {
        qSwap(center1, center2);
        qSwap(int1, ext2);
        qSwap(ext1, int2);
        extValue = -extValue;
    }

    Q_ASSERT(ext1->x() - center1->x() == center1->x() - int1->x());
    Q_ASSERT(ext1->y() - center1->y() == center1->y() - int1->y());
    Q_ASSERT(ext2->x() - center2->x() == center2->x() - int2->x());
    Q_ASSERT(ext2->y() - center2->y() == center2->y() - int2->y());

    const int yc1 = clip == Clip ? qBound(0, center1->y() >> 8, height) : center1->y() >> 8;
    const int yc2 = clip == Clip ? qBound(0, center2->y() >> 8, height) : center2->y() >> 8;
    const int yi1 = clip == Clip ? qBound(0, int1->y() >> 8, height) : int1->y() >> 8;
    const int yi2 = clip == Clip ? qBound(0, int2->y() >> 8, height) : int2->y() >> 8;
    const int ye1 = clip == Clip ? qBound(0, ext1->y() >> 8, height) : ext1->y() >> 8;
    const int ye2 = clip == Clip ? qBound(0, ext2->y() >> 8, height) : ext2->y() >> 8;

    const int center1Frac = clip == Clip ? (yc1 << 8) + 0xff - center1->y() : ~center1->y() & 0xff;
    const int center2Frac = clip == Clip ? (yc2 << 8) + 0xff - center2->y() : ~center2->y() & 0xff;
    const int int1Frac = clip == Clip ? (yi1 << 8) + 0xff - int1->y() : ~int1->y() & 0xff;
    const int ext1Frac = clip == Clip ? (ye1 << 8) + 0xff - ext1->y() : ~ext1->y() & 0xff;

    int dxC = 0, dxE = 0; // cap slope, edge slope
    qint32 ddC = 0;
    if (ext1->y() != int1->y()) {
        dxC = ((ext1->x() - int1->x()) << 8) / (ext1->y() - int1->y());
        ddC = (extValue << 9) / (ext1->y() - int1->y());
    }
    if (ext1->y() != ext2->y())
        dxE = ((ext1->x() - ext2->x()) << 8) / (ext1->y() - ext2->y());

    const qint32 div = (ext1->x() - int1->x()) * (ext2->y() - int1->y())
                     - (ext1->y() - int1->y()) * (ext2->x() - int1->x());
    const qint32 dd = div ? qint32((qint64(extValue * (ext2->y() - ext1->y())) << 9) / div) : 0;

    int xe1, xe2, xc1, xc2;
    qint32 d;

    qint32 intValue = -extValue;

    if (center2->x() < center1->x()) {
        // Leaning to the right. '/'
        if (int1->y() < ext2->y()) {
            // Mostly vertical.
            Q_ASSERT(ext1->y() != ext2->y());
            xe1 = ext1->x() + ext1Frac * (ext1->x() - ext2->x()) / (ext1->y() - ext2->y());
            xe2 = int1->x() + int1Frac * (ext1->x() - ext2->x()) / (ext1->y() - ext2->y());
            if (ye1 != yi1) {
                xc2 = center1->x() + center1Frac * (ext1->x() - int1->x()) / (ext1->y() - int1->y());
                xc2 += (ye1 - yc1) * dxC;
                fillLines<clip, TopDown, LeftToRight>(bits, width, height, ye1, yi1, xe1, dxE,
                                                      xc2, dxC, extValue, 0, dd);
            }
            if (yi1 != ye2)
                fillLines<clip, TopDown, LeftToRight>(bits, width, height, yi1, ye2, xe1, dxE,
                                                      xe2, dxE, extValue, 0, dd);
            if (ye2 != yi2) {
                xc1 = center2->x() + center2Frac * (ext1->x() - int1->x()) / (ext1->y() - int1->y());
                xc1 += (ye2 - yc2) * dxC;
                fillLines<clip, TopDown, RightToLeft>(bits, width, height, ye2, yi2, xc1, dxC,
                                                      xe2, dxE, intValue, 0, dd);
            }
        } else {
            // Mostly horizontal.
            Q_ASSERT(ext1->y() != int1->y());
            xc1 = center2->x() + center2Frac * (ext1->x() - int1->x()) / (ext1->y() - int1->y());
            xc2 = center1->x() + center1Frac * (ext1->x() - int1->x()) / (ext1->y() - int1->y());
            xc1 += (ye2 - yc2) * dxC;
            xc2 += (ye1 - yc1) * dxC;
            if (ye1 != ye2) {
                xe1 = ext1->x() + ext1Frac * (ext1->x() - ext2->x()) / (ext1->y() - ext2->y());
                fillLines<clip, TopDown, LeftToRight>(bits, width, height, ye1, ye2, xe1, dxE,
                                                      xc2, dxC, extValue, 0, dd);
            }
            if (ye2 != yi1) {
                d = (clip == Clip ? (ye2 << 8) + 0xff - center2->y()
                                  : (ext2->y() | 0xff) - center2->y())
                    * 2 * extValue / (ext1->y() - int1->y());
                fillLines<clip, TopDown, LeftToRight>(bits, width, height, ye2, yi1, xc1, dxC,
                                                      xc2, dxC, d, ddC, dd);
            }
            if (yi1 != yi2) {
                xe2 = int1->x() + int1Frac * (ext1->x() - ext2->x()) / (ext1->y() - ext2->y());
                fillLines<clip, TopDown, RightToLeft>(bits, width, height, yi1, yi2, xc1, dxC,
                                                      xe2, dxE, intValue, 0, dd);
            }
        }
    } else {
        // Leaning to the left. '\'
        if (ext1->y() < int2->y()) {
            // Mostly vertical.
            Q_ASSERT(ext1->y() != ext2->y());
            xe1 = ext1->x() + ext1Frac * (ext1->x() - ext2->x()) / (ext1->y() - ext2->y());
            xe2 = int1->x() + int1Frac * (ext1->x() - ext2->x()) / (ext1->y() - ext2->y());
            if (yi1 != ye1) {
                xc1 = center1->x() + center1Frac * (ext1->x() - int1->x()) / (ext1->y() - int1->y());
                xc1 += (yi1 - yc1) * dxC;
                fillLines<clip, TopDown, RightToLeft>(bits, width, height, yi1, ye1, xc1, dxC,
                                                      xe2, dxE, intValue, 0, dd);
            }
            if (ye1 != yi2)
                fillLines<clip, TopDown, RightToLeft>(bits, width, height, ye1, yi2, xe1, dxE,
                                                      xe2, dxE, intValue, 0, dd);
            if (yi2 != ye2) {
                xc2 = center2->x() + center2Frac * (ext1->x() - int1->x()) / (ext1->y() - int1->y());
                xc2 += (yi2 - yc2) * dxC;
                fillLines<clip, TopDown, LeftToRight>(bits, width, height, yi2, ye2, xe1, dxE,
                                                      xc2, dxC, extValue, 0, dd);
            }
        } else {
            // Mostly horizontal.
            Q_ASSERT(ext1->y() != int1->y());
            xc1 = center1->x() + center1Frac * (ext1->x() - int1->x()) / (ext1->y() - int1->y());
            xc2 = center2->x() + center2Frac * (ext1->x() - int1->x()) / (ext1->y() - int1->y());
            xc1 += (yi1 - yc1) * dxC;
            xc2 += (yi2 - yc2) * dxC;
            if (yi1 != yi2) {
                xe2 = int1->x() + int1Frac * (ext1->x() - ext2->x()) / (ext1->y() - ext2->y());
                fillLines<clip, TopDown, RightToLeft>(bits, width, height, yi1, yi2, xc1, dxC,
                                                      xe2, dxE, intValue, 0, dd);
            }
            if (yi2 != ye1) {
                d = (clip == Clip ? (yi2 << 8) + 0xff - center2->y()
                                  : (int2->y() | 0xff) - center2->y())
                    * 2 * extValue / (ext1->y() - int1->y());
                fillLines<clip, TopDown, RightToLeft>(bits, width, height, yi2, ye1, xc1, dxC,
                                                      xc2, dxC, d, ddC, dd);
            }
            if (ye1 != ye2) {
                xe1 = ext1->x() + ext1Frac * (ext1->x() - ext2->x()) / (ext1->y() - ext2->y());
                fillLines<clip, TopDown, LeftToRight>(bits, width, height, ye1, ye2, xe1, dxE,
                                                      xc2, dxC, extValue, 0, dd);
            }
        }
    }
}

static void drawPolygons(qint32 *bits, int width, int height, const QPoint *vertices,
                         const quint32 *indices, int indexCount, qint32 value)
{
    Q_ASSERT(indexCount != 0);
    Q_ASSERT(height <= 128);
    QVarLengthArray<quint8, 16> scans[128];
    int first = 0;
    for (int i = 1; i < indexCount; ++i) {
        quint32 idx1 = indices[i - 1];
        quint32 idx2 = indices[i];
        Q_ASSERT(idx1 != quint32(-1));
        if (idx2 == quint32(-1)) {
            idx2 = indices[first];
            Q_ASSERT(idx2 != quint32(-1));
            first = ++i;
        }
        const QPoint *v1 = &vertices[idx1];
        const QPoint *v2 = &vertices[idx2];
        if (v2->y() < v1->y())
            qSwap(v1, v2);
        int fromY = qMax(0, v1->y() >> 8);
        int toY = qMin(height, v2->y() >> 8);
        if (fromY >= toY)
            continue;
        int dx = ((v2->x() - v1->x()) << 8) / (v2->y() - v1->y());
        int x = v1->x() + ((fromY << 8) + 0xff - v1->y()) * (v2->x() - v1->x()) / (v2->y() - v1->y());
        for (int y = fromY; y < toY; ++y) {
            quint32 c = quint32(x >> 8);
            if (c < quint32(width))
                scans[y].append(quint8(c));
            x += dx;
        }
    }
    for (int i = 0; i < height; ++i) {
        quint8 *scanline = scans[i].data();
        int size = scans[i].size();
        for (int j = 1; j < size; ++j) {
            int k = j;
            quint8 value = scanline[k];
            for (; k != 0 && value < scanline[k - 1]; --k)
                scanline[k] = scanline[k - 1];
            scanline[k] = value;
        }
        qint32 *line = bits + i * width;
        int j = 0;
        for (; j + 1 < size; j += 2) {
            for (quint8 x = scanline[j]; x < scanline[j + 1]; ++x)
                line[x] = value;
        }
        if (j < size) {
            for (int x = scanline[j]; x < width; ++x)
                line[x] = value;
        }
    }
}

static QImage makeDistanceField(int imgSize, const QPainterPath &path, int dfScale, int offs)
{
    QImage image(imgSize, imgSize, QImage::Format_Indexed8);

    if (path.isEmpty()) {
        image.fill(0);
        return image;
    }

    QTransform transform;
    transform.translate(offs, offs);
    transform.scale(qreal(1) / dfScale, qreal(1) / dfScale);

    QDataBuffer<quint32> pathIndices(0);
    QDataBuffer<QPoint> pathVertices(0);
    qSimplifyPath(path, pathVertices, pathIndices, transform);

    const qint32 interiorColor = -0x7f80; // 8:8 signed format, -127.5
    const qint32 exteriorColor = 0x7f80; // 8:8 signed format, 127.5

    QScopedArrayPointer<qint32> bits(new qint32[imgSize * imgSize]);
    for (int i = 0; i < imgSize * imgSize; ++i)
        bits[i] = exteriorColor;

    const qreal angleStep = qreal(15 * 3.141592653589793238 / 180);
    const QPoint rotation(qRound(cos(angleStep) * 0x4000),
                          qRound(sin(angleStep) * 0x4000)); // 2:14 signed

    const quint32 *indices = pathIndices.data();
    QVarLengthArray<QPoint> normals;
    QVarLengthArray<QPoint> vertices;
    QVarLengthArray<bool> isConvex;
    QVarLengthArray<bool> needsClipping;

    drawPolygons(bits.data(), imgSize, imgSize, pathVertices.data(), indices, pathIndices.size(),
                 interiorColor);

    int index = 0;

    while (index < pathIndices.size()) {
        normals.clear();
        vertices.clear();
        needsClipping.clear();

        // Find end of polygon.
        int end = index;
        while (indices[end] != quint32(-1))
            ++end;

        // Calculate vertex normals.
        for (int next = index, prev = end - 1; next < end; prev = next++) {
            quint32 fromVertexIndex = indices[prev];
            quint32 toVertexIndex = indices[next];

            const QPoint &from = pathVertices.at(fromVertexIndex);
            const QPoint &to = pathVertices.at(toVertexIndex);

            QPoint n(to.y() - from.y(), from.x() - to.x());
            if (n.x() == 0 && n.y() == 0)
                continue;
            int scale = qRound((offs << 16) / sqrt(qreal(n.x() * n.x() + n.y() * n.y()))); // 8:16
            n.rx() = n.x() * scale >> 8;
            n.ry() = n.y() * scale >> 8;
            normals.append(n);
            QPoint v(to.x() + 0x7f, to.y() + 0x7f);
            vertices.append(v);
            needsClipping.append((to.x() < offs << 8) || (to.x() >= (imgSize - offs) << 8)
                                 || (to.y() < offs << 8) || (to.y() >= (imgSize - offs) << 8));
        }

        isConvex.resize(normals.count());
        for (int next = 0, prev = normals.count() - 1; next < normals.count(); prev = next++) {
            isConvex[prev] = normals.at(prev).x() * normals.at(next).y()
                           - normals.at(prev).y() * normals.at(next).x() < 0;
        }

        // Draw quads.
        for (int next = 0, prev = normals.count() - 1; next < normals.count(); prev = next++) {
            QPoint n = normals.at(next);
            QPoint intPrev = vertices.at(prev);
            QPoint extPrev = vertices.at(prev);
            QPoint intNext = vertices.at(next);
            QPoint extNext = vertices.at(next);

            extPrev.rx() -= n.x();
            extPrev.ry() -= n.y();
            intPrev.rx() += n.x();
            intPrev.ry() += n.y();
            extNext.rx() -= n.x();
            extNext.ry() -= n.y();
            intNext.rx() += n.x();
            intNext.ry() += n.y();

            if (needsClipping[prev] || needsClipping[next]) {
                drawRectangle<Clip>(bits.data(), imgSize, imgSize,
                                    &intPrev, &vertices.at(prev), &extPrev,
                                    &intNext, &vertices.at(next), &extNext,
                                    exteriorColor);
            } else {
                drawRectangle<NoClip>(bits.data(), imgSize, imgSize,
                                      &intPrev, &vertices.at(prev), &extPrev,
                                      &intNext, &vertices.at(next), &extNext,
                                      exteriorColor);
            }

            if (isConvex.at(prev)) {
                QPoint p = extPrev;
                if (needsClipping[prev]) {
                    for (;;) {
                        QPoint rn((n.x() * rotation.x() - n.y() * rotation.y()) >> 14,
                                  (n.y() * rotation.x() + n.x() * rotation.y()) >> 14);
                        n = rn;
                        if (n.x() * normals.at(prev).y() - n.y() * normals.at(prev).x() <= 0) {
                            p.rx() = vertices.at(prev).x() - normals.at(prev).x();
                            p.ry() = vertices.at(prev).y() - normals.at(prev).y();
                            drawTriangle<Clip>(bits.data(), imgSize, imgSize, &vertices.at(prev),
                                               &extPrev, &p, exteriorColor);
                            break;
                        }

                        p.rx() = vertices.at(prev).x() - n.x();
                        p.ry() = vertices.at(prev).y() - n.y();
                        drawTriangle<Clip>(bits.data(), imgSize, imgSize, &vertices.at(prev),
                                           &extPrev, &p, exteriorColor);
                        extPrev = p;
                    }
                } else {
                    for (;;) {
                        QPoint rn((n.x() * rotation.x() - n.y() * rotation.y()) >> 14,
                                  (n.y() * rotation.x() + n.x() * rotation.y()) >> 14);
                        n = rn;
                        if (n.x() * normals.at(prev).y() - n.y() * normals.at(prev).x() <= 0) {
                            p.rx() = vertices.at(prev).x() - normals.at(prev).x();
                            p.ry() = vertices.at(prev).y() - normals.at(prev).y();
                            drawTriangle<NoClip>(bits.data(), imgSize, imgSize, &vertices.at(prev),
                                                 &extPrev, &p, exteriorColor);
                            break;
                        }

                        p.rx() = vertices.at(prev).x() - n.x();
                        p.ry() = vertices.at(prev).y() - n.y();
                        drawTriangle<NoClip>(bits.data(), imgSize, imgSize, &vertices.at(prev),
                                             &extPrev, &p, exteriorColor);
                        extPrev = p;
                    }
                }
            } else {
                QPoint p = intPrev;
                if (needsClipping[prev]) {
                    for (;;) {
                        QPoint rn((n.x() * rotation.x() + n.y() * rotation.y()) >> 14,
                                  (n.y() * rotation.x() - n.x() * rotation.y()) >> 14);
                        n = rn;
                        if (n.x() * normals.at(prev).y() - n.y() * normals.at(prev).x() >= 0) {
                            p.rx() = vertices.at(prev).x() + normals.at(prev).x();
                            p.ry() = vertices.at(prev).y() + normals.at(prev).y();
                            drawTriangle<Clip>(bits.data(), imgSize, imgSize, &vertices.at(prev),
                                               &p, &intPrev, interiorColor);
                            break;
                        }

                        p.rx() = vertices.at(prev).x() + n.x();
                        p.ry() = vertices.at(prev).y() + n.y();
                        drawTriangle<Clip>(bits.data(), imgSize, imgSize, &vertices.at(prev),
                                           &p, &intPrev, interiorColor);
                        intPrev = p;
                    }
                } else {
                    for (;;) {
                        QPoint rn((n.x() * rotation.x() + n.y() * rotation.y()) >> 14,
                                  (n.y() * rotation.x() - n.x() * rotation.y()) >> 14);
                        n = rn;
                        if (n.x() * normals.at(prev).y() - n.y() * normals.at(prev).x() >= 0) {
                            p.rx() = vertices.at(prev).x() + normals.at(prev).x();
                            p.ry() = vertices.at(prev).y() + normals.at(prev).y();
                            drawTriangle<NoClip>(bits.data(), imgSize, imgSize, &vertices.at(prev),
                                                 &p, &intPrev, interiorColor);
                            break;
                        }

                        p.rx() = vertices.at(prev).x() + n.x();
                        p.ry() = vertices.at(prev).y() + n.y();
                        drawTriangle<NoClip>(bits.data(), imgSize, imgSize, &vertices.at(prev),
                                             &p, &intPrev, interiorColor);
                        intPrev = p;
                    }
                }
            }
        }

        index = end + 1;
    }

    const qint32 *inLine = bits.data();
    uchar *outLine = image.bits();
    int padding = image.bytesPerLine() - image.width();
    for (int y = 0; y < imgSize; ++y) {
        for (int x = 0; x < imgSize; ++x, ++inLine, ++outLine)
            *outLine = uchar((0x7f80 - *inLine) >> 8);
        outLine += padding;
    }

    return image;
}

bool qt_fontHasNarrowOutlines(const QRawFont &f)
{
    QRawFont font = f;
    font.setPixelSize(QT_DISTANCEFIELD_DEFAULT_BASEFONTSIZE);
    Q_ASSERT(font.isValid());

    QVector<quint32> glyphIndices = font.glyphIndexesForString(QLatin1String("O"));
    if (glyphIndices.size() < 1)
        return false;

    QImage im = font.alphaMapForGlyph(glyphIndices.at(0), QRawFont::PixelAntialiasing);
    if (im.isNull())
        return false;

    int minHThick = 999;
    int minVThick = 999;

    int thick = 0;
    bool in = false;
    int y = (im.height() + 1) / 2;
    for (int x = 0; x < im.width(); ++x) {
        int a = qAlpha(im.pixel(x, y));
        if (a > 127) {
            in = true;
            ++thick;
        } else if (in) {
            in = false;
            minHThick = qMin(minHThick, thick);
            thick = 0;
        }
    }

    thick = 0;
    in = false;
    int x = (im.width() + 1) / 2;
    for (int y = 0; y < im.height(); ++y) {
        int a = qAlpha(im.pixel(x, y));
        if (a > 127) {
            in = true;
            ++thick;
        } else if (in) {
            in = false;
            minVThick = qMin(minVThick, thick);
            thick = 0;
        }
    }

    return minHThick == 1 || minVThick == 1;
}

QImage qt_renderDistanceFieldGlyph(const QRawFont &font, glyph_t glyph, bool doubleResolution)
{
    QRawFont renderFont = font;
    renderFont.setPixelSize(QT_DISTANCEFIELD_BASEFONTSIZE(doubleResolution) * QT_DISTANCEFIELD_SCALE(doubleResolution));

    QPainterPath path = renderFont.pathForGlyph(glyph);
    path.translate(-path.boundingRect().topLeft());
    path.setFillRule(Qt::WindingFill);

    QImage im = makeDistanceField(QT_DISTANCEFIELD_TILESIZE(doubleResolution),
                                  path,
                                  QT_DISTANCEFIELD_SCALE(doubleResolution),
                                  QT_DISTANCEFIELD_RADIUS(doubleResolution) / QT_DISTANCEFIELD_SCALE(doubleResolution));
    return im;
}

QSGDistanceFieldGlyphCacheManager::QSGDistanceFieldGlyphCacheManager(QSGContext *c)
    : sgCtx(c)
    , m_threshold_func(defaultThresholdFunc)
    , m_antialiasingSpread_func(defaultAntialiasingSpreadFunc)
{
#ifndef QT_OPENGL_ES
    m_defaultAntialiasingMode = QSGGlyphNode::HighQualitySubPixelAntialiasing;
#else
    m_defaultAntialiasingMode = QSGGlyphNode::GrayAntialiasing;
#endif
}

QSGDistanceFieldGlyphCacheManager::~QSGDistanceFieldGlyphCacheManager()
{
    qDeleteAll(m_caches.values());
}

QSGDistanceFieldGlyphCache *QSGDistanceFieldGlyphCacheManager::cache(const QRawFont &font)
{
    QRawFontPrivate *fontD = QRawFontPrivate::get(font);
    QHash<QFontEngine *, QSGDistanceFieldGlyphCache *>::iterator cache = m_caches.find(fontD->fontEngine);
    if (cache == m_caches.end())
        cache = m_caches.insert(fontD->fontEngine, sgCtx->createDistanceFieldGlyphCache(font));
    return cache.value();
}

QT_END_NAMESPACE
