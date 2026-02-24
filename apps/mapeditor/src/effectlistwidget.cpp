// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "effectlistwidget.h"

#include <QListWidget>
#include <QVBoxLayout>

EffectListWidget::EffectListWidget(std::vector<int32_t> effects, QWidget *parent)
    : QDialog(parent)
{
    setMinimumSize(QSize(640, 480));

    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto listWidget = new QListWidget();
    listWidget->setEditTriggers(QListWidget::EditTrigger::NoEditTriggers);

    for (const auto &objectId : effects) {
        if (objectId > 0) {
            listWidget->addItem(QString::number(objectId));
        }
    }

    layout->addWidget(listWidget);
}

#include "moc_effectlistwidget.cpp"
