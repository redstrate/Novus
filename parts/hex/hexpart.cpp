// SPDX-FileCopyrightText: 2014 Dax89
// SPDX-License-Identifier: MIT

#include "hexpart.h"

#include "document/buffer/qmemoryrefbuffer.h"

HexPart::HexPart(QWidget *parent)
    : QHexView(parent)
{
}

void HexPart::loadFile(const physis_Buffer buffer)
{
    setDocument(QHexDocument::fromMemory<QMemoryRefBuffer>(reinterpret_cast<char *>(buffer.data), buffer.size, this));
    setReadOnly(true);
}

#include "moc_hexpart.cpp"
