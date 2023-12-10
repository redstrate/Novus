// SPDX-FileCopyrightText: 2014 Dax89
// SPDX-License-Identifier: MIT

#include "qhexbuffer.h"
#include <QBuffer>

QHexBuffer::QHexBuffer(QObject *parent)
    : QObject(parent)
{
}

uchar QHexBuffer::at(qint64 idx)
{
    return this->read(idx, 1)[0];
}
bool QHexBuffer::isEmpty() const
{
    return this->length() <= 0;
}

void QHexBuffer::replace(qint64 offset, const QByteArray &data)
{
    this->remove(offset, data.length());
    this->insert(offset, data);
}

void QHexBuffer::readRaw(char *data, int size)
{
    QBuffer *buffer = new QBuffer(this);
    buffer->setData(data, size);

    if (!buffer->isOpen())
        buffer->open(QBuffer::ReadWrite);

    this->readDevice(buffer);
}

void QHexBuffer::readArray(const QByteArray &ba)
{
    QBuffer *buffer = new QBuffer(this);

    buffer->setData(ba);
    if (!buffer->isOpen())
        buffer->open(QBuffer::ReadWrite);

    this->readDevice(buffer);
}

#include "moc_qhexbuffer.cpp"
