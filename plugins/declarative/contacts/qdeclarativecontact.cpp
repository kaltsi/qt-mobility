/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#include "qcontactdetails.h"
#include "qcontactmanager.h"
#include "qdeclarativecontact_p.h"
#include "qdeclarativecontactdetail_p.h"
#include "qdeclarativecontactmetaobject_p.h"
#include "qdeclarativecontactmodel_p.h"
#include <QImage>
#include <QUrl>
#include <QDeclarativeListProperty>

/*!
    \qmlclass Contact QDeclarativeContact

    \brief The Contact element represents an addressbook contact, A Contact
     object has a collection of details (like a name, phone numbers and
     email addresses).  Each detail (which can have multiple fields) is stored
     in an appropriate subclass of ContactDetail, and the Contact allows
     retrieving these details in various ways.

    If some of the contact details are not unique details, all of this type of detail values
    can be accessed by dynamic properties. For example, there are 3 phone numbers stored in
    a contact, they can be accessed by contact.phoneNumbers property, Which holds a list of
    all PhoneNumber details. If the dynamic property was not exists (for unique details), an undefined
    value returned. The list of dynamic detail properties depends on the contact engine implementations.

    \ingroup qml-contacts
    \since Mobility 1.1

    \sa QContact
    The Contact element is part of the \bold{QtMobility.contacts 1.1} module.
*/

QDeclarativeContact::QDeclarativeContact(QObject *parent)
    :QObject(parent),
    d(new QDeclarativeContactMetaObject(this, QContact()))
{
    connect(this, SIGNAL(detailsChanged()), SLOT(setModified()));
}


QDeclarativeContact::QDeclarativeContact(const QContact& contact, const QMap<QString, QContactDetailDefinition>& defs, QObject *parent)
    :QObject(parent),
    d(new QDeclarativeContactMetaObject(this, contact))
{
    setDetailDefinitions(defs);
    connect(this, SIGNAL(detailsChanged()), SLOT(setModified()));
}

QDeclarativeContact::~QDeclarativeContact()
{
    delete d;
}

void QDeclarativeContact::setDetailDefinitions(const QMap<QString, QContactDetailDefinition>& defs)
{
    d->m_defs = defs;
}

QMap<QString, QContactDetailDefinition> QDeclarativeContact::detailDefinitions() const
{
    return d->m_defs;
}

void QDeclarativeContact::setContact(const QContact& contact)
{
   d->setContact(contact);
   d->m_modified = false;
}

QContact QDeclarativeContact::contact() const
{
    return d->contact();
}

/*!
    \qmlproperty bool Contact::modified
    \since Mobility 1.1

    This property holds the dirty flag of the Contact object.
    If the Contact has been changed, returns true, otherwise returns false.
*/
bool QDeclarativeContact::modified() const
{
     return d->m_modified;
}
void QDeclarativeContact::setModified()
{
     d->m_modified = true;
}

/*!
    \qmlproperty enumeration Contact::type
    \since Mobility 1.1

    This property holds type of the Contact, the value can be one of:

    \list
    \o Contact.Contact
    \o Contact.Group
    \endlist
*/
void QDeclarativeContact::setType(QDeclarativeContact::ContactType newType)
{
    if (newType != type()) {
        if (newType == QDeclarativeContact::Contact)
            d->m_contact.setType(QContactType::TypeContact);
        else if (newType == QDeclarativeContact::Group)
            d->m_contact.setType(QContactType::TypeGroup);
        emit detailsChanged();
    }
}

QDeclarativeContact::ContactType QDeclarativeContact::type() const
{
    if (d->m_contact.type() == QContactType::TypeGroup)
        return QDeclarativeContact::Group;
    return QDeclarativeContact::Contact;
}

/*!
    \qmlmethod Contact::removeDetail(detail)
    \since Mobility 1.1

    Removes the give contact \a detail from the contact, returns true if successful, otherwise returns false.
*/

