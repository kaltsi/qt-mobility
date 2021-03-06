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
 ** This file is part of the Ovi services plugin for the Maps and
 ** Navigation API.  The use of these services, whether by use of the
 ** plugin or by other means, is governed by the terms and conditions
 ** described by the file OVI_SERVICES_TERMS_AND_CONDITIONS.txt in
 ** this package, located in the directory containing the Ovi services
 ** plugin source code.
 **
 ****************************************************************************/
#ifndef QGEOMAPDATA_NOKIA_H
#define QGEOMAPDATA_NOKIA_H

#include <qgeotiledmapdata.h>
#include <QPixmap>
#include <QNetworkReply>

QTM_USE_NAMESPACE

class QGeoMappingManagerEngineNokia;

struct CopyrightDescriptor
{
    qreal maxLevel;
    QString alt;
    QString label;
    qreal minLevel;
    QList<QGeoBoundingBox> boxes;
};

enum LogoPosition
{
    TopLeft = 0,
    ShiftedRight = 1,
    ShiftedDown = 2
};

class QGeoTiledMapDataNokia: public QGeoTiledMapData
{
Q_OBJECT
public:
    QGeoTiledMapDataNokia(QGeoMappingManagerEngineNokia *engine);
    virtual ~QGeoTiledMapDataNokia();
    void paintProviderNotices(QPainter *painter, const QStyleOptionGraphicsItem *option);

    QString getViewCopyright();

private:
    Q_DISABLE_COPY(QGeoTiledMapDataNokia)

    QPixmap watermark;

    QNetworkAccessManager *m_networkManager;

    QHash<QString, QList<CopyrightDescriptor> > copyrights;
    int m_logoPosition;

private slots:
    void copyrightReplyFinished(QNetworkReply*);
};

#endif // QGEOMAPDATA_NOKIA_H
