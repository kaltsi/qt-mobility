/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef QCONTACTMANAGERENGINE_H
#define QCONTACTMANAGERENGINE_H

#include <QSharedData>
#include <QMap>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QObject>

#include "qcontact.h"
#include "qcontactdetaildefinition.h"
#include "qcontactmanager.h"
#include "qcontactmanagerinfo.h"
#include "qcontactabstractrequest.h"

class QContactFilter;
class QContactSortOrder;

class QTCONTACTS_EXPORT QContactManagerEngine : public QObject
{
    Q_OBJECT

public:
    QContactManagerEngine() {}
    virtual void deref() = 0;

    /* URI reporting */
    virtual QString managerName() const;                       // e.g. "Symbian"
    virtual QMap<QString, QString> managerParameters() const;  // e.g. "filename=private.db"
    QString managerUri() const;

    /* Filtering */
    virtual QList<QContactLocalId> contacts(const QContactFilter& filter, const QList<QContactSortOrder>& sortOrders, QContactManager::Error& error) const;

    /* Contacts - Accessors and Mutators */
    virtual QList<QContactLocalId> contacts(const QList<QContactSortOrder>& sortOrders, QContactManager::Error& error) const;
    virtual QContact contact(const QContactLocalId& contactId, QContactManager::Error& error) const;
    virtual QList<QContactLocalId> contacts(const QString& contactType, const QList<QContactSortOrder>& sortOrders, QContactManager::Error& error) const;

    virtual bool saveContact(QContact* contact, QContactManager::Error& error);
    virtual QList<QContactManager::Error> saveContacts(QList<QContact>* contacts, QContactManager::Error& error);

    virtual bool removeContact(const QContactLocalId& contactId, QContactManager::Error& error);
    virtual QList<QContactManager::Error> removeContacts(QList<QContactLocalId>* contactIds, QContactManager::Error& error);

    /* Synthesise the display label of a contact */
    virtual QString synthesiseDisplayLabel(const QContact& contact, QContactManager::Error& error) const;

    /* "Self" contact id (MyCard) */
    virtual bool setSelfContactId(const QContactLocalId& contactId, QContactManager::Error& error);
    virtual QContactLocalId selfContactId(QContactManager::Error& error) const;

    /* Relationships between contacts */
    virtual QList<QContactRelationship> relationships(const QString& relationshipType, const QContactId& participantId, QContactRelationshipFilter::Role role, QContactManager::Error& error) const;
    virtual bool saveRelationship(QContactRelationship* relationship, QContactManager::Error& error);
    virtual QList<QContactManager::Error> saveRelationships(QList<QContactRelationship>* relationships, QContactManager::Error& error);
    virtual bool removeRelationship(const QContactRelationship& relationship, QContactManager::Error& error);
    virtual QList<QContactManager::Error> removeRelationships(const QList<QContactRelationship>& relationships, QContactManager::Error& error);

    /* Validation for saving */
    virtual bool validateContact(const QContact& contact, QContactManager::Error& error) const;
    virtual bool validateDefinition(const QContactDetailDefinition& def, QContactManager::Error& error) const;

    /* Definitions - Accessors and Mutators */
    virtual QMap<QString, QContactDetailDefinition> detailDefinitions(QContactManager::Error& error) const;
    virtual QContactDetailDefinition detailDefinition(const QString& definitionId, QContactManager::Error& error) const;
    virtual bool saveDetailDefinition(const QContactDetailDefinition& def, QContactManager::Error& error);
    virtual bool removeDetailDefinition(const QString& definitionId, QContactManager::Error& error);

    /* Asynchronous Request Support */
    virtual void requestDestroyed(QContactAbstractRequest* req);
    virtual bool startRequest(QContactAbstractRequest* req);
    virtual bool cancelRequest(QContactAbstractRequest* req);
    virtual bool waitForRequestProgress(QContactAbstractRequest* req, int msecs);
    virtual bool waitForRequestFinished(QContactAbstractRequest* req, int msecs);
    static void updateRequestStatus(QContactAbstractRequest* req, QContactManager::Error error, QList<QContactManager::Error>& errors, QContactAbstractRequest::Status status, bool appendOnly = false);
    static void updateRequest(QContactAbstractRequest* req, const QList<QContactLocalId>& result, QContactManager::Error error, const QList<QContactManager::Error>& errors, QContactAbstractRequest::Status status, bool appendOnly = false);
    static void updateRequest(QContactAbstractRequest* req, const QList<QContact>& result, QContactManager::Error error, const QList<QContactManager::Error>& errors, QContactAbstractRequest::Status status, bool appendOnly = false);
    static void updateRequest(QContactAbstractRequest* req, const QList<QContactDetailDefinition>& result, QContactManager::Error error, const QList<QContactManager::Error>& errors, QContactAbstractRequest::Status status);
    static void updateRequest(QContactAbstractRequest* req, const QMap<QString, QContactDetailDefinition>& result, QContactManager::Error error, const QList<QContactManager::Error>& errors, QContactAbstractRequest::Status status, bool appendOnly = false);
    static void updateRequest(QContactAbstractRequest* req, const QList<QContactRelationship>& result, QContactManager::Error error, const QList<QContactManager::Error>& errors, QContactAbstractRequest::Status status, bool appendOnly = false);

    /* Capabilities reporting */
    virtual bool hasFeature(QContactManagerInfo::ManagerFeature feature) const;
    virtual bool filterSupported(const QContactFilter& filter) const;
    virtual QList<QVariant::Type> supportedDataTypes() const;
    virtual QStringList supportedRelationshipTypes() const;

    /* Reports the built-in definitions from the schema */
    static QMap<QString, QContactDetailDefinition> schemaDefinitions();

signals:
    void dataChanged();
    void contactsAdded(const QList<QContactLocalId>& contactIds);
    void contactsChanged(const QList<QContactLocalId>& contactIds);
    void contactsRemoved(const QList<QContactLocalId>& contactIds);
    void relationshipsAdded(const QList<QContactLocalId>& affectedContactIds);
    void relationshipsRemoved(const QList<QContactLocalId>& affectedContactIds);

public:
    /* Helper functions */
    static int compareContact(const QContact& a, const QContact& b, const QList<QContactSortOrder>& sortOrders);
    static void addSorted(QList<QContact>* sorted, const QContact& toAdd, const QList<QContactSortOrder>& sortOrders);
    static int compareVariant(const QVariant& first, const QVariant& second, Qt::CaseSensitivity sensitivity);
    static bool testFilter(const QContactFilter& filter, const QContact& contact);
    static bool validateActionFilter(const QContactFilter& filter);
    static QList<QContactLocalId> sortContacts(const QList<QContact>& contacts, const QList<QContactSortOrder>& sortOrders);
    static void setContactRelationships(QContact* contact, const QList<QContactRelationship>& relationships);

private:
    /* QContactChangeSet is a utility class used to emit the appropriate signals */
    friend class QContactChangeSet;
};

#endif