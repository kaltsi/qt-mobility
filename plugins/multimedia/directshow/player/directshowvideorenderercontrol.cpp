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

#include "directshowvideorenderercontrol.h"

#include "videosurfacefilter.h"

DirectShowVideoRendererControl::DirectShowVideoRendererControl(DirectShowEventLoop *loop, QObject *parent)
    : QVideoRendererControl(parent)
    , m_loop(loop)
    , m_surface(0)
    , m_filter(0)
{
}

DirectShowVideoRendererControl::~DirectShowVideoRendererControl()
{
    delete m_filter;
}

QAbstractVideoSurface *DirectShowVideoRendererControl::surface() const
{
    return m_surface;
}

void DirectShowVideoRendererControl::setSurface(QAbstractVideoSurface *surface)
{
    if (surface != m_surface) {
        m_surface = surface;

        VideoSurfaceFilter *existingFilter = m_filter;

        if (surface) {
            m_filter = new VideoSurfaceFilter(surface, m_loop);
        } else {
            m_filter = 0;
        }

        emit filterChanged();

        delete existingFilter;
    }
}

IBaseFilter *DirectShowVideoRendererControl::filter()
{
    return m_filter;
}
