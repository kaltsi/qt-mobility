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
#ifndef TRANSFORMTHUMBNAIL_H
#define TRANSFORMTHUMBNAIL_H

#include "cnttransformcontactdata.h"

#ifdef SYMBIAN_USE_SMALL_THUMBNAILS
const TSize KMaxThumbnailSize(80, 96);
#else
const TSize KMaxThumbnailSize(150, 150);
#endif

QTM_USE_NAMESPACE

class CntTransformThumbnail : public CntTransformContactData
{
public:
    CntTransformThumbnail();
    ~CntTransformThumbnail();

protected:
	QList<CContactItemField *> transformDetailL(const QContactDetail &detail);
	QContactDetail *transformItemField(const CContactItemField& field, const QContact &contact);
	bool supportsDetail(QString detailName) const;
	QList<TUid> supportedFields() const;
	QList<TUid> supportedSortingFieldTypes(QString detailFieldName) const;
    bool supportsSubType(const QString& subType) const;
    quint32 getIdForField(const QString& fieldName) const;
    void detailDefinitions(QMap<QString, QContactDetailDefinition> &definitions, const QString& contactType) const;
private:    
    void initializeThumbnailFieldL();
private:
    CContactItemField *m_thumbnailFieldFromTemplate;
};

#endif
