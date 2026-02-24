// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>

class EffectListWidget : public QDialog
{
    Q_OBJECT

public:
    explicit EffectListWidget(std::vector<int32_t> effects, QWidget *parent = nullptr);
};
