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

#ifndef S60FORMATSUPPORTED_H
#define S60FORMATSUPPORTED_H

#include <mmf/common/mmfcontrollerpluginresolver.h>
#include <mmf/server/mmfdatasourcesink.hrh>
#include <qstringlist.h>
#include <badesca.h>
#include <qstring.h>

class S60FormatSupported
{
public:
    S60FormatSupported();
    ~S60FormatSupported();

    QStringList supportedPlayMimeTypesL();

private:
    CMMFFormatSelectionParameters* m_playformatparam;
    CMMFControllerPluginSelectionParameters* m_controllerparam;
    QStringList m_supportedplaymime;
};

#endif // S60FORMATSUPPORTED_H
