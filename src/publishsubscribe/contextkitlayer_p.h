/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CONTEXTKITLAYER_P_H
#define CONTEXTKITLAYER_P_H

#include "qvaluespace.h"
#include "qvaluespace_p.h"

#include <QSet>
#include <QStringList>

#include <contextproperty.h>
#include <context/property.h>
#include <context/service.h>
#include <contextregistryinfo.h>

QTM_BEGIN_NAMESPACE

using namespace QValueSpace;

class ContextKitPath
{
public:
    ContextKitPath();
    ContextKitPath(QString path);
    ContextKitPath(const ContextKitPath &other);

    QString toQtPath() const;
    QString toCKPath() const;

    inline bool wasDotPath() const { return dotPath; }
    inline bool wasSlashPath() const { return slashPath; }

    bool includes(ContextKitPath &other) const;
    bool isRegistered() const;

    inline int size() const { return parts.size(); }
    inline QString at(int i) const { return parts.at(i); }

    ContextKitPath &operator=(const ContextKitPath &other);
    bool operator==(const ContextKitPath &other) const;

    ContextKitPath operator+(const QString &str) const;
    ContextKitPath operator-(const ContextKitPath &other) const;

private:
    QStringList parts;
    bool dotPath;
    bool slashPath;
};

class ContextKitHandle : public QObject
{
    Q_OBJECT

public:
    ContextKitHandle(ContextKitHandle *parent, const QString &root);
    ~ContextKitHandle();

    bool value(const QString &path, QVariant *data);
    bool setValue(const QString &path, const QVariant &data);
    bool unsetValue(const QString &path);
    void subscribe();
    void unsubscribe();
    QSet<QString> children();

signals:
    void valueChanged();

private:
    ContextKitPath path;
    QHash<QString, ContextProperty*> readProps;
    QHash<QString, ContextProvider::Property*> writeProps;
    ContextProvider::Service *service;

    void insertRead(const ContextKitPath &path);

private slots:
    void updateSubtrees();
};


class ContextKitLayer : public QAbstractValueSpaceLayer
{
    Q_OBJECT

public:
    ContextKitLayer();
    virtual ~ContextKitLayer();

    /* ValueSpaceLayer interface - Common functions */
    QString name();
    bool startup(Type);
    QUuid id();
    unsigned int order();
    LayerOptions layerOptions() const;

    Handle item(Handle parent, const QString &);
    void removeHandle(Handle);
    void setProperty(Handle handle, Properties);

    bool value(Handle, QVariant *);
    bool value(Handle, const QString &, QVariant *);
    QSet<QString> children(Handle);

    /* ValueSpaceLayer interface - QValueSpaceSubscriber functions */
    bool supportsInterestNotification() const { return true; }
    bool notifyInterest(Handle handle, bool interested);

    /* ValueSpaceLayer interface - QValueSpacePublisher functions */
    bool setValue(QValueSpacePublisher *vsp, Handle handle,
                  const QString &path, const QVariant &value);
    bool removeValue(QValueSpacePublisher *vsp, Handle handle,
                     const QString &path);
    bool removeSubTree(QValueSpacePublisher *vsp, Handle handle);

    void addWatch(QValueSpacePublisher *, Handle) { return; }
    void removeWatches(QValueSpacePublisher *, Handle) { return; }
    void sync();

    static ContextKitLayer *instance();

private slots:
    void contextHandleChanged();

private:
    static ContextKitHandle *handleToCKHandle(Handle handle);
};

QTM_END_NAMESPACE

#endif // CONTEXTKITLAYER_P_H