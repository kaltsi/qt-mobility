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

#include "camerabinresourcepolicy.h"

#if defined(Q_WS_MAEMO_6) || defined(Q_WS_MEEGO)
#define HAVE_RESOURCE_POLICY
#endif

//#define DEBUG_RESOURCE_POLICY
#include <QtCore/qdebug.h>
#include <QtCore/qset.h>

#ifdef HAVE_RESOURCE_POLICY
#include <policy/resource.h>
#include <policy/resources.h>
#include <policy/resource-set.h>
#endif

#include <QString>

CamerabinResourcePolicy::CamerabinResourcePolicy(QObject *parent) :
    QObject(parent),
    m_errorState(false),
    m_resourceSet(NoResources),
    m_releasingResources(false)
{
#ifdef HAVE_RESOURCE_POLICY
    //loaded resource set is also kept requested for image and video capture sets
    m_resource = new ResourcePolicy::ResourceSet("camera");
    m_resource->setAlwaysReply();
    m_resource->initAndConnect();

    connect(m_resource, SIGNAL(managerIsUp()),
            this, SLOT(handleManagerIsUp()));
    connect(m_resource, SIGNAL(errorCallback(quint32, const char*)),
            this, SLOT(handleError(quint32, const char*)));

    connect(m_resource, SIGNAL(resourcesGranted(const QList<ResourcePolicy::ResourceType>)),
            SIGNAL(resourcesGranted()));
    connect(m_resource, SIGNAL(resourcesDenied()), SIGNAL(resourcesDenied()));
    connect(m_resource, SIGNAL(lostResources()), SIGNAL(resourcesLost()));
    connect(m_resource, SIGNAL(resourcesReleased()), SLOT(handleResourcesReleased()));
#endif
}

CamerabinResourcePolicy::~CamerabinResourcePolicy()
{
#ifdef HAVE_RESOURCE_POLICY
    //ensure the resources are released
    if (m_resourceSet != NoResources)
        setResourceSet(NoResources);

    //don't delete the resource set until resources are released
    if (m_releasingResources) {
        m_resource->connect(m_resource, SIGNAL(resourcesReleased()),
                            SLOT(deleteLater()));
    } else {
        delete m_resource;
        m_resource = 0;
    }
#endif
}

CamerabinResourcePolicy::ResourceSet CamerabinResourcePolicy::resourceSet() const
{
    return m_resourceSet;
}

