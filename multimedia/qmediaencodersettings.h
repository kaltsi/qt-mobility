/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QMEDIAENCODERSETTINGS_H
#define QMEDIAENCODERSETTINGS_H

#include <QtCore/qsharedpointer.h>
#include <QtCore/qstring.h>
#include <QtCore/qsize.h>
#include <qmultimediaglobal.h>
#include <qtmedianamespace.h>


class QAudioEncoderSettingsPrivate;
class Q_MEDIA_EXPORT QAudioEncoderSettings
{
public:
    QAudioEncoderSettings();
    QAudioEncoderSettings(const QAudioEncoderSettings& other);

    ~QAudioEncoderSettings();

    QAudioEncoderSettings& operator=(const QAudioEncoderSettings &other);
    bool operator==(const QAudioEncoderSettings &other) const;
    bool operator!=(const QAudioEncoderSettings &other) const;

    bool isNull() const;

    QtMedia::EncodingMode encodingMode() const;
    void setEncodingMode(QtMedia::EncodingMode);

    QString codec() const;
    void setCodec(const QString& codec);

    int bitrate() const;
    void setBitrate(int bitrate);

    int channels() const;
    void setChannels(int channels);

    int sampleRate() const;
    void setSampleRate(int rate);

    QtMedia::EncodingQuality quality() const;
    void setQuality(QtMedia::EncodingQuality quality);

private:
    QSharedDataPointer<QAudioEncoderSettingsPrivate> d;
};

class QVideoEncoderSettingsPrivate;
class Q_MEDIA_EXPORT QVideoEncoderSettings
{
public:
    QVideoEncoderSettings();
    QVideoEncoderSettings(const QVideoEncoderSettings& other);

    ~QVideoEncoderSettings();

    QVideoEncoderSettings& operator=(const QVideoEncoderSettings &other);
    bool operator==(const QVideoEncoderSettings &other) const;
    bool operator!=(const QVideoEncoderSettings &other) const;

    bool isNull() const;

    QtMedia::EncodingMode encodingMode() const;
    void setEncodingMode(QtMedia::EncodingMode);

    QString codec() const;
    void setCodec(const QString &);

    QSize resolution() const;
    void setResolution(const QSize &);
    void setResolution(int width, int height);

    qreal frameRate() const;
    void setFrameRate(qreal rate);

    int bitrate() const;
    void setBitrate(int bitrate);

    QtMedia::EncodingQuality quality() const;
    void setQuality(QtMedia::EncodingQuality quality);

private:
    QSharedDataPointer<QVideoEncoderSettingsPrivate> d;
};


#endif
