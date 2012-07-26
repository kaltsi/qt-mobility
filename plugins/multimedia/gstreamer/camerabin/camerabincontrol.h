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


#ifndef CAMERABINCONTROL_H
#define CAMERABINCONTROL_H

#include <QHash>
#include <QSize>
#include <qcameracontrol.h>
#include <qmediarecorder.h>
#include "camerabinsession.h"

QT_USE_NAMESPACE

class CamerabinResourcePolicy;

class CameraBinControl : public QCameraControl
{
    Q_OBJECT
    Q_PROPERTY(bool viewfinderColorSpaceConversion READ viewfinderColorSpaceConversion WRITE setViewfinderColorSpaceConversion)
    Q_PROPERTY(QSize viewfinderResolution READ viewfinderResolution WRITE setViewfinderResolution NOTIFY viewfinderResolutionChanged)
    Q_PROPERTY(qreal viewfinderFramerate READ viewfinderFramerate WRITE setViewfinderFramerate NOTIFY viewfinderFramerateChanged)
public:
    CameraBinControl( CameraBinSession *session );
    virtual ~CameraBinControl();

    bool isValid() const { return true; }

    QCamera::State state() const;
    void setState(QCamera::State state);

    QCamera::Status status() const { return m_status; }

    QCamera::CaptureMode captureMode() const;
    void setCaptureMode(QCamera::CaptureMode mode);

    bool isCaptureModeSupported(QCamera::CaptureMode mode) const;
    bool canChangeProperty(PropertyChangeType changeType, QCamera::Status status) const;
    bool viewfinderColorSpaceConversion() const;

    QSize viewfinderResolution() const;
    qreal viewfinderFramerate() const;

    bool eventFilter(QObject *watched, QEvent *event );
public slots:
    void reloadLater();
    void setViewfinderColorSpaceConversion(bool enabled);

    void setViewfinderResolution(const QSize& resolution);
    void setViewfinderFramerate(qreal framerate);

signals:

    void viewfinderResolutionChanged(const QSize&);
    void viewfinderFramerateChanged(qreal);

private slots:
    void updateStatus();
    void delayedReload();

    void handleResourcesGranted();
    void handleResourcesLost();

    void handleBusyChanged(bool);
    void handleCameraError(int error, const QString &errorString);

    void updateRecorderResources(QMediaRecorder::State);

private:
    void updateSupportedResolutions(const QString &device);

    CameraBinSession *m_session;
    QCamera::State m_state;
    QCamera::Status m_status;
    CamerabinResourcePolicy *m_resourcePolicy;

    bool m_reloadPending;
};

#endif // CAMERABINCONTROL_H
