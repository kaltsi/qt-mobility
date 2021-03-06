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
#include "qgeocoordinate.h"
#include "qgeocoordinate_p.h"
#include "qlocationutils_p.h"

#include <QDateTime>
#include <QHash>
#include <QDataStream>
#include <QDebug>
#include <qnumeric.h>

#include <math.h>

#include <QDebug>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

QTM_BEGIN_NAMESPACE

static const double qgeocoordinate_EARTH_MEAN_RADIUS = 6371.0072;

inline static double qgeocoordinate_degToRad(double deg)
{
    return deg * M_PI / 180;
}
inline static double qgeocoordinate_radToDeg(double rad)
{
    return rad * 180 / M_PI;
}


QGeoCoordinatePrivate::QGeoCoordinatePrivate() {
    lat = qQNaN();
    lng = qQNaN();
    alt = qQNaN();
}


/*!
    \class QGeoCoordinate
    \brief The QGeoCoordinate class defines a geographical position on the surface of the Earth.

    \inmodule QtLocation
    \since 1.0

    \ingroup location

    A QGeoCoordinate is defined by latitude, longitude, and optionally, altitude.

    Use type() to determine whether a coordinate is a 2D coordinate (has
    latitude and longitude only) or 3D coordinate (has latitude, longitude
    and altitude). Use distanceTo() and azimuthTo() to calculate the distance
    and bearing between coordinates.

    The coordinate values should be specified using the WGS84 datum.
*/

/*!
    \enum QGeoCoordinate::CoordinateType
    Defines the types of a coordinate.

    \value InvalidCoordinate An invalid coordinate. A coordinate is invalid if its latitude or longitude values are invalid.
    \value Coordinate2D A coordinate with valid latitude and longitude values.
    \value Coordinate3D A coordinate with valid latitude and longitude values, and also an altitude value.
*/

/*!
    \enum QGeoCoordinate::CoordinateFormat
    Defines the possible formatting options for toString().

    \value Degrees Returns a string representation of the coordinates in decimal degrees format.
    \value DegreesWithHemisphere Returns a string representation of the coordinates in decimal degrees format, using 'N', 'S', 'E' or 'W' to indicate the hemispheres of the coordinates.
    \value DegreesMinutes Returns a string representation of the coordinates in degrees-minutes format.
    \value DegreesMinutesWithHemisphere Returns a string representation of the coordinates in degrees-minutes format, using 'N', 'S', 'E' or 'W' to indicate the hemispheres of the coordinates.
    \value DegreesMinutesSeconds Returns a string representation of the coordinates in degrees-minutes-seconds format.
    \value DegreesMinutesSecondsWithHemisphere Returns a string representation of the coordinates in degrees-minutes-seconds format, using 'N', 'S', 'E' or 'W' to indicate the hemispheres of the coordinates.

    \sa toString()
*/


/*!
    Constructs a coordinate. The coordinate will be invalid until
    setLatitude() and setLongitude() have been called.
*/
QGeoCoordinate::QGeoCoordinate()
        : d(new QGeoCoordinatePrivate)
{
}

/*!
    Constructs a coordinate with the given \a latitude and \a longitude.

    If the latitude is not between -90 to 90 inclusive, or the longitude
    is not between -180 to 180 inclusive, none of the values are set and
    the type() will be QGeoCoordinate::InvalidCoordinate.

    \sa isValid()
    \since 1.0
*/
QGeoCoordinate::QGeoCoordinate(double latitude, double longitude)
        : d(new QGeoCoordinatePrivate)
{
    if (QLocationUtils::isValidLat(latitude) && QLocationUtils::isValidLong(longitude)) {
        d->lat = latitude;
        d->lng = longitude;
    }
}

