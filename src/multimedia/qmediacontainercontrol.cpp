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


#include "qmediacontainercontrol.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMediaContainerControl

    \brief The QMediaContainerControl class provides access to the output container format of a QMediaService

    \inmodule QtMultimediaKit
    \ingroup multimedia-serv
    \since 1.0

    If a QMediaService supports writing encoded data it will implement
    QMediaContainerControl.  This control provides information about the output
    containers supported by a media service and allows one to be selected as
    the current output containers.

    The functionality provided by this control is exposed to application code
    through the QMediaRecorder class.

    The interface name of QMediaContainerControl is \c com.nokia.Qt.QMediaContainerControl/1.0 as
    defined in QMediaContainerControl_iid.

    \sa QMediaService::requestControl(), QMediaRecorder
*/

/*!
    \macro QMediaContainerControl_iid

    \c com.nokia.Qt.QMediaContainerControl/1.0

    Defines the interface name of the QMediaContainerControl class.

    \relates QMediaContainerControl
*/

/*!
    Constructs a new media container control with the given \a parent.
*/
QMediaContainerControl::QMediaContainerControl(QObject *parent)
    :QMediaControl(parent)
{
}

/*!
    Destroys a media container control.
*/
QMediaContainerControl::~QMediaContainerControl()
{
}


/*!
    \fn QMediaContainerControl::supportedContainers() const

    Returns a list of MIME types of supported container formats.
    \since 1.0
*/

/*!
    \fn QMediaContainerControl::containerMimeType() const

    Returns the MIME type of the selected container format.
    \since 1.0
*/

/*!
    \fn QMediaContainerControl::setContainerMimeType(const QString &mimeType)

    Sets the current container format to the format identified by the given \a mimeType.
    \since 1.0
*/

/*!
    \fn QMediaContainerControl::containerDescription(const QString &mimeType) const

    Returns a description of the container format identified by the given \a mimeType.
    \since 1.0
*/

#include "moc_qmediacontainercontrol.cpp"
QT_END_NAMESPACE

