// SPDX-FileCopyrightText: 2014 Dax89
// SPDX-License-Identifier: MIT

#pragma once

#include <QIODevice>
#include <QObject>

class QHexBuffer : public QObject
{
    Q_OBJECT

public:
    explicit QHexBuffer(QObject *parent = nullptr);
    bool isEmpty() const;

public:
    virtual uchar at(qint64 idx);
    virtual void replace(qint64 offset, const QByteArray &data);
    virtual void readRaw(char *data, int size);
    virtual void readArray(const QByteArray &ba);

public:
    virtual qint64 length() const = 0;
    virtual void insert(qint64 offset, const QByteArray &data) = 0;
    virtual void remove(qint64 offset, int length) = 0;
    virtual QByteArray read(qint64 offset, int length) = 0;
    virtual bool readDevice(QIODevice *iodevice) = 0;
    virtual void writeDevice(QIODevice *iodevice) = 0;

    virtual qint64 indexOf(const QByteArray &ba, qint64 from) = 0;
    virtual qint64 lastIndexOf(const QByteArray &ba, qint64 from) = 0;
};
