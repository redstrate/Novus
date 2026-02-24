// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>
#include <physis.hpp>

class SceneState;
class QSortFilterProxyModel;

class GimmickListWidget : public QDialog
{
    Q_OBJECT

public:
    explicit GimmickListWidget(SceneState *state, physis_SqPackResource *data, QWidget *parent = nullptr);
};