bool QDeclarativeContact::removeDetail(QDeclarativeContactDetail* detail)
{
    if (detail->removable()) {
        d->m_details.removeAll(detail);
        emit detailsChanged();
        return true;
    }
    return false;

}

/*!
    \qmlmethod Contact::addDetail(detail)
    \since Mobility 1.1

    Adds the given contact \a detail to the contact, returns true if successful, otherwise returns false.
    \note: If the \a detail has been added into the same contact before, this operation will be ignored,
    so if you want to add a \a detail multiple times, the \a detail should be copied before calling this function.
*/
bool QDeclarativeContact::addDetail(QDeclarativeContactDetail* detail)
{
    if (detail) {
        if (!d->m_details.contains(detail)) {
            d->m_details.append(detail);
            emit detailsChanged();
        }
        return true;
    }
    return false;
}

/*!
    \qmlproperty list<ContactDetail> Contact::details
    \since Mobility 1.1

    This property holds the list of \l ContactDetail elements that the contact has.
*/
QDeclarativeListProperty<QDeclarativeContactDetail> QDeclarativeContact::details()
{
    return d->details(QString()).value< QDeclarativeListProperty<QDeclarativeContactDetail> >();
}

/*!
    \qmlproperty int Contact::contactId
    \since Mobility 1.1

    This property holds the id of the Contact object.
    This property is read only.
*/
QContactLocalId QDeclarativeContact::contactId() const
{
    return d->localId();
}

/*!
    \qmlproperty string Contact::manager
    \since Mobility 1.1

    This property holds the manager name which the Contact object comes from.
*/
QString QDeclarativeContact::manager() const
{
    return d->contactId().managerUri();
}

/*!
    \qmlmethod ContactDetail Contact::detail(name)
    \since Mobility 1.1

    Returns ContactDetail object which detail name or detail type is \a name.
*/
QDeclarativeContactDetail* QDeclarativeContact::detail(const QVariant& name)
{
    if (name.type() == QVariant::String)
        return d->detail(name.toString()).value<QDeclarativeContactDetail*>();
    else
        return d->detail(static_cast<QDeclarativeContactDetail::ContactDetailType>(name.value<int>())).value<QDeclarativeContactDetail*>();
}

/*!
    \qmlmethod list<ContactDetail> Contact::details(name)
    \since Mobility 1.1

    Returns a list of ContactDetail objects which detail name or detail type is \a name.
*/
QDeclarativeListProperty<QDeclarativeContactDetail> QDeclarativeContact::details(const QVariant& name)
{
    if (name.type() == QVariant::String)
        return d->details(name.toString()).value< QDeclarativeListProperty<QDeclarativeContactDetail> >();
    else
        return d->details(static_cast<QDeclarativeContactDetail::ContactDetailType>(name.value<int>())).value< QDeclarativeListProperty<QDeclarativeContactDetail> >();
}

/*!
    \qmlmethod Contact::clearDetails()
    \since Mobility 1.1

    Remove all detail objects in this contact.
*/
void QDeclarativeContact::clearDetails()
{
    d->m_details.clear();
    emit detailsChanged();
}


/*!
    \qmlmethod Contact::save()
    \since Mobility 1.1

    Saves this Contact if the contact has been modified.

    \sa  Contact::modified
*/
void QDeclarativeContact::save()
{
    if (modified()) {
        QDeclarativeContactModel* model = qobject_cast<QDeclarativeContactModel*>(parent());
        if (model) {
            model->saveContact(this);
        }
    }
}

