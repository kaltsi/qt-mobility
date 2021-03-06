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

#include <e32base.h>
#include <e32property.h>
#include <nwhandlingengine.h>
#include <AknUtils.h>
#include <cnwsession.h>
#include <networkhandlingproxy.h>

class CtestNetworkOperatorNameListener : public CBase, public MNWMessageObserver
 {
  // from base class MNWMessageObserver

    /**
     * From MNWMessageObserver.
     * Offers message interface to the client.
     *
     * @param aMessage
     * This methods execute time must be short,since code
     * starting to run from RunL.
     */
    virtual void HandleNetworkMessage( const TNWMessages aMessage );

    /**
     * From MNWMessageObserver.
     * Offers error message interface to the client.
     *
     * @param aOperation operation which failed
     * @param aErrorCode returned Symbian OS error code
     */
    virtual void HandleNetworkError( const TNWOperation aOperation,
        TInt aErrorCode );
 };

 void CtestNetworkOperatorNameListener::HandleNetworkMessage( const TNWMessages /*aMessage*/ )
  {
  }

void CtestNetworkOperatorNameListener::HandleNetworkError( const TNWOperation /*aOperation*/, TInt /*aErrorCode*/ )
  {
  }

int main(int, char**)
{
    CtestNetworkOperatorNameListener test;
}

