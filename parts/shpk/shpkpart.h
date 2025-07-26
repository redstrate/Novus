// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTabWidget>
#include <QWidget>
#include <physis.hpp>

class SHPKPart : public QWidget
{
    Q_OBJECT

public:
    explicit SHPKPart(QWidget *parent = nullptr);

    void load(physis_Buffer buffer);

private:
    QTabWidget *pageTabWidget = nullptr;
};
