// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QListView>
#include <QWidget>
#include <physis.hpp>

class SheetListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SheetListWidget(SqPackResource *data, QWidget *parent = nullptr);

Q_SIGNALS:
    void sheetSelected(const QString &name);

private:
    QListView *listWidget = nullptr;

    SqPackResource *data = nullptr;
};
