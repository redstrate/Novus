// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "materialview.h"

#include <QVBoxLayout>

MaterialView::MaterialView(GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);
}

#include "moc_materialview.cpp"