/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.12

ListView {
    id: root
    objectName: "root"
    property Item buttonUnderTest: null
    property Item delegateUnderTest: null
    width: 800
    height: 480
    model: 10
    spacing: 2

    delegate: Rectangle {
        objectName: "itemview delegate"
        color: delegateDrag.active ? "wheat" : "beige"
        border.color: "black"
        width: parent.width; height: 140
        Rectangle {
            objectName: "button"
            x: 350; y: 20
            border.color: "tomato"
            border.width: 10
            color: buttonDrag.active ? "goldenrod" : "beige"
            width: 100
            height: 100
            DragHandler {
                id: buttonDrag
                objectName: "buttonDrag"
            }
            Component.onCompleted: if (!root.buttonUnderTest) {
                root.buttonUnderTest = this
                root.delegateUnderTest = parent
            }
        }
        DragHandler {
            id: delegateDrag
            objectName: "delegateDrag " + index
        }
    }

    DragHandler {
        objectName: "contentItemDrag"
    }

    Component.onCompleted: contentItem.objectName = "ListView's contentItem"
}