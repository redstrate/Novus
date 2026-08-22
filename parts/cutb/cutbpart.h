// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QWidget>
#include <physis.hpp>

class CutbPart : public QWidget
{
    Q_OBJECT

public:
    explicit CutbPart(QWidget *parent = nullptr);

    void load(Platform platform, physis_Buffer file);

private:
    physis_CMP m_cmp{};

    QHBoxLayout *m_layout = nullptr;
};
