// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "effectlistwidget.h"

#include "scenestate.h"

#include <QListWidget>
#include <QVBoxLayout>

EffectListWidget::EffectListWidget(SceneState *state, std::vector<int32_t> effects, QWidget *parent)
    : QDialog(parent)
{
    setMinimumSize(QSize(640, 480));

    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto listWidget = new QListWidget();
    listWidget->setEditTriggers(QListWidget::EditTrigger::NoEditTriggers);

    connect(listWidget, &QListWidget::activated, this, [state](const QModelIndex &index) {
        const uint32_t objectId = index.data(Qt::UserRole).toUInt();
        Q_EMIT state->selectObject(objectId);
    });

    for (const auto &objectId : effects) {
        if (objectId > 0) {
            auto item = new QListWidgetItem(QString::number(objectId));
            item->setData(Qt::UserRole, objectId);
            listWidget->addItem(item);
        }
    }

    layout->addWidget(listWidget);
}

#include "moc_effectlistwidget.cpp"
