/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "meegocompass.h"

char const * const meegocompass::id("meego.compass");
bool meegocompass::m_initDone = false;

meegocompass::meegocompass(QSensor *sensor)
    : meegosensorbase(sensor)
{
    initSensor<CompassSensorChannelInterface>(m_initDone);
    setReading<QCompassReading>(&m_reading);
}

void meegocompass::slotDataAvailable(const Compass& data)
{
    // The scale for level is [0,3], where 3 is the best
    // Qt: Measured as a value from 0 to 1 with higher values being better.
    m_reading.setCalibrationLevel(((float) data.level()) / 3.0);

    // The scale for degrees from sensord is [0,359]
    // Value can be directly used as azimuth
    m_reading.setAzimuth(data.degrees());

    m_reading.setTimestamp(data.data().timestamp_);
    newReadingAvailable();
}


bool meegocompass::doConnect(){
    return QObject::connect(m_sensorInterface, SIGNAL(dataAvailable(const Compass&)),
                            this, SLOT(slotDataAvailable(const Compass&)));
}

QString meegocompass::sensorName() const{
    return "compasssensor";
}

