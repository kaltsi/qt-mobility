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

#include "qtlv_p.h"

#include "qnearfieldtagtype1.h"

#include <QtCore/QDebug>

QTM_BEGIN_NAMESPACE

/*!
    \class QTlvReader
    \brief The QTlvReader class provides a TLV parser.

    \ingroup connectivity-nfc
    \inmodule QtConnectivity

    \internal

    QTlvReader can parse TLV data from either a QByteArray or directly from a QNearFieldTarget.

    TLV stands for Tag, Length, Value and is a common structure for storing data in near field
    targets.

    The following code shows how to use QTlvReader to extract NDEF messages from a near field
    target.

    \code
    QTlvReader reader(target);
    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.tag() == 0x03)
            QNdefMessage ndefMessage = QNdefMessage::fromByteArray(reader.data());
    }
    \endcode
*/

QPair<int, int> qParseReservedMemoryControlTlv(const QByteArray &tlvData)
{
    quint8 position = tlvData.at(0);
    int pageAddr = position >> 4;
    int byteOffset = position & 0x0f;

    int size = quint8(tlvData.at(1));
    if (size == 0)
        size = 256;

    quint8 pageControl = tlvData.at(2);
    int bytesPerPage = pageControl & 0x0f;

    if (!bytesPerPage)
        return qMakePair(0, 0);

    int byteAddress = pageAddr * (1 << bytesPerPage) + byteOffset;
    return qMakePair(byteAddress, size);
}

QPair<int, int> qParseLockControlTlv(const QByteArray &tlvData)
{
    quint8 position = tlvData.at(0);
    int pageAddr = position >> 4;
    int byteOffset = position & 0x0f;

    int size = quint8(tlvData.at(1));
    if (size == 0)
        size = 256;
    size = size / 8;

    quint8 pageControl = tlvData.at(2);
    int bytesPerPage = pageControl & 0x0f;

    if (!bytesPerPage)
        return qMakePair(0, 0);

    int byteAddress = pageAddr * (1 << bytesPerPage) + byteOffset;
    return qMakePair(byteAddress, size);
}

/*!
    Constructs a new TLV reader for \a target.
*/
QTlvReader::QTlvReader(QNearFieldTarget *target)
:   m_target(target), m_index(-1)
{
    if (qobject_cast<QNearFieldTagType1 *>(m_target)) {
        addReservedMemory(0, 12);   // skip uid, cc
        addReservedMemory(104, 16); // skip reserved block D, lock block E

        addReservedMemory(120, 8);  // skip reserved block F
    }
}

/*!
    Constructs a new TLV reader for \a data.
*/
QTlvReader::QTlvReader(const QByteArray &data)
:   m_target(0), m_rawData(data), m_index(-1)
{
}

/*!
    Add reserved memory area from \a offset of \a length bytes.  The parser will skip over reserved
    memory areas.
*/
void QTlvReader::addReservedMemory(int offset, int length)
{
    m_reservedMemory.insert(offset, length);
}

/*!
    Returns the number of bytes of reserved memory found so far.  The actual number of reserved
    bytes will not be known until atEnd() returns true.
*/
int QTlvReader::reservedMemorySize() const
{
    int total = 0;

    QMap<int, int>::ConstIterator i;
    for (i = m_reservedMemory.constBegin(); i != m_reservedMemory.constEnd(); ++i)
        total += i.value();

    return total;
}

/*!
    Returns true if the TLV reader is at the end of the list of TLVs; otherwise returns false.
*/
bool QTlvReader::atEnd() const
{
    if (m_index == -1)
        return false;

    return (m_index == m_tlvData.length()) || (tag() == 0xfe);
}

