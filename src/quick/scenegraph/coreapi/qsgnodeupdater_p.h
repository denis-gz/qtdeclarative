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

#ifndef NODEUPDATER_P_H
#define NODEUPDATER_P_H

#include "qsgnode.h"
#include <QtGui/private/qdatabuffer_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK_EXPORT QSGNodeUpdater
{
public:
    QSGNodeUpdater();

    virtual void updateStates(QSGNode *n);
    virtual bool isNodeBlocked(QSGNode *n, QSGNode *root) const;

    void setToplevelOpacity(qreal alpha) { m_opacity_stack.last() = alpha; }
    qreal toplevelOpacity() const { return m_opacity_stack.last(); }

protected:
    virtual void enterTransformNode(QSGTransformNode *);
    virtual void leaveTransformNode(QSGTransformNode *);
    void enterClipNode(QSGClipNode *c);
    void leaveClipNode(QSGClipNode *c);
    void enterOpacityNode(QSGOpacityNode *o);
    void leaveOpacityNode(QSGOpacityNode *o);
    void enterGeometryNode(QSGGeometryNode *);
    void leaveGeometryNode(QSGGeometryNode *);

    void visitNode(QSGNode *n);
    void visitChildren(QSGNode *n);


    QDataBuffer<const QMatrix4x4 *> m_combined_matrix_stack;
    QDataBuffer<qreal> m_opacity_stack;
    const QSGClipNode *m_current_clip;

    int m_force_update;

    qreal m_toplevel_alpha;
};

QT_END_NAMESPACE

#endif // NODEUPDATER_P_H
