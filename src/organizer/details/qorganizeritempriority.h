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


#ifndef QORGANIZERITEMPRIORITY_H
#define QORGANIZERITEMPRIORITY_H

#include <QtDebug>
#include <QString>

#include "qtorganizerglobal.h"
#include "qorganizeritemdetail.h"
#include "qorganizeritem.h"
#include "qorganizeritemfilter.h"

QTM_BEGIN_NAMESPACE

/* Leaf class */
class Q_ORGANIZER_EXPORT QOrganizerItemPriority : public QOrganizerItemDetail
{
public:
#ifdef Q_QDOC
    static const QLatin1Constant DefinitionName;
    static const QLatin1Constant FieldPriority;
#else
    Q_DECLARE_CUSTOM_ORGANIZER_DETAIL(QOrganizerItemPriority, "Priority")
    Q_DECLARE_LATIN1_CONSTANT(FieldPriority, "Priority");
#endif

    // time vs impact priority?  greater granularity?
    enum Priority {
        UnknownPriority = 0,
        HighestPriority = 1,
        ExtremelyHighPriority = 2,
        VeryHighPriority = 3,
        HighPriority = 4,
        MediumPriority = 5,
        LowPriority = 6,
        VeryLowPriority = 7,
        ExtremelyLowPriority = 8,
        LowestPriority = 9
    };

    void setPriority(Priority priority) {setValue(FieldPriority, static_cast<int>(priority));}
    Priority priority() const {return static_cast<Priority>(value<int>(FieldPriority));}

    // Convenience filter
    static QOrganizerItemFilter match(Priority priority);
};

QTM_END_NAMESPACE

#endif

