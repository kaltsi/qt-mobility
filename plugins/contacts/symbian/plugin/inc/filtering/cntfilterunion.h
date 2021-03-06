/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef CNTFILTERUNION_H
#define CNTFILTERUNION_H

#include "cntabstractcontactfilter.h"
#include "cntsymbiansrvconnection.h"
#include "cntdbinfo.h"

#include <qmobilityglobal.h>

QTM_USE_NAMESPACE

class CntFilterUnion : public CntAbstractContactFilter
{
public:
    CntFilterUnion(CContactDatabase& contactDatabase,CntSymbianSrvConnection &cntServer,CntDbInfo& dbInfo);
    virtual ~CntFilterUnion();
    QList<QContactLocalId> contacts(
            const QContactFilter &filter,
            const QList<QContactSortOrder> &sortOrders,
            bool &filterSupported,
            QContactManager::Error* error) ;
    bool filterSupported(const QContactFilter& filter) ;
    
    void createSelectQuery(const QContactFilter& filter,
                                 QString& selectquery,
                                 QContactManager::Error* error);
#ifdef PBK_UNIT_TEST
    void emulateBestMatching();
#endif //PBK_UNIT_TEST

private:
    void getSelectQueryforFilter(const QContactFilter& filter,QString& sqlSelectQuery,QContactManager::Error* error);
    
protected:
    CContactDatabase& m_contactdatabase;
    CntSymbianSrvConnection &m_srvConnection;
    CntDbInfo& m_dbInfo;
    bool m_emulateBestMatching; //PBK_UNIT_TEST
};

#endif // CNTFILTERUNION_H
