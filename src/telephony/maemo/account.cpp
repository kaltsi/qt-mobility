/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QDebug>

#include "qtelephony.h"
#include "maemo/account.h"
#include "maemo/constants.h"
#include "maemo/qtelephonycalllist_maemo_p.h"
#include "maemo/pendingoperation.h"

namespace DBus
{
    const Feature Account::FeatureCore = Feature(QLatin1String(Account::staticMetaObject.className()), 0, true);

    Account::Account(QDBusConnection busconnection, const QString busname, const QString &objectPath, QTelephonyCallListPrivate* callList)
        : StatefullDBusProxy(busconnection, busname, objectPath)
        , ReadyObject(this, FeatureCore)
        , ptelephonyCallList(callList)
    {
        qDebug() << " Account::Account(";
        qDebug() << "- QDBusConnection base service: " << busconnection.baseService();
        qDebug() << "- QDBusConnection name: " << busconnection.name();
        qDebug() << "- objectPath: " << objectPath;
        qDebug() << "- DBusProxy.busName: " << this->busName();
        qDebug() << "- DBusProxy.objectPath: " << this->objectPath();

        //Create Account Manager interface
        pIAccount = new DBus::Interfaces::IAccount(this->dbusConnection(),this->busName(), this->objectPath());
        QString connectionpath = pIAccount->Connection().path();
        qDebug() << "- connection bus " << connectionpath;
        qDebug() << "- connection path " << connectionpath;
        connection = ConnectionPtr(new Connection(QDBusConnection::sessionBus(), PATH2BUS(connectionpath), connectionpath, ptelephonyCallList));
    }

    Account::~Account()
    {
        qDebug() << "Account::~Account";
        if(pIAccount)
            delete pIAccount;
    }
}