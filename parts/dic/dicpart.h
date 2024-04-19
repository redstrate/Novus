// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTableWidget>
#include <QWidget>
#include <physis.hpp>

class DicPart : public QWidget
{
    Q_OBJECT

public:
    explicit DicPart(QWidget *parent = nullptr);

    void load(physis_Buffer file);

private:
    GameData *data = nullptr;

    QTableWidget *m_tableWidget = nullptr;
};