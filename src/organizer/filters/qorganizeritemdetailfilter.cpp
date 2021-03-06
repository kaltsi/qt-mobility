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

#include "qorganizeritemdetailfilter.h"
#include "qorganizeritemdetailfilter_p.h"
#include "qorganizeritemfilter_p.h"
#include "qorganizermanager.h"

QTM_BEGIN_NAMESPACE

/*!
  \class QOrganizerItemDetailFilter
  \brief The QOrganizerItemDetailFilter class provides a filter based around a detail value criterion
  \inmodule QtOrganizer
  \ingroup organizer-filters
  \since 1.1

  It may be used to select organizeritems which contain a detail of a particular definition with a particular value
 */

Q_IMPLEMENT_ORGANIZERITEMFILTER_PRIVATE(QOrganizerItemDetailFilter);

/*!
 * \fn QOrganizerItemDetailFilter::QOrganizerItemDetailFilter(const QOrganizerItemFilter& other)
 * Constructs a copy of \a other if possible, otherwise constructs a new detail filter
   \since 1.1
 */

/*!
 * Constructs a new detail filter
   \since 1.1
 */
QOrganizerItemDetailFilter::QOrganizerItemDetailFilter()
    : QOrganizerItemFilter(new QOrganizerItemDetailFilterPrivate)
{
}

/*!
 * Sets the name of the detail definition of which details will be matched to \a definitionName, and the name of the field in
 * details of that definition which will contain the value criterion to \a fieldName
 * \sa detailDefinitionName()
   \since 1.1
 */
void QOrganizerItemDetailFilter::setDetailDefinitionName(const QString& definitionName, const QString& fieldName)
{
    Q_D(QOrganizerItemDetailFilter);
    d->m_defId = definitionName;
    d->m_fieldId = fieldName;
}

/*!
 * Sets the value criterion of the filter to \a value
 * \sa value()
   \since 1.1
 */
void QOrganizerItemDetailFilter::setValue(const QVariant& value)
{
    Q_D(QOrganizerItemDetailFilter);
    d->m_exactValue = value;
}

/*!
 * Sets the semantics of the value matching criterion to those defined in \a flags
 * \sa matchFlags()
   \since 1.1
 */
void QOrganizerItemDetailFilter::setMatchFlags(QOrganizerItemFilter::MatchFlags flags)
{
    Q_D(QOrganizerItemDetailFilter);
    d->m_flags = flags;
}

/*!
 * Returns the semantics of the value matching criterion
 * \sa setMatchFlags()
   \since 1.1
 */
QOrganizerItemFilter::MatchFlags QOrganizerItemDetailFilter::matchFlags() const
{
    Q_D(const QOrganizerItemDetailFilter);
    return d->m_flags;
}

/*!
 * Returns the definition name of the details which will be inspected for matching values
 * \sa setDetailDefinitionName()
   \since 1.1
 */
QString QOrganizerItemDetailFilter::detailDefinitionName() const
{
    Q_D(const QOrganizerItemDetailFilter);
    return d->m_defId;
}

/*!
 * Returns the name of the field which contains the value which will be matched against the value criterion
 * \sa setDetailDefinitionName()
   \since 1.1
 */
QString QOrganizerItemDetailFilter::detailFieldName() const
{
    Q_D(const QOrganizerItemDetailFilter);
    return d->m_fieldId;
}

/*!
 * Returns the value criterion of the detail filter
 * \sa setValue()
   \since 1.1
 */
QVariant QOrganizerItemDetailFilter::value() const
{
    Q_D(const QOrganizerItemDetailFilter);
    return d->m_exactValue;
}

QTM_END_NAMESPACE
