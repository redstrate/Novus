// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

#include "physis.hpp"

class TextEditor;
class QHexView;

class HexPart : public QWidget
{
    Q_OBJECT

public:
    explicit HexPart(QWidget *parent = nullptr);

    void loadFile(physis_Buffer buffer);

private:
    QHexView *m_hexView = nullptr;
    TextEditor *m_textEdit = nullptr;
};
