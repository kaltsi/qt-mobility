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

#ifndef MEEGOORIENTATIONSENSOR_H
#define MEEGOORIENTATIONSENSOR_H

#include "meegosensorbase.h"
#include <qorientationsensor.h>

#include <orientationsensor_i.h>
#include <unsigned.h>

QTM_USE_NAMESPACE

class meegoorientationsensor : public meegosensorbase
{
    Q_OBJECT

public:
    static char const * const id;
    meegoorientationsensor(QSensor *sensor);
protected:
    virtual bool doConnect();
    virtual QString sensorName() const;
    virtual void start();

private:
    QOrientationReading m_reading;
    static QOrientationReading::Orientation getOrientation(int orientation);
    static bool m_initDone;

private slots:
    void slotDataAvailable(const Unsigned& orientation);
};

#endif
