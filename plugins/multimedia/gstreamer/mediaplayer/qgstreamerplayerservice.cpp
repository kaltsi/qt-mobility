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

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>
#include <QtGui/qwidget.h>

#include "qgstreamerplayerservice.h"
#include "qgstreamerplayercontrol.h"
#include "qgstreamerplayersession.h"
#include "qgstreamermetadataprovider.h"

#include "qgstreamervideooverlay.h"
#include "qgstreamervideowindow.h"
#include "qgstreamervideorenderer.h"

#if defined(Q_WS_MAEMO_6) && defined(__arm__)
#include "qgstreamergltexturerenderer.h"
#endif

#include "qgstreamervideowidget.h"
#include "qgstreamerstreamscontrol.h"

#include <qmediaplaylistnavigator.h>
#include <qmediaplaylist.h>

QGstreamerPlayerService::QGstreamerPlayerService(QObject *parent):
     QMediaService(parent),
     m_videoOutput(0),
     m_videoRenderer(0),
     m_videoWindow(0),
     m_videoWidget(0)
{
    m_session = new QGstreamerPlayerSession(this);
    m_control = new QGstreamerPlayerControl(m_session, this);
    m_metaData = new QGstreamerMetaDataProvider(m_session, this);
    m_streamsControl = new QGstreamerStreamsControl(m_session,this);

#if defined(Q_WS_MAEMO_6) && defined(__arm__)
    m_videoRenderer = new QGstreamerGLTextureRenderer(this);
#else
    m_videoRenderer = new QGstreamerVideoRenderer(this);
#endif

#if defined(Q_WS_X11) && !defined(QT_NO_XVIDEO)

#if defined(Q_WS_MAEMO_6)
    m_videoWindow = new QGstreamerVideoWindow(this, "omapxvsink");
#elif defined(Q_WS_MEEGO)
    m_videoWindow = new QGstreamerVideoWindow(this);
#else
    m_videoWindow = new QGstreamerVideoOverlay(this);
#endif

    m_videoWidget = new QGstreamerVideoWidgetControl(this);
#endif
}

QGstreamerPlayerService::~QGstreamerPlayerService()
{
    // make sure the renderer does not draw a final
    // still image of the video, prevents a crash
    m_session->setVideoRenderer( 0 );

    delete m_videoWidget;
    m_videoWidget = 0;
    delete m_videoWindow;
    m_videoWindow = 0;
    delete m_videoRenderer;
    m_videoRenderer = 0;
    delete m_streamsControl;
    m_streamsControl = 0;
    delete m_metaData;
    m_metaData = 0;
    delete m_control;
    m_control = 0;
    delete m_session;
    m_session = 0;
}

QMediaControl *QGstreamerPlayerService::requestControl(const char *name)
{
    if (qstrcmp(name,QMediaPlayerControl_iid) == 0)
        return m_control;

    if (qstrcmp(name,QMetaDataReaderControl_iid) == 0)
        return m_metaData;

    if (qstrcmp(name,QMediaStreamsControl_iid) == 0)
        return m_streamsControl;

    if (!m_videoOutput) {
        if (qstrcmp(name, QVideoWidgetControl_iid) == 0)
            m_videoOutput = m_videoWidget;
        else if (qstrcmp(name, QVideoRendererControl_iid) == 0)
            m_videoOutput = m_videoRenderer;
        else  if (qstrcmp(name, QVideoWindowControl_iid) == 0)
            m_videoOutput = m_videoWindow;

        if (m_videoOutput) {
            m_control->setVideoOutput(m_videoOutput);
            return m_videoOutput;
        }
    }

    return 0;
}

void QGstreamerPlayerService::releaseControl(QMediaControl *control)
{
    if (control == m_videoOutput) {
        m_videoOutput = 0;
        m_control->setVideoOutput(0);
    }
}