/*!
    Constructs a coordinate with the given \a latitude, \a longitude
    and \a altitude.

    If the latitude is not between -90 to 90 inclusive, or the longitude
    is not between -180 to 180 inclusive, none of the values are set and
    the type() will be QGeoCoordinate::InvalidCoordinate.

    Note that \a altitude specifies the metres above sea level.

    \sa isValid()
    \since 1.0
*/
QGeoCoordinate::QGeoCoordinate(double latitude, double longitude, double altitude)
        : d(new QGeoCoordinatePrivate)
{
    if (QLocationUtils::isValidLat(latitude) && QLocationUtils::isValidLong(longitude)) {
        d->lat = latitude;
        d->lng = longitude;
        d->alt = altitude;
    }
}

/*!
    Constructs a coordinate from the contents of \a other.
    \since 1.0
*/
QGeoCoordinate::QGeoCoordinate(const QGeoCoordinate &other)
        : d(new QGeoCoordinatePrivate)
{
    operator=(other);
}

/*!
    Destroys the coordinate object.
*/
QGeoCoordinate::~QGeoCoordinate()
{
    delete d;
}

/*!
    Assigns \a other to this coordinate and returns a reference to this
    coordinate.
    \since 1.0
*/
QGeoCoordinate &QGeoCoordinate::operator=(const QGeoCoordinate & other)
{
    if (this == &other)
        return *this;

    d->lat = other.d->lat;
    d->lng = other.d->lng;
    d->alt = other.d->alt;

    return *this;
}

/*!
    Returns true if the latitude, longitude and altitude of this
    coordinate are the same as those of \a other.

    The longitude will be ignored if the latitude is +/- 90 degrees.
    \since 1.0
*/
bool QGeoCoordinate::operator==(const QGeoCoordinate &other) const
{
    bool latEqual = (qIsNaN(d->lat) && qIsNaN(other.d->lat))
                        || qFuzzyCompare(d->lat, other.d->lat);
    bool lngEqual = (qIsNaN(d->lng) && qIsNaN(other.d->lng))
                        || qFuzzyCompare(d->lng, other.d->lng);
    bool altEqual = (qIsNaN(d->alt) && qIsNaN(other.d->alt))
                        || qFuzzyCompare(d->alt, other.d->alt);

    if (!qIsNaN(d->lat) && ((d->lat == 90.0) || (d->lat == -90.0)))
        lngEqual = true;

    return (latEqual && lngEqual && altEqual);
}

/*!
    \fn bool QGeoCoordinate::operator!=(const QGeoCoordinate &other) const;

    Returns true if the latitude, longitude or altitude of this
    coordinate are not the same as those of \a other.
    \since 1.0
*/

/*!
    Returns true if the type() is Coordinate2D or Coordinate3D.
    \since 1.0
*/
bool QGeoCoordinate::isValid() const
{
    CoordinateType t = type();
    return t == Coordinate2D || t == Coordinate3D;
}

/*!
    Returns the type of this coordinate.
    \since 1.0
*/
QGeoCoordinate::CoordinateType QGeoCoordinate::type() const
{
    if (QLocationUtils::isValidLat(d->lat)
            && QLocationUtils::isValidLong(d->lng)) {
        if (qIsNaN(d->alt))
            return Coordinate2D;
        return Coordinate3D;
    }
    return InvalidCoordinate;
}


/*!
    Returns the latitude, in decimal degrees. The return value is undefined
    if the latitude has not been set.

    A positive latitude indicates the Northern Hemisphere, and a negative
    latitude indicates the Southern Hemisphere.

    \sa setLatitude(), type()
    \since 1.0
*/
double QGeoCoordinate::latitude() const
{
    return d->lat;
}

/*!
    Sets the latitude (in decimal degrees) to \a latitude. The value should
    be in the WGS84 datum.

    To be valid, the latitude must be between -90 to 90 inclusive.

    \sa latitude()
    \since 1.0
*/
void QGeoCoordinate::setLatitude(double latitude)
{
    d->lat = latitude;
}

