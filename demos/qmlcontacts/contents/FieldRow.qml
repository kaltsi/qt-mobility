/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/

import Qt 4.7

Item {
    property string label
    property string value
    property alias newValue: textEdit.text

    signal blur

    height: childrenRect.height
    Text {
        id: fieldName
        width: parent.width * 0.4;
        height: 20;
        anchors.margins: 3
        anchors.left: parent.left
        text: label
        color: "white"
    }
    Rectangle {
        id: textRect
        anchors.left: fieldName.right
        anchors.right: parent.right
        anchors.rightMargin: 3
        height: 30;
        color: "white"
        border.color: "#00000000";
        border.width: 0
        radius: 2
        opacity: 0.5

        states: [
                State {
                    name: "focused"
                    when: textEdit.activeFocus
                    PropertyChanges {
                        target: textRect
                        color: "#aaffffff"
                        radius: 2
                        border.width: 1
                        border.color: "black"
                    }
                }
            ]
    }

    TextInput {
        id: textEdit
        anchors.fill: textRect
        anchors.margins: 3
        text: value
        color: "black"
        onActiveFocusChanged: { if (!activeFocus) { blur(); } }
    }
}
