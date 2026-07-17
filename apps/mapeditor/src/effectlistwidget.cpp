// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "effectlistwidget.h"

#include "scenestate.h"

#include <QListWidget>
#include <QVBoxLayout>

#include <KLocalizedString>

EffectListWidget::EffectListWidget(SceneState *state, const std::vector<int32_t> &effects, QWidget *parent)
    : QDialog(parent)
{
    setMinimumSize(QSize(640, 480));
    setWindowTitle(i18n("Map Effects"));

    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    const auto listWidget = new QListWidget();
    listWidget->setEditTriggers(QListWidget::EditTrigger::NoEditTriggers);

    connect(listWidget, &QListWidget::activated, this, [state](const QModelIndex &index) {
        const uint32_t objectId = index.data(Qt::UserRole).toUInt();
        Q_EMIT state->selectObject(objectId);
    });

    for (const auto &objectId : effects) {
        if (objectId > 0) {
            const auto item = new QListWidgetItem(QString::number(objectId));
            item->setData(Qt::UserRole, objectId);
            listWidget->addItem(item);
        }
    }

    layout->addWidget(listWidget);
}

#include "moc_effectlistwidget.cpp"