/*!
    Returns the longitude, in decimal degrees. The return value is undefined
    if the longitude has not been set.

    A positive longitude indicates the Eastern Hemisphere, and a negative
    longitude indicates the Western Hemisphere.

    \sa setLongitude(), type()
    \since 1.0
*/
double QGeoCoordinate::longitude() const
{
    return d->lng;
}

/*!
    Sets the longitude (in decimal degrees) to \a longitude. The value should
    be in the WGS84 datum.

    To be valid, the longitude must be between -180 to 180 inclusive.

    \sa longitude()
    \since 1.0
*/
void QGeoCoordinate::setLongitude(double longitude)
{
    d->lng = longitude;
}

/*!
    Returns the altitude (meters above sea level).

    The return value is undefined if the altitude has not been set.

    \since 1.0
    \sa setAltitude(), type()
*/
double QGeoCoordinate::altitude() const
{
    return d->alt;
}

/*!
    Sets the altitude (meters above sea level) to \a altitude.

    \since 1.0
    \sa altitude()
*/
void QGeoCoordinate::setAltitude(double altitude)
{
    d->alt = altitude;
}

/*!
    Returns the distance (in meters) from this coordinate to the coordinate
    specified by \a other. Altitude is not used in the calculation.

    This calculation returns the great-circle distance between the two
    coordinates, with an assumption that the Earth is spherical for the
    purpose of this calculation.

    Returns 0 if the type of this coordinate or the type of \a other is
    QGeoCoordinate::InvalidCoordinate.
    \since 1.0
*/
qreal QGeoCoordinate::distanceTo(const QGeoCoordinate &other) const
{
    if (type() == QGeoCoordinate::InvalidCoordinate
            || other.type() == QGeoCoordinate::InvalidCoordinate) {
        return 0;
    }

    // Haversine formula
    double dlat = qgeocoordinate_degToRad(other.d->lat - d->lat);
    double dlon = qgeocoordinate_degToRad(other.d->lng - d->lng);
    double haversine_dlat = sin(dlat / 2.0);
    haversine_dlat *= haversine_dlat;
    double haversine_dlon = sin(dlon / 2.0);
    haversine_dlon *= haversine_dlon;
    double y = haversine_dlat
             + cos(qgeocoordinate_degToRad(d->lat))
             * cos(qgeocoordinate_degToRad(other.d->lat))
             * haversine_dlon;
    double x = 2 * asin(sqrt(y));
    return qreal(x * qgeocoordinate_EARTH_MEAN_RADIUS * 1000);
}

/*!
    Returns the azimuth (or bearing) in degrees from this coordinate to the
    coordinate specified by \a other. Altitude is not used in the calculation.

    The bearing returned is the bearing from the origin to \a other along the
    great-circle between the two coordinates. There is an assumption that the
    Earth is spherical for the purpose of this calculation.

    Returns 0 if the type of this coordinate or the type of \a other is
    QGeoCoordinate::InvalidCoordinate.
    \since 1.0
*/
qreal QGeoCoordinate::azimuthTo(const QGeoCoordinate &other) const
{
    if (type() == QGeoCoordinate::InvalidCoordinate
            || other.type() == QGeoCoordinate::InvalidCoordinate) {
        return 0;
    }

    double dlon = qgeocoordinate_degToRad(other.d->lng - d->lng);
    double lat1Rad = qgeocoordinate_degToRad(d->lat);
    double lat2Rad = qgeocoordinate_degToRad(other.d->lat);

    double y = sin(dlon) * cos(lat2Rad);
    double x = cos(lat1Rad) * sin(lat2Rad) - sin(lat1Rad) * cos(lat2Rad) * cos(dlon);

    double whole;
    double fraction = modf(qgeocoordinate_radToDeg(atan2(y, x)), &whole);
    return qreal((int(whole + 360) % 360) + fraction);
}

