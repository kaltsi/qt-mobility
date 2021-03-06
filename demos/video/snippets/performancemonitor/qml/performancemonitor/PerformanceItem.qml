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
    property bool logging: true
    property bool displayed: true
    property bool videoActive
    property int margins: 5
    property bool enabled: true

    color: "transparent"

    // This should ensure that the monitor is on top of all other content
    z: 999

    Column {
        id: column
        anchors {
            fill: root
            margins: 10
        }
        spacing: 10
    }

    QtObject {
        id: d
        property Item qmlFrameRateItem: null
        property Item videoFrameRateItem: null
    }

    Connections {
        id: videoFrameRateActiveConnections
        ignoreUnknownSignals: true
        onActiveChanged: root.videoActive = videoFrameRateActiveConnections.target.active
    }

    states: [
        State {
            name: "hidden"
            PropertyChanges {
                target: root
                opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation {
                properties: "opacity"
                easing.type: Easing.OutQuart
                duration: 500
            }
        }
    ]

    state: enabled ? "baseState" : "hidden"

    function createQmlFrameRateItem() {
        var component = Qt.createComponent("../frequencymonitor/FrequencyItem.qml")
        if (component.status == Component.Ready)
            d.qmlFrameRateItem = component.createObject(column, { label: "QML frame rate",
                                                                   displayed: root.displayed,
                                                                  logging: root.logging
                                                                })
    }

    function createVideoFrameRateItem() {
        var component = Qt.createComponent("../frequencymonitor/FrequencyItem.qml")
        if (component.status == Component.Ready)
            d.videoFrameRateItem = component.createObject(column, { label: "Video frame rate",
                                                                     displayed: root.displayed,
                                                                    logging: root.logging
                                                                  })
        videoFrameRateActiveConnections.target = d.videoFrameRateItem
    }


    function init() {
        createQmlFrameRateItem()
        createVideoFrameRateItem()
    }

    function videoFramePainted() {
        if (d.videoFrameRateItem)
            d.videoFrameRateItem.notify()
    }

    function qmlFramePainted() {
        if (d.qmlFrameRateItem)
            d.qmlFrameRateItem.notify()
    }

    onVideoActiveChanged: {
        if (d.videoFrameRateItem)
            d.videoFrameRateItem.active = root.videoActive
    }
}
