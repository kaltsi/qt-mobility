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

#ifndef CNTSIMSTORE_H_
#define CNTSIMSTORE_H_

#include <e32base.h>
#ifdef SYMBIANSIM_BACKEND_USE_ETEL_TESTSERVER
#include <mpbutil_etel_test_server.h>
#else
#include <mpbutil.h>
#endif
#include <QObject>
#include <qcontactmanager.h>

QTM_USE_NAMESPACE

struct SimStoreInfo{
    QString m_storeName;
    int m_totalEntries;
    int m_usedEntries;
    bool m_readOnlyAccess;
    bool m_numberSupported;
    bool m_nameSupported;
    bool m_secondNameSupported;
    bool m_additionalNumberSupported;
    bool m_emailSupported;
};

class CntSimStorePrivate;
class CntSymbianSimEngine;

class CntSimStore : public QObject
{
Q_OBJECT
public:
    CntSimStore(CntSymbianSimEngine* engine, QString storeName, QContactManager::Error* error);
    ~CntSimStore();
    
    SimStoreInfo storeInfo();
    bool read(int index, int numSlots, QContactManager::Error* error);
    bool write(const QContact &contact, QContactManager::Error* error);
    bool remove(int index, QContactManager::Error* error);
    bool getReservedSlots(QContactManager::Error* error);
    void cancel();
    bool isBusy();
    TInt lastAsyncError();

signals:
    // NOTE: Use Qt::QueuedConnection as connection type to make signals asynchronous.
    // CntSimStorePrivate emitting these signals is an active object and emitting
    // signals synchronously will corrupt the AO state.
    void readComplete(QList<QContact> contacts, QContactManager::Error error);
    void writeComplete(QContact contacts, QContactManager::Error error);
    void removeComplete(QContactManager::Error error);
    void getReservedSlotsComplete(QList<int> reservedSlots, QContactManager::Error error);
    
private:
    CntSimStorePrivate *d_ptr;
    friend class CntSimStorePrivate;    
};

#endif // CNTSIMSTORE_H_