void QGeoCoordinatePrivate::atDistanceAndAzimuth(const QGeoCoordinate &coord,
                                                 qreal distance, qreal azimuth,
                                                 double *lon, double *lat)
{
    double latRad = qgeocoordinate_degToRad(coord.d->lat);
    double lonRad = qgeocoordinate_degToRad(coord.d->lng);
    double cosLatRad = cos(latRad);
    double sinLatRad = sin(latRad);

    double azimuthRad = qgeocoordinate_degToRad(azimuth);

    double ratio = (distance / (qgeocoordinate_EARTH_MEAN_RADIUS * 1000.0));
    double cosRatio = cos(ratio);
    double sinRatio = sin(ratio);

    double resultLatRad = asin(sinLatRad * cosRatio
                               + cosLatRad * sinRatio * cos(azimuthRad));
    double resultLonRad  = lonRad
                           + atan2(sin(azimuthRad) * sinRatio * cosLatRad,
                                   cosRatio - sinLatRad * sin(resultLatRad));

    *lat = qgeocoordinate_radToDeg(resultLatRad);
    *lon = qgeocoordinate_radToDeg(resultLonRad);
}

/*!
    Returns the coordinate that is reached by traveling \a distance metres
    from the current coordinate at \a azimuth (or bearing) along a great-circle.
    There is an assumption that the Earth is spherical for the purpose of this
    calculation.

    The altitude will have \a distanceUp added to it.

    Returns an invalid coordinate if this coordinate is invalid.
*/
QGeoCoordinate QGeoCoordinate::atDistanceAndAzimuth(qreal distance, qreal azimuth, qreal distanceUp) const
{
    if (!isValid())
        return QGeoCoordinate();

    double resultLon, resultLat;
    QGeoCoordinatePrivate::atDistanceAndAzimuth(*this, distance, azimuth,
                                                &resultLon, &resultLat);

    if (resultLon > 180.0)
        resultLon -= 360.0;
    else if (resultLon < -180.0)
        resultLon += 360.0;

    double resultAlt = d->alt + distanceUp;
    return QGeoCoordinate(resultLat, resultLon, resultAlt);
}

