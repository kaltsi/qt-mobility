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

//TESTED_COMPONENT=src/systeminfo

#include <QtTest/QtTest>
 #include <QDesktopWidget>

#include "qsysteminfo.h"

QTM_USE_NAMESPACE
class tst_QSystemScreenSaver : public QObject
{
    Q_OBJECT

private slots:
   void initTestCase();
    void tst_screenSaverInhibited();
    void tst_setScreenSaverInhibit();
    void tst_setScreenSaverInhibited();

};

 void tst_QSystemScreenSaver::initTestCase()
 {

 }

void tst_QSystemScreenSaver::tst_screenSaverInhibited()
{
    QSystemScreenSaver si;
    QDesktopWidget wid;
   bool enabled = si.setScreenSaverInhibit();
    if(wid.screenCount() > 0) {
        QVERIFY( si.screenSaverInhibited() && enabled);
    } else{
        QVERIFY(!si.screenSaverInhibited() && !enabled);
    }
}

void tst_QSystemScreenSaver::tst_setScreenSaverInhibit()
{
    QSystemScreenSaver si;
    QDesktopWidget wid;
    bool enabled = si.setScreenSaverInhibit();
    if(wid.screenCount() > 0) {
        QVERIFY(enabled);
    } else{
        QVERIFY(!enabled);
    }
}

void tst_QSystemScreenSaver::tst_setScreenSaverInhibited()
{
    QSystemScreenSaver si;
    QDesktopWidget wid;
    si.setScreenSaverInhibited(true);
    if(wid.screenCount() > 0) {
        QVERIFY(si.screenSaverInhibited());
    } else{
        QVERIFY(!si.screenSaverInhibited());
    }
    si.setScreenSaverInhibited(false);
    QVERIFY(!si.screenSaverInhibited());
}

QTEST_MAIN(tst_QSystemScreenSaver)
#include "tst_qsystemscreensaver.moc"
