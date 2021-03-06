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
#ifndef QSIMULATORGALLERYRESULTSET_H
#define QSIMULATORGALLERYRESULTSET_H

#include <qmobilityglobal.h>
#include "docgalleryconnection_simulator.h"
#include "qgalleryresultset.h"
#include "qgalleryqueryrequest.h"
#include "qgalleryitemrequest.h"
#include <QtCore/QObject>
#include <QtCore/QFileInfo>
#include <QtGui/QImage>

QTM_BEGIN_NAMESPACE

class QSimulatorGalleryResultSet  : public QGalleryResultSet
{
    Q_OBJECT
public:
    explicit QSimulatorGalleryResultSet(QObject *parent = 0);

    ~QSimulatorGalleryResultSet();

    int propertyKey(const QString &property) const;
    QGalleryProperty::Attributes propertyAttributes(int key) const ;
    QVariant::Type propertyType(int key) const;

    int itemCount() const;

    bool isValid() const;

    QVariant itemId() const;
    QUrl itemUrl() const;
    QString itemType() const;
    QVariant metaData(int key) const;
    bool setMetaData(int key, const QVariant &value);

    int currentIndex() const;
    bool fetch(int index);

signals:

public slots:

private:
    QFileInfo currentFileInfo() const;

    Simulator::DocGalleryConnection* connection;
    QGalleryQueryRequest* queryRequest;
    QGalleryItemRequest* itemRequest;

    QString filePath;
    QString itemTypeString;
    QImage image;
    bool valid;
    int mCurrentIndex;
};

QTM_END_NAMESPACE

#endif // QSIMULATORGALLERYRESULTSET_H
