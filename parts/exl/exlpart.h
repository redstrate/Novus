// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTableWidget>
#include <QWidget>
#include <physis.hpp>

class EXLPart : public QWidget
{
    Q_OBJECT

public:
    explicit EXLPart(QWidget *parent = nullptr);

    void load(Platform platform, physis_Buffer file) const;

private:
    QTableWidget *m_tableWidget = nullptr;
};