void CamerabinResourcePolicy::setResourceSet(CamerabinResourcePolicy::ResourceSet set)
{
    CamerabinResourcePolicy::ResourceSet oldSet = m_resourceSet;
    m_resourceSet = set;

#ifdef DEBUG_RESOURCE_POLICY
    qDebug() << Q_FUNC_INFO << set;
#endif

#ifdef HAVE_RESOURCE_POLICY
    QSet<ResourcePolicy::ResourceType> requestedTypes;
    QSet<ResourcePolicy::ResourceType> optionalTypes;

    switch (set) {
    case NoResources:
        break;
    case LoadedResources:
        requestedTypes << ResourcePolicy::VideoRecorderType //to open camera device
                       << ResourcePolicy::SnapButtonType; //to detect capture button events

        optionalTypes  << ResourcePolicy::LensCoverType; //to detect lens cover is opened/closed
        break;
    case VideoCaptureResources:
    case ImageCaptureResources:
        requestedTypes << ResourcePolicy::VideoPlaybackType
                       << ResourcePolicy::VideoRecorderType;

        optionalTypes  << ResourcePolicy::LensCoverType
                       << ResourcePolicy::ScaleButtonType
                       << ResourcePolicy::LedsType
                       << ResourcePolicy::SnapButtonType;
        break;
    case VideoRecordingResources:
        requestedTypes << ResourcePolicy::VideoPlaybackType
                       << ResourcePolicy::VideoRecorderType;

        //audio resources are marked as optional to avoid losing the resource
        //on video recording requested
        optionalTypes  << ResourcePolicy::LensCoverType
                       << ResourcePolicy::ScaleButtonType
                       << ResourcePolicy::LedsType
                       << ResourcePolicy::SnapButtonType
                       << ResourcePolicy::AudioRecorderType
                       << ResourcePolicy::AudioPlaybackType;
        break;
    }

    QSet<ResourcePolicy::ResourceType> allRequestedTypes = requestedTypes+optionalTypes;

    QSet<ResourcePolicy::ResourceType> currentTypes;
    foreach (ResourcePolicy::Resource *resource, m_resource->resources())
        currentTypes << resource->type();

    foreach (ResourcePolicy::ResourceType resourceType, currentTypes - allRequestedTypes)
        m_resource->deleteResource(resourceType);

    foreach (ResourcePolicy::ResourceType resourceType, allRequestedTypes - currentTypes) {
        ResourcePolicy::Resource *resource = 0;
        switch (resourceType) {
        case ResourcePolicy::VideoPlaybackType:
            resource = new ResourcePolicy::VideoResource(QCoreApplication::applicationPid());
            break;
        case ResourcePolicy::VideoRecorderType:
            resource = new ResourcePolicy::VideoRecorderResource;
            break;
        case ResourcePolicy::ScaleButtonType:
            resource = new ResourcePolicy::ScaleButtonResource;
            break;
        case ResourcePolicy::LedsType:
            resource = new ResourcePolicy::LedsResource;
            break;
        case ResourcePolicy::SnapButtonType:
            resource = new ResourcePolicy::SnapButtonResource;
            break;
        case ResourcePolicy::LensCoverType:
            resource = new ResourcePolicy::LensCoverResource;
            break;
        case ResourcePolicy::AudioPlaybackType:
        {
            ResourcePolicy::AudioResource *audioResource = new ResourcePolicy::AudioResource(QLatin1String("camera"));
            audioResource->setProcessID(QCoreApplication::applicationPid());
            resource = audioResource;
            break;
        }
        case ResourcePolicy::AudioRecorderType:
            resource = new ResourcePolicy::AudioRecorderResource;
            break;
        default:
            qWarning() << "unhandled resource type:" << resourceType;
        }

        if (!resource)
            continue;

        if (optionalTypes.contains(resourceType))
            resource->setOptional(true);

        m_resource->addResourceObject(resource);
    }

    if (m_errorState) {
        emit resourcesGranted();
        return;
    }

    m_resource->update();
    if (set != NoResources) {
        m_resource->acquire();
    } else {
        if (oldSet != NoResources) {
            m_releasingResources = true;
            m_resource->release();
        }
    }
#endif
}

bool CamerabinResourcePolicy::isResourcesGranted() const
{
    // If connection to resource manager is broken resources are always granted
    // if there are any requested for.
    if (m_errorState) {
        return m_resourceSet != NoResources;
    }

#ifdef HAVE_RESOURCE_POLICY
    foreach (ResourcePolicy::Resource *resource, m_resource->resources())
        if (!resource->isOptional() && !resource->isGranted())
            return false;
#endif
    return true;
}

void CamerabinResourcePolicy::handleResourcesReleased()
{
#ifdef HAVE_RESOURCE_POLICY
#ifdef DEBUG_RESOURCE_POLICY
    qDebug() << Q_FUNC_INFO;
#endif
    m_releasingResources = false;
#endif
}

void CamerabinResourcePolicy::handleError(quint32 error, const char *error_msg)
{
#ifdef HAVE_RESOURCE_POLICY
    if (error_msg) {
        QString str = QString::fromAscii(error_msg);

        if (str == "DBus.Error.ServiceUnknown") {
            // Resource manager is not running or cannot connect to it.
            m_errorState = true;
        }
    }
#endif
}

void CamerabinResourcePolicy::handleManagerIsUp()
{
#ifdef HAVE_RESOURCE_POLICY

#ifdef DEBUG_RESOURCE_POLICY
    qDebug() << Q_FUNC_INFO << "Connected to Manager";
#endif

    bool oldErrorState = m_errorState;
    m_errorState = false;

    // If state was previously in error and user has requested resources already,
    // try to acquire them now for real.
    if (oldErrorState && m_resourceSet != NoResources) {
        m_resource->acquire();
    }
#endif
}
