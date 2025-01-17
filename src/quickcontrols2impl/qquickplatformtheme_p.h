/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKPLATFORMTHEME_P_H
#define QQUICKPLATFORMTHEME_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtGui/qpa/qplatformtheme.h>
#include <QtQml/qqml.h>
#include <QtQuickControls2Impl/private/qtquickcontrols2implglobal_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICKCONTROLS2IMPL_PRIVATE_EXPORT QQuickPlatformTheme : public QObject
{
    Q_OBJECT
    // This exposes the enums in QPlatformTheme to QML. We can't use QML_EXTENDED
    // because it's not possible to extend a QGadget (QPlatformTheme) with an QObject (us).
    QML_EXTENDED_NAMESPACE(QPlatformTheme)
    QML_NAMED_ELEMENT(PlatformTheme)
    QML_SINGLETON
    QML_ADDED_IN_VERSION(6, 3)

public:
    explicit QQuickPlatformTheme(QObject *parent = nullptr);

    Q_INVOKABLE QVariant themeHint(QPlatformTheme::ThemeHint themeHint) const;
};

QT_END_NAMESPACE

#endif // QQUICKPLATFORMTHEME_P_H