/*!
    Moves to the next TLV. Returns true on success; otherwise returns false.
*/
bool QTlvReader::readNext()
{
    if (atEnd())
        return false;

    // Move to next TLV
    if (m_index == -1) {
        m_index = 0;
    } else if (tag() == 0x00 || tag() == 0xfe) {
        ++m_index;
    } else {
        int tlvLength = length();
        m_index += (tlvLength < 0xff) ? tlvLength + 2 : tlvLength + 4;
    }

    // Ensure that tag byte is available
    if (!readMoreData(m_index))
        return false;

    // Ensure that length byte(s) are available
    if (length() == -1)
        return false;

    // Ensure that data bytes are available
    int tlvLength = length();

    int dataOffset = (tlvLength < 0xff) ? m_index + 2 : m_index + 4;

    if (!readMoreData(dataOffset + tlvLength - 1))
        return false;

    switch (tag()) {
    case 0x01: { // Lock Control TLV
        QPair<int, int> locked = qParseLockControlTlv(data());
        addReservedMemory(locked.first, locked.second);
        break;
    }
    case 0x02: { // Reserved Memory Control TLV
        QPair<int, int> reserved = qParseReservedMemoryControlTlv(data());
        addReservedMemory(reserved.first, reserved.second);
        break;
    }
    }

    return true;
}

/*!
    Returns the tag of the current TLV.
*/
quint8 QTlvReader::tag() const
{
    return m_tlvData.at(m_index);
}

/*!
    Returns the length of the data of the current TLV. Returns -1 if a parse error occurs.
*/
int QTlvReader::length()
{
    if (tag() == 0x00 || tag() == 0xfe)
        return 0;

    if (!readMoreData(m_index + 1))
        return -1;

    quint8 shortLength = m_tlvData.at(m_index + 1);
    if (shortLength != 0xff)
        return shortLength;

    if (!readMoreData(m_index + 3))
        return -1;

    quint16 longLength = (quint8(m_tlvData.at(m_index + 2)) << 8) |
                          quint8(m_tlvData.at(m_index + 3));

    if (longLength < 0xff || longLength == 0xffff) {
        qWarning("Invalid 3 byte length");
        return 0;
    }

    return longLength;
}

/*!
    Returns the data of the current TLV.
*/
QByteArray QTlvReader::data()
{
    int tlvLength = length();

    int dataOffset = (tlvLength < 0xff) ? m_index + 2 : m_index + 4;

    if (!readMoreData(dataOffset + tlvLength - 1))
        return QByteArray();

    return m_tlvData.mid(dataOffset, tlvLength);
}

/*!
    Reads more data until \a sparseOffset is available in m_data.
*/
bool QTlvReader::readMoreData(int sparseOffset)
{
    while (sparseOffset >= m_tlvData.length()) {
        int absOffset = absoluteOffset(m_tlvData.length());

        QByteArray data;

        if (!m_rawData.isEmpty()) {
            data = m_rawData.mid(absOffset, dataLength(absOffset));
        } else if (QNearFieldTagType1 *tag = qobject_cast<QNearFieldTagType1 *>(m_target)) {
            quint8 segment = absOffset / 128;

            data = (absOffset < 120) ? tag->readAll().mid(2) : tag->readSegment(segment);

            int length = dataLength(absOffset);

            data = data.mid(absOffset - (segment * 128), length);
        }

        if (data.isEmpty() && sparseOffset >= m_tlvData.length())
            return false;

        m_tlvData.append(data);
    }

    return true;
}

/*!
    Returns the absoluate offset for \a sparseOffset, taking all reserved memory into account.
*/
int QTlvReader::absoluteOffset(int sparseOffset) const
{
    int absoluteOffset = sparseOffset;
    foreach (int offset, m_reservedMemory.keys()) {
        if (offset <= absoluteOffset)
            absoluteOffset += m_reservedMemory.value(offset);
    }

    return absoluteOffset;
}

/*!
    Returns the length of the contiguous non-reserved data block starting from absolute offset
    \a startOffset.  -1 is return as the length of the last contiguous data block.
*/
int QTlvReader::dataLength(int startOffset) const
{
    foreach (int offset, m_reservedMemory.keys()) {
        if (offset <= startOffset)
            continue;

        return offset - startOffset;
    }

    return -1;
}


QTlvWriter::QTlvWriter(QNearFieldTarget *target)
:   m_target(target), m_rawData(0), m_index(0)
{
    if (qobject_cast<QNearFieldTagType1 *>(m_target)) {
        addReservedMemory(0, 12);   // skip uid, cc
        addReservedMemory(104, 16); // skip reserved block D, lock block E

        addReservedMemory(120, 8);  // skip reserved block F
    }
}

