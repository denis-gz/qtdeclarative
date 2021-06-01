/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
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

#ifndef QTQUICKDIALOGS2QUICKIMPLFOREIGN_P_H
#define QTQUICKDIALOGS2QUICKIMPLFOREIGN_P_H

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

#include <QtQml/qqml.h>
#include <QtQuickDialogs2Utils/private/qquickfilenamefilter_p.h>
#include <QtQuickTemplates2/private/qquickabstractbutton_p.h>
#include <QtQuickTemplates2/private/qquickcontrol_p.h>
#include <QtQuickTemplates2/private/qquickdialog_p.h>
#include <QtQuickTemplates2/private/qquickicon_p.h>
#include <QtQuickTemplates2/private/qquickpopup_p.h>

QT_BEGIN_NAMESPACE

struct QQuickFileNameFilterForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QQuickFileNameFilter)
    QML_ADDED_IN_VERSION(6, 2)
};

// TODO: remove these ones when not needed (QTBUG-88179)

// verticalPadding, etc.
struct QQuickControlForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QQuickControl)
    QML_ADDED_IN_VERSION(2, 0)
};

struct QQuickAbstractButtonForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QQuickAbstractButton)
    QML_ADDED_IN_VERSION(2, 0)
};

struct QQuickIconForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QQuickIcon)
    QML_ADDED_IN_VERSION(6, 2)
};

// For leftInset, etc.
struct QQuickPopupForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QQuickPopup)
    QML_ADDED_IN_VERSION(2, 0)
};

struct QQuickDialogForeign
{
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QQuickDialog)
    QML_ADDED_IN_VERSION(2, 1)
};

QT_END_NAMESPACE

#endif // QTQUICKDIALOGS2QUICKIMPLFOREIGN_P_H