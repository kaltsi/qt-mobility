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

#ifndef QDECLARATIVEGEOROUTESEGMENT_H
#define QDECLARATIVEGEOROUTESEGMENT_H

#include "qdeclarativegeomaneuver_p.h"

#include <qgeoroutesegment.h>

#include <QObject>

QTM_BEGIN_NAMESPACE

class QDeclarativeGeoRouteSegment : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int travelTime READ travelTime CONSTANT)
    Q_PROPERTY(qreal distance READ distance CONSTANT)
    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeCoordinate> path READ path)
    Q_PROPERTY(QDeclarativeGeoManeuver* maneuver READ maneuver CONSTANT)

public:
    QDeclarativeGeoRouteSegment(QObject *parent = 0);
    QDeclarativeGeoRouteSegment(const QGeoRouteSegment &segment, QObject *parent = 0);
    ~QDeclarativeGeoRouteSegment();

    int travelTime() const;
    qreal distance() const;
    QDeclarativeListProperty<QDeclarativeCoordinate> path();
    QDeclarativeGeoManeuver* maneuver() const;

private:
    static void path_append(QDeclarativeListProperty<QDeclarativeCoordinate> *prop, QDeclarativeCoordinate *coordinate);
    static int path_count(QDeclarativeListProperty<QDeclarativeCoordinate> *prop);
    static QDeclarativeCoordinate* path_at(QDeclarativeListProperty<QDeclarativeCoordinate> *prop, int index);
    static void path_clear(QDeclarativeListProperty<QDeclarativeCoordinate> *prop);

    QGeoRouteSegment segment_;
    QDeclarativeGeoManeuver* maneuver_;
    QList<QDeclarativeCoordinate*> path_;
};

QTM_END_NAMESPACE

#endif
