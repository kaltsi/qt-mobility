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

#ifndef S60CAMERALOCKSCONTROL_H
#define S60CAMERALOCKSCONTROL_H

#include <QtCore/qobject.h>
#include "qcameralockscontrol.h"

QT_USE_NAMESPACE

QT_FORWARD_DECLARE_CLASS(S60CameraService)
QT_FORWARD_DECLARE_CLASS(S60ImageCaptureSession)
QT_FORWARD_DECLARE_CLASS(S60CameraFocusControl)

/*
 * Control for searching and locking 3A algorithms (AutoFocus, AutoExposure
 * and AutoWhitebalance).
 */
class S60CameraLocksControl : public QCameraLocksControl
{
    Q_OBJECT

public: // Contructors & Destrcutor

    S60CameraLocksControl(QObject *parent = 0);
    S60CameraLocksControl(S60CameraService *service,
                          S60ImageCaptureSession *session,
                          QObject *parent = 0);
    ~S60CameraLocksControl();

public: // QCameraLocksControl

    QCamera::LockTypes supportedLocks() const;

    QCamera::LockStatus lockStatus(QCamera::LockType lock) const;

    void searchAndLock(QCamera::LockTypes locks);
    void unlock(QCamera::LockTypes locks);

/*
Q_SIGNALS: // QCameraLocksControl

    void lockStatusChanged(QCamera::LockType type,
                           QCamera::LockStatus status,
                           QCamera::LockChangeReason reason);
*/

private slots: // Internal Slots

    void focusStatusChanged(QCamera::LockStatus status, QCamera::LockChangeReason reason);

private: // Internal

    // Focus
    void startFocusing();
    void cancelFocusing();

private: // Data

    S60ImageCaptureSession  *m_session;
    S60CameraService        *m_service;
    S60CameraFocusControl   *m_focusControl;
    QCamera::LockStatus     m_focusStatus;
    QCamera::LockStatus     m_exposureStatus;
    QCamera::LockStatus     m_whiteBalanceStatus;
};

#endif // S60CAMERALOCKSCONTROL_H