QTlvWriter::QTlvWriter(QByteArray *data)
:   m_target(0), m_rawData(data), m_index(0)
{
}

QTlvWriter::~QTlvWriter()
{
    flush(true);
}

void QTlvWriter::addReservedMemory(int offset, int length)
{
    m_reservedMemory.insert(offset, length);
}

void QTlvWriter::writeTlv(quint8 tagType, const QByteArray &data)
{
    m_buffer.append(tagType);

    if (tagType != 0x00 && tagType != 0xfe) {
        int length = data.length();
        if (length < 0xff) {
            m_buffer.append(quint8(length));
        } else {
            m_buffer.append(0xff);
            m_buffer.append(quint16(length) >> 8);
            m_buffer.append(quint16(length) & 0x00ff);
        }

        m_buffer.append(data);
    }

    flush();

    switch (tagType) {
    case 0x01: {    // Lock Control TLV
        QPair<int, int> locked = qParseLockControlTlv(data);
        addReservedMemory(locked.first, locked.second);
        break;
    }
    case 0x02: {    // Reserved Memory Control TLV
        QPair<int, int> reserved = qParseReservedMemoryControlTlv(data);
        addReservedMemory(reserved.first, reserved.second);
        break;
    }
    }
}

void QTlvWriter::flush(bool all)
{
    while (!m_buffer.isEmpty()) {
        int spaceRemaining = moveToNextAvailable();
        if (spaceRemaining < 1)
            break;

        int length = qMin(spaceRemaining, m_buffer.length());

        if (m_rawData) {
            m_rawData->replace(m_index, length, m_buffer);
            m_index += length;
            m_buffer = m_buffer.mid(length);
        } else if (QNearFieldTagType1 *tag = qobject_cast<QNearFieldTagType1 *>(m_target)) {
            int bufferIndex = 0;

            // static memory - can only use writeByte()
            while (m_index < 120 && bufferIndex < length)
                tag->writeByte(m_index++, m_buffer.at(bufferIndex++));

            // dynamic memory - writeBlock() full
            while (m_index >= 120 && (m_index % 8 == 0) && bufferIndex + 8 < length) {
                tag->writeBlock(m_index / 8, m_buffer.mid(bufferIndex, 8));
                m_index += 8;
                bufferIndex += 8;
            }

            // partial block

            int currentBlock = m_index / 8;
            int nextBlock = currentBlock + 1;
            int currentBlockStart = currentBlock * 8;
            int nextBlockStart = nextBlock * 8;

            int fillLength = qMin(nextBlockStart - m_index, spaceRemaining - bufferIndex);

            if (fillLength && (all || m_buffer.length() - bufferIndex >= fillLength)) {
                // sufficient data available
                QByteArray block = tag->readBlock(currentBlock);

                int fill = qMin(fillLength, m_buffer.length() - bufferIndex);

                for (int i = m_index - currentBlockStart; i < fill; ++i)
                    block[i] = m_buffer.at(bufferIndex++);

                tag->writeBlock(currentBlock, block);
            }

            m_buffer = m_buffer.mid(bufferIndex);

            if (m_buffer.length() < fillLength)
                break;
        }
    }
}

int QTlvWriter::moveToNextAvailable()
{
    int length = -1;

    // move index to next available byte
    QMap<int, int>::ConstIterator i;
    for (i = m_reservedMemory.constBegin(); i != m_reservedMemory.constEnd(); ++i) {
        if (m_index < i.key()) {
            length = i.key() - m_index;
            break;
        } else if (m_index == i.key()) {
            m_index += i.value();
        } else if (m_index > i.key() && m_index < (i.key() + i.value())) {
            m_index = i.key() + i.value();
        }
    }

    if (length == -1) {
        if (m_rawData)
            return m_rawData->length() - m_index;
        else if (QNearFieldTagType1 *tag = qobject_cast<QNearFieldTagType1 *>(m_target))
            return tag->memorySize() - m_index;
    }

    Q_ASSERT(length != -1);

    return length;
}

QTM_END_NAMESPACE