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

#ifndef _TELEPATHYHELPERS_MAEMO6_H_
#define _TELEPATHYHELPERS_MAEMO6_H_

#include <QDebug>

//controls TelepathyEngine and StorageEngine debug info printing
//#define PRINT_DEBUG_INFO

//controls printing of "begin" and "end" markers in TelepathyEngine and StorageEngine
//#define PRINT_DEBUG_BEGIN_END

#ifdef PRINT_DEBUG_INFO
#define QDEBUG(x) qDebug() << x
#define QPRETTYDEBUG(x) qDebug() << __PRETTY_FUNCTION__ << x
#else
#define QDEBUG(x)
#define QPRETTYDEBUG(x)
#endif

#ifdef PRINT_DEBUG_BEGIN_END
#define QDEBUG_FUNCTION_BEGIN qDebug() << __PRETTY_FUNCTION__ << "begin" << endl;
#define QDEBUG_FUNCTION_END qDebug() << __PRETTY_FUNCTION__ << "end" << endl;
#else
#define QDEBUG_FUNCTION_BEGIN
#define QDEBUG_FUNCTION_END
#endif

#endif //_TELEPATHYHELPERS_MAEMO6_H_