/*!
    Returns this coordinate as a string in the specified \a format.

    For example, if this coordinate has a latitude of -27.46758, a longitude
    of 153.027892 and an altitude of 28.1, these are the strings
    returned depending on \a format:

    \table
    \header
        \o \a format value
        \o Returned string
    \row
        \o \l Degrees
        \o -27.46758\unicode{0xB0}, 153.02789\unicode{0xB0}, 28.1m
    \row
        \o \l DegreesWithHemisphere
        \o 27.46758\unicode{0xB0} S, 153.02789\unicode{0xB0} E, 28.1m
    \row
        \o \l DegreesMinutes
        \o -27\unicode{0xB0} 28.054', 153\unicode{0xB0} 1.673', 28.1m
    \row
        \o \l DegreesMinutesWithHemisphere
        \o 27\unicode{0xB0} 28.054 S', 153\unicode{0xB0} 1.673' E, 28.1m
    \row
        \o \l DegreesMinutesSeconds
        \o -27\unicode{0xB0} 28' 3.2", 153\unicode{0xB0} 1' 40.4", 28.1m
    \row
        \o \l DegreesMinutesSecondsWithHemisphere
        \o 27\unicode{0xB0} 28' 3.2" S, 153\unicode{0xB0} 1' 40.4" E, 28.1m
    \endtable

    The altitude field is omitted if no altitude is set.

    If the coordinate is invalid, an empty string is returned.
    \since 1.0
*/
QString QGeoCoordinate::toString(CoordinateFormat format) const
{
    if (type() == QGeoCoordinate::InvalidCoordinate)
        return QString();

    QString latStr;
    QString longStr;

    double absLat = qAbs(d->lat);
    double absLng = qAbs(d->lng);
    QChar symbol(0x00B0);   // degrees symbol

    switch (format) {
        case Degrees:
        case DegreesWithHemisphere: {
            latStr = QString::number(absLat, 'f', 5) + symbol;
            longStr = QString::number(absLng, 'f', 5) + symbol;
            break;
        }
        case DegreesMinutes:
        case DegreesMinutesWithHemisphere: {
            double latMin = (absLat - int(absLat)) * 60;
            double lngMin = (absLng - int(absLng)) * 60;
            latStr = QString("%1%2 %3'")
                     .arg(QString::number(int(absLat)))
                     .arg(symbol)
                     .arg(QString::number(latMin, 'f', 3));
            longStr = QString("%1%2 %3'")
                      .arg(QString::number(int(absLng)))
                      .arg(symbol)
                      .arg(QString::number(lngMin, 'f', 3));
            break;
        }
        case DegreesMinutesSeconds:
        case DegreesMinutesSecondsWithHemisphere: {
            double latMin = (absLat - int(absLat)) * 60;
            double lngMin = (absLng - int(absLng)) * 60;
            double latSec = (latMin - int(latMin)) * 60;
            double lngSec = (lngMin - int(lngMin)) * 60;

            latStr = QString("%1%2 %3' %4\"")
                     .arg(QString::number(int(absLat)))
                     .arg(symbol)
                     .arg(QString::number(int(latMin)))
                     .arg(QString::number(latSec, 'f', 1));
            longStr = QString("%1%2 %3' %4\"")
                      .arg(QString::number(int(absLng)))
                      .arg(symbol)
                      .arg(QString::number(int(lngMin)))
                      .arg(QString::number(lngSec, 'f', 1));
            break;
        }
    }

    // now add the "-" to the start, or append the hemisphere char
    switch (format) {
        case Degrees:
        case DegreesMinutes:
        case DegreesMinutesSeconds: {
            if (d->lat < 0)
                latStr.insert(0, "-");
            if (d->lng < 0)
                longStr.insert(0, "-");
            break;
        }
        case DegreesWithHemisphere:
        case DegreesMinutesWithHemisphere:
        case DegreesMinutesSecondsWithHemisphere: {
            if (d->lat < 0)
                latStr.append(QString(" S"));
            else if (d->lat > 0)
                latStr.append(QString(" N"));
            if (d->lng < 0)
                longStr.append(QString(" W"));
            else if (d->lng > 0)
                longStr.append(QString(" E"));
            break;
        }
    }

    if (qIsNaN(d->alt))
        return QString("%1, %2").arg(latStr, longStr);
    return QString("%1, %2, %3m").arg(latStr).arg(longStr).arg(QString::number(d->alt));
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QGeoCoordinate &coord)
{
    double lat = coord.latitude();
    double lng = coord.longitude();

    dbg.nospace() << "QGeoCoordinate(";
    if (qIsNaN(lat))
        dbg.nospace() << '?';
    else
        dbg.nospace() << lat;
    dbg.nospace() << ", ";
    if (qIsNaN(lng))
        dbg.nospace() << '?';
    else
        dbg.nospace() << lng;
    if (coord.type() == QGeoCoordinate::Coordinate3D) {
        dbg.nospace() << ", ";
        dbg.nospace() << coord.altitude();
    }
    dbg.nospace() << ')';
    return dbg;
}
#endif

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QGeoCoordinate &coordinate)

    \relates QGeoCoordinate

    Writes the given \a coordinate to the specified \a stream.

    \since 1.0
    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &stream, const QGeoCoordinate &coordinate)
{
    stream << coordinate.latitude();
    stream << coordinate.longitude();
    stream << coordinate.altitude();
    return stream;
}
#endif

#ifndef QT_NO_DATASTREAM
/*!
    \fn  QDataStream &operator>>(QDataStream &stream, QGeoCoordinate &coordinate)
    \relates QGeoCoordinate

    Reads a coordinate from the specified \a stream into the given
    \a coordinate.

    \since 1.0
    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &stream, QGeoCoordinate &coordinate)
{
    double value;
    stream >> value;
    coordinate.setLatitude(value);
    stream >> value;
    coordinate.setLongitude(value);
    stream >> value;
    coordinate.setAltitude(value);
    return stream;
}
#endif

QTM_END_NAMESPACE
