// SPDX-FileCopyrightText: 2014 Dax89
// SPDX-License-Identifier: MIT

#pragma once

#include "qbuffer.h"
#include "qhexbuffer.h"

class QMemoryRefBuffer : public QHexBuffer
{
    Q_OBJECT

public:
    explicit QMemoryRefBuffer(QObject *parent = nullptr);
    qint64 length() const override;
    void insert(qint64 offset, const QByteArray &data) override;
    void remove(qint64 offset, int length) override;
    QByteArray read(qint64 offset, int length) override;
    bool readDevice(QIODevice *device) override;
    void writeDevice(QIODevice *device) override;

    qint64 indexOf(const QByteArray &ba, qint64 from) override;
    qint64 lastIndexOf(const QByteArray &ba, qint64 from) override;

private:
    QBuffer *m_buffer;
};
