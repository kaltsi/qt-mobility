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
import FrequencyMonitor 1.0

Rectangle {
    id: root
    property bool logging: true
    property bool displayed: true
    property bool enabled: logging || displayed
    property alias active: monitor.active
    property int samplingInterval: 500
    property color textColor: "yellow"
    property int textSize: 20
    property alias label: monitor.label

    border.width: 1
    border.color: "yellow"
    width: 5.5 * root.textSize
    height: 3.0 * root.textSize
    color: "black"
    opacity: 0.5
    radius: 10
    visible: displayed && active

    // This should ensure that the monitor is on top of all other content
    z: 999

    function notify() {
        monitor.notify()
    }

    FrequencyMonitor {
        id: monitor
        samplingInterval: root.enabled ? root.samplingInterval : 0
        onAverageFrequencyChanged: {
            if (root.logging) trace()
            averageFrequencyText.text = monitor.averageFrequency.toFixed(2)
        }
    }

    Text {
        id: labelText
        anchors {
            left: parent.left
            top: parent.top
            margins: 10
        }
        color: root.textColor
        font.pixelSize: 0.6 * root.textSize
        text: root.label
        width: root.width - 2*anchors.margins
        elide: Text.ElideRight
    }

    Text {
        id: averageFrequencyText
        anchors {
            right: parent.right
            bottom: parent.bottom
            margins: 10
        }
        color: root.textColor
        font.pixelSize: root.textSize
    }
}