/*!
    \qmlproperty Address Contact::address
    \since Mobility 1.1

    This property holds the address detail of the Contact object.
*/
QDeclarativeContactAddress* QDeclarativeContact::address()
{
    return static_cast<QDeclarativeContactAddress*>(d->detail(QDeclarativeContactDetail::Address).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Anniversary Contact::anniversary
    \since Mobility 1.1

    This property holds the anniversary detail of the Contact object.
*/
QDeclarativeContactAnniversary* QDeclarativeContact::anniversary()
{
    return static_cast<QDeclarativeContactAnniversary*>(d->detail(QDeclarativeContactDetail::Anniversary).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Avatar Contact::avatar
    \since Mobility 1.1

    This property holds the avatar detail of the Contact object.
*/
QDeclarativeContactAvatar* QDeclarativeContact::avatar()
{
    return static_cast<QDeclarativeContactAvatar*>(d->detail(QDeclarativeContactDetail::Avatar).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Birthday Contact::birthday
    \since Mobility 1.1

    This property holds the birthday detail of the Contact object.
*/
QDeclarativeContactBirthday*  QDeclarativeContact::birthday()
{
    return static_cast<QDeclarativeContactBirthday*>(d->detail(QDeclarativeContactDetail::Birthday).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty string Contact::displayLabel
    \since Mobility 1.1

    This property holds the display label value of the Contact object.
*/
QString QDeclarativeContact::displayLabel()
{
    return d->m_contact.displayLabel();
}

/*!
    \qmlproperty EmailAddress Contact::email
    \since Mobility 1.1

    This property holds the email address detail of the Contact object.
*/
QDeclarativeContactEmailAddress*  QDeclarativeContact::email()
{
    return static_cast<QDeclarativeContactEmailAddress*>(d->detail(QDeclarativeContactDetail::Email).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Family Contact::family
    \since Mobility 1.1

    This property holds the family detail of the Contact object.
*/
QDeclarativeContactFamily*  QDeclarativeContact::family()
{
    return static_cast<QDeclarativeContactFamily*>(d->detail(QDeclarativeContactDetail::Family).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Favorite Contact::favorite
    \since Mobility 1.1

    This property holds the favorite detail of the Contact object.
*/
QDeclarativeContactFavorite*  QDeclarativeContact::favorite()
{
    return static_cast<QDeclarativeContactFavorite*>(d->detail(QDeclarativeContactDetail::Favorite).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Gender Contact::gender
    \since Mobility 1.1

    This property holds the gender detail of the Contact object.
*/
QDeclarativeContactGender*  QDeclarativeContact::gender()
{
    return static_cast<QDeclarativeContactGender*>(d->detail(QDeclarativeContactDetail::Gender).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty GeoLocation Contact::geolocation
    \since Mobility 1.1

    This property holds the geolocation detail of the Contact object.
*/
QDeclarativeContactGeoLocation*  QDeclarativeContact::geolocation()
{
    return static_cast<QDeclarativeContactGeoLocation*>(d->detail(QDeclarativeContactDetail::Geolocation).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty GlobalPresence Contact::globalPresence
    \since Mobility 1.1

    This property holds the globalPresence detail of the Contact object.
*/
QDeclarativeContactGlobalPresence*  QDeclarativeContact::globalPresence()
{
    return static_cast<QDeclarativeContactGlobalPresence*>(d->detail(QDeclarativeContactDetail::GlobalPresence).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Guid Contact::guid
    \since Mobility 1.1

    This property holds the guid detail of the Contact object.
*/
QDeclarativeContactGuid*  QDeclarativeContact::guid()
{
    return static_cast<QDeclarativeContactGuid*>(d->detail(QDeclarativeContactDetail::Guid).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Name Contact::name
    \since Mobility 1.1

    This property holds the name detail of the Contact object.
*/
QDeclarativeContactName*  QDeclarativeContact::name()
{
    return static_cast<QDeclarativeContactName*>(d->detail(QDeclarativeContactDetail::Name).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Nickname Contact::nickname
    \since Mobility 1.1

    This property holds the nickname detail of the Contact object.
*/
QDeclarativeContactNickname*  QDeclarativeContact::nickname()
{
    return static_cast<QDeclarativeContactNickname*>(d->detail(QDeclarativeContactDetail::NickName).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Note Contact::note
    \since Mobility 1.1

    This property holds the note detail of the Contact object.
*/
QDeclarativeContactNote*  QDeclarativeContact::note()
{
    return static_cast<QDeclarativeContactNote*>(d->detail(QDeclarativeContactDetail::Note).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty OnlineAccount Contact::onlineAccount
    \since Mobility 1.1

    This property holds the onlineAccount detail of the Contact object.
*/
QDeclarativeContactOnlineAccount*  QDeclarativeContact::onlineAccount()
{
    return static_cast<QDeclarativeContactOnlineAccount*>(d->detail(QDeclarativeContactDetail::OnlineAccount).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Organization Contact::organization
    \since Mobility 1.1

    This property holds the organization detail of the Contact object.
*/
QDeclarativeContactOrganization*  QDeclarativeContact::organization()
{
    return static_cast<QDeclarativeContactOrganization*>(d->detail(QDeclarativeContactDetail::Organization).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty PhoneNumber Contact::phoneNumber
    \since Mobility 1.1

    This property holds the phoneNumber detail of the Contact object.
*/
QDeclarativeContactPhoneNumber*  QDeclarativeContact::phoneNumber()
{
    return static_cast<QDeclarativeContactPhoneNumber*>(d->detail(QDeclarativeContactDetail::PhoneNumber).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Presence Contact::presence
    \since Mobility 1.1

    This property holds the presence detail of the Contact object.
*/
QDeclarativeContactPresence*  QDeclarativeContact::presence()
{
    return static_cast<QDeclarativeContactPresence*>(d->detail(QDeclarativeContactDetail::Presence).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Ringtone Contact::ringtone
    \since Mobility 1.1

    This property holds the ringtone detail of the Contact object.
*/
QDeclarativeContactRingtone*  QDeclarativeContact::ringtone()
{
    return static_cast<QDeclarativeContactRingtone*>(d->detail(QDeclarativeContactDetail::Ringtone).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty SyncTarget Contact::syncTarget
    \since Mobility 1.1

    This property holds the syncTarget detail of the Contact object.
*/
QDeclarativeContactSyncTarget*  QDeclarativeContact::syncTarget()
{
    return static_cast<QDeclarativeContactSyncTarget*>(d->detail(QDeclarativeContactDetail::SyncTarget).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty Tag Contact::tag
    \since Mobility 1.1

    This property holds the tag detail of the Contact object.
*/
QDeclarativeContactTag*  QDeclarativeContact::tag()
{
    return static_cast<QDeclarativeContactTag*>(d->detail(QDeclarativeContactDetail::Tag).value<QDeclarativeContactDetail*>());
}

/*!
    \qmlproperty url Contact::thumbnail
    \since Mobility 1.1

    This property holds the thumbnail URL of the Contact object.
*/
QUrl QDeclarativeContact::thumbnail() const
{
    //Just let the imager provider deal with it
    return QUrl(QString("image://thumbnail/%1.%2").arg(manager()).arg(contactId()).toLatin1());
}

//Only support local file
void QDeclarativeContact::setThumbnail(const QUrl& url)
{
    QImage image(100, 50, QImage::Format_RGB32);
    image.load(url.toLocalFile());
    QContactThumbnail detail;
    detail.setThumbnail(image);
    d->m_contact.saveDetail(&detail);
    emit detailsChanged();
}

/*!
    \qmlproperty Url Contact::url
    \since Mobility 1.1

    This property holds the url detail of the Contact object.
*/
QDeclarativeContactUrl*  QDeclarativeContact::url()
{
    return static_cast<QDeclarativeContactUrl*>(d->detail(QDeclarativeContactDetail::Url).value<QDeclarativeContactDetail*>());
}


/*!
    \qmlproperty Hobby Contact::hobby
    \since Mobility 1.1

    This property holds the hobby detail of the Contact object.
*/
QDeclarativeContactHobby*  QDeclarativeContact::hobby()
{
    return static_cast<QDeclarativeContactHobby*>(d->detail(QDeclarativeContactDetail::Hobby).value<QDeclarativeContactDetail*>());
}
