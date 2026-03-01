// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

#include "novuscommon_export.h"

class NOVUSCOMMON_EXPORT EditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EditWidget(QWidget *parent = nullptr);
    ~EditWidget() override = default;

Q_SIGNALS:
    void onValueChanged();
    void editingFinished();
};
