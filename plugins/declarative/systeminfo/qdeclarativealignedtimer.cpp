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


#include "qdeclarativealignedtimer_p.h"
#include "qsystemgeneralinfo.h"

QT_BEGIN_NAMESPACE
/*!
    \qmlclass AlignedTimer QDeclarativeAlignedTimer
    \brief The AlignedTimer element allows applications to synchronize activity,
     such as network access, like checking for updates.


    \inherits QSystemAlignedTimer

    \ingroup qml-systeminfo
    \since Mobility 1.2

    This element is part of the \bold{QtMobility.systeminfo 1.2} module.
    It is a convience class to make QML usage easier.

    \sa QSystemAlignedTimer
    */
/*!
  \qmlproperty int AlignedTimer::minimumInterval
 The timers's minimumInterval.
 \since Mobility 1.2

  Returns this current timer minimum interval.
  */

/*!
  \qmlproperty int AlignedTimer::maximumInterval
  The timers's maximumInterval.
  \since Mobility 1.2

  Returns this current timer maximum interval.
  */
/*!
  \qmlproperty bool AlignedTimer::singleShot
  \since Mobility 1.2
  Whether the timer is single shot.
*/

/*!
    \qmlproperty bool AlignedTimer::running
    \since Mobility 1.2
    If the timer is running; otherwise false.
*/

/*!
  Constructs the AlignedTimer
  \since Mobility 1.2
  */

QDeclarativeAlignedTimer::QDeclarativeAlignedTimer(QObject *parent) :
    QSystemAlignedTimer(parent)
{
}

/*!
   Sets the timer if \a running is true, otherwise stops it.
   \since Mobility 1.2
   */
void QDeclarativeAlignedTimer::setRunning(bool running)
{

}
/*
Returns if the timer is running.
*/
 bool QDeclarativeAlignedTimer::running() const
 {
     return false;
 }


