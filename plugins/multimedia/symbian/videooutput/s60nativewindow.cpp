/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "s60mmtrace.h"
#include "s60nativewindow.h"
#include <coemain.h>
#include <w32std.h>
#include <graphics/surfaceconfiguration.h>

static const int WindowWidth = 320;
static const int WindowHeight = 240;

S60NativeWindow::S60NativeWindow(QObject *parent)
    : QObject(parent)
    , m_window(0)
    , m_surface(TSurfaceId::CreateNullId())
{
    TRACE("S60NativeWindow::S60NativeWindow" << qtThisPtr());
    RWsSession &session = CCoeEnv::Static()->WsSession();
    RWindowGroup &windowGroup = CCoeEnv::Static()->RootWin();
    m_window = q_check_ptr(new RWindow(session));
    const int err = m_window->Construct(windowGroup, reinterpret_cast<quint32>(this));
    if (err) {
        delete m_window;
        m_window = 0;
    } else {
        m_window->SetExtent(TPoint(0, 0), TSize(WindowWidth, WindowHeight));
    }
}

S60NativeWindow::~S60NativeWindow()
{
    TRACE("S60NativeWindow::~S60NativeWindow" << qtThisPtr());
    if (m_window)
        m_window->Close();
    delete m_window;
}

RWindow *S60NativeWindow::windowHandle() const
{
    return m_window;
}

void S60NativeWindow::setNativeSurface(const TSurfaceId &surface)
{
    getSurface();
    if (surface != m_surface) {
        TRACE("S60NativeWindow::setNativeSurface" << (void*)surface.iInternal[3]);
        if (RWindow *const window = windowHandle())
            window->SetBackgroundSurface(surface);
    }
}

TSurfaceId S60NativeWindow::nativeSurface()
{
    getSurface();
    return m_surface;
}

void S60NativeWindow::getSurface()
{
    TSurfaceId surface = TSurfaceId::CreateNullId();
    int err = 0;
    RWindow *const window = windowHandle();
    if (window) {
        TSurfaceConfiguration config;
        err = window->GetBackgroundSurface(config);
        if (!err)
            config.GetSurfaceId(surface);
    }
    if (!err) {
        m_surface = surface;
        if (!m_surface.IsNull())
            window->RemoveBackgroundSurface(ETrue);
    }
}

