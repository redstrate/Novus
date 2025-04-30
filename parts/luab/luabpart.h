// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTableWidget>
#include <QWidget>
#include <physis.hpp>

class QTextEdit;

class LuabPart : public QWidget
{
    Q_OBJECT

public:
    explicit LuabPart(QWidget *parent = nullptr);

    void load(physis_Buffer file);

private:
    GameData *data = nullptr;
    QTextEdit *m_codeEdit = nullptr;
};