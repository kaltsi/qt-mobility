/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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

#include <QtTest/QtTest>
#include <qsysteminfo.h>

QTM_USE_NAMESPACE

//#define SHOWDEBUGS

QSemaphore MainThreadRunningSemaphore;

class Thread : public QThread
{
    Q_OBJECT
public:
    void run() {
#ifdef SHOWDEBUGS
        qDebug()<<"Thread::run - create QSystemNetworkInfo - running in thread: "<<thread()->currentThreadId();
#endif//SHOWDEBUGS
        QSystemNetworkInfo networkInfo;
        MainThreadRunningSemaphore.release();
    }
};

class tst_QSystemInfo_CreateDifferentInfosInDifferentThreads : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCase() {
#ifdef SHOWDEBUGS
        qDebug()<<"CreateInfosInDifferentThreads::testCase - create QSystemDeviceInfo - running in thread: "<<thread()->currentThreadId();
#endif//SHOWDEBUGS
        QSystemDeviceInfo deviceInfo;
        Thread testThread;
        testThread.start();
        MainThreadRunningSemaphore.acquire();
        testThread.wait();
    }
};

QTEST_MAIN(tst_QSystemInfo_CreateDifferentInfosInDifferentThreads);

#include "tst_CreateDifferentInfosInDifferentThreads.moc"
