/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qorganizeriteminvalidbackend_p.h"

#include <QSharedData>

/*!
  \class QOrganizerItemInvalidEngine
  \brief The QOrganizerItemInvalidEngine class provides an implementation of
  QOrganizerManagerEngine whose functions always return an error.
  \inmodule QtOrganizer

  The invalid engine may be used as a reference or test engine, but does nothing.
  \since 1.1
 */

QTM_BEGIN_NAMESPACE

/*! Constructs a new invalid organizeritems backend. */
QOrganizerItemInvalidEngine::QOrganizerItemInvalidEngine()
{
}

/*! \reimp
  \since 1.1
*/
QString QOrganizerItemInvalidEngine::managerName() const
{
    return QString(QLatin1String("invalid"));
}

QTM_END_NAMESPACE
