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

#include "qproximitysensor.h"
#include "qproximitysensor_p.h"

QTM_BEGIN_NAMESPACE

IMPLEMENT_READING(QProximityReading)

/*!
    \class QProximityReading
    \ingroup sensors_reading
    \inmodule QtSensors
    \since 1.0

    \brief The QProximityReading class represents one reading from the
           proximity sensor.

    \target QProximityReading_Units
    The proximity sensor can only indicate if an object is close or not.

    The distance at which an object is considered close is device-specific. This
    distance may be available in the QSensor::outputRanges property.
*/

/*!
    \property QProximityReading::close
    \brief a value indicating if something is close.

    Set to true if something is close.
    Set to false is nothing is close.

    \sa QProximityReading_Units
    \since 1.0
*/

bool QProximityReading::close() const
{
    return d->close;
}

/*!
    Sets the close value to \a close.
    \since 1.0
*/
void QProximityReading::setClose(bool close)
{
    d->close = close;
}

// =====================================================================

/*!
    \class QProximityFilter
    \ingroup sensors_filter
    \inmodule QtSensors

    \brief The QProximityFilter class is a convenience wrapper around QSensorFilter.

    The only difference is that the filter() method features a pointer to QProximityReading
    instead of QSensorReading.
    \since 1.0
*/

/*!
    \fn QProximityFilter::filter(QProximityReading *reading)

    Called when \a reading changes. Returns false to prevent the reading from propagating.

    \sa QSensorFilter::filter()
    \since 1.0
*/

char const * const QProximitySensor::type("QProximitySensor");

/*!
    \class QProximitySensor
    \ingroup sensors_type
    \inmodule QtSensors

    \brief The QProximitySensor class is a convenience wrapper around QSensor.

    The only behavioural difference is that this class sets the type properly.

    This class also features a reading() function that returns a QProximityReading instead of a QSensorReading.

    For details about how the sensor works, see \l QProximityReading.

    \sa QProximityReading
    \since 1.0
*/

/*!
    \fn QProximitySensor::QProximitySensor(QObject *parent)

    Construct the sensor as a child of \a parent.
    \since 1.0
*/

/*!
    \fn QProximitySensor::~QProximitySensor()

    Destroy the sensor. Stops the sensor if it has not already been stopped.
    \since 1.0
*/

/*!
    \fn QProximitySensor::reading() const

    Returns the reading class for this sensor.

    \sa QSensor::reading()
    \since 1.0
*/

#include "moc_qproximitysensor.cpp"
QTM_END_NAMESPACE

