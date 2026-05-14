// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>

class EnemyInfoWindow : public QDialog
{
public:
    explicit EnemyInfoWindow(uint32_t id, const QString &mdlPath, const QString &mtrlPath, QWidget *parent);
};
