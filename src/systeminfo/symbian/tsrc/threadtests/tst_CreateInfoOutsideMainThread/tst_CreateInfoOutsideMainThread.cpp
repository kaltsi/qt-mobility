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

#define SHOWDEBUGS

QSemaphore MainThreadRunningSemaphore;
QSemaphore OtherThreadRunningSemaphore;

class ThreadBase : public QThread
{
    Q_OBJECT
public:

    void run() {
#ifdef SHOWDEBUGS
        qDebug()<<"Thread::run - create object in thread: "<<thread()->currentThreadId();
#endif//SHOWDEBUGS
        construct();
#if 0
        qDebug()<<"Thread::run - read something directly from object in this thread: "
                <<networkInfo.currentMode();
#endif//SHOWDEBUGS
        MainThreadRunningSemaphore.release();
    }

    virtual void construct() = 0;
};

class QSystemBatteryInfoThread : public ThreadBase
{
    void construct() {
        QSystemBatteryInfo info;
    }
};

class QSystemDeviceInfoThread : public ThreadBase
{
    void construct() {
        QSystemDeviceInfo info;
    }
};

class QSystemInfoThread : public ThreadBase
{
    void construct() {
        QSystemInfo info;
    }
};

class QSystemNetworkInfoThread : public ThreadBase
{
    void construct() {
        QSystemNetworkInfo info;
    }
};

class QSystemScreenSaverThread : public ThreadBase
{
    void construct() {
        QSystemScreenSaver info;
    }
};

class QSystemStorageInfoThread : public ThreadBase
{
    void construct() {
        QSystemStorageInfo info;
    }
};

class tst_QSystemInfo_CreateInfoOutsideMainThread : public QObject
{
    Q_OBJECT

private:
    void runThread(ThreadBase& testThread) {
        testThread.start();
        MainThreadRunningSemaphore.acquire();
        OtherThreadRunningSemaphore.release();
        testThread.wait();
    }

    void logThread(const QString message) {
#ifdef SHOWDEBUGS
        qDebug() << message << thread()->currentThreadId();
#endif//SHOWDEBUGS
    }

private Q_SLOTS:
    void testBatteryInfo() {
        logThread("running in thread:");
        QSystemBatteryInfoThread testThread;
        runThread(testThread);
    }

    void testDeviceInfo() {
        logThread("running in thread:");
        QSystemDeviceInfoThread testThread;
        runThread(testThread);
    }

    void testInfo() {
        logThread("running in thread:");
        QSystemInfoThread testThread;
        runThread(testThread);
    }

    void testNetworkInfo() {
        logThread("running in thread:");
        QSystemNetworkInfoThread testThread;
        runThread(testThread);
    }

    void testScreenSaver() {
        logThread("running in thread:");
        QSystemScreenSaverThread testThread;
        runThread(testThread);
    }

    void testStorageInfo() {
        logThread("running in thread:");
        QSystemStorageInfoThread testThread;
        runThread(testThread);
    }
};

QTEST_MAIN(tst_QSystemInfo_CreateInfoOutsideMainThread);

#include "tst_CreateInfoOutsideMainThread.moc"
