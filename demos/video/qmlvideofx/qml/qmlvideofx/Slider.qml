/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

import QtQuick 1.0

Rectangle {
    id: root
    color: "transparent"
    radius: 5
    property alias value: grip.value
    property color fillColor: "white"
    property color lineColor: "black"
    property color gripColor: "white"
    property real gripSize: 20
    property real gripTolerance: 3.0
    property real increment: 0.1
    property bool enabled: true

    Rectangle {
        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        height: 3
        color: displayedColor(root.lineColor)

        Rectangle {
            anchors { fill: parent; margins: 1 }
            color: root.fillColor
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        onClicked: {
            if (parent.width) {
                var newValue = mouse.x / parent.width
                if (Math.abs(newValue - parent.value) > parent.increment) {
                    if (newValue > parent.value)
                        parent.value = Math.min(1.0, parent.value + parent.increment)
                    else
                        parent.value = Math.max(0.0, parent.value - parent.increment)
                }
            }
        }
    }

    Rectangle {
        id: grip
        property real value: 0.5
        x: (value * parent.width) - width/2
        anchors.verticalCenter: parent.verticalCenter
        width: root.gripTolerance * root.gripSize
        height: width
        radius: width/2
        color: "transparent"

        MouseArea {
            id: mouseArea
            enabled: root.enabled
            anchors.fill:  parent
            drag {
                target: grip
                axis: Drag.XAxis
                minimumX: -parent.width/2
                maximumX: root.width - parent.width/2
            }
            onPositionChanged:  {
                if (drag.active)
                    updatePosition()
            }
            onReleased: {
                updatePosition()
            }
            function updatePosition() {
                value = (grip.x + grip.width/2) / grip.parent.width
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: root.gripSize
            height: width
            radius: width/2
            color: root.gripColor
        }
    }

    function displayedColor(c) {
        var tint = Qt.rgba(c.r, c.g, c.b, 0.25)
        return enabled ? c : Qt.tint(c, tint)
    }
}
