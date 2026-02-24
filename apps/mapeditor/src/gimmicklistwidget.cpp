// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gimmicklistwidget.h"

#include "physis.hpp"

#include <KLocalizedString>
#include <QTableWidget>
#include <QVBoxLayout>

#include "scenestate.h"

GimmickListWidget::GimmickListWidget(SceneState *state, physis_SqPackResource *data, QWidget *parent)
    : QDialog(parent)
{
    setMinimumSize(QSize(640, 480));
    setWindowTitle(i18n("Map Gimmicks"));

    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto eobjExh = physis_exh_parse(data->platform, physis_sqpack_read(data, "exd/EObj.exh"));
    auto eobjSheet = physis_sqpack_read_excel_sheet(data, "EObj", &eobjExh, Language::None);

    auto tableWidget = new QTableWidget();
    tableWidget->setEditTriggers(QTableWidget::EditTrigger::NoEditTriggers);
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels({i18n("Gimmick ID"), i18n("Game Object ID")});

    connect(tableWidget, &QTableWidget::activated, this, [state](const QModelIndex &index) {
        const uint32_t objectId = index.data(Qt::UserRole).toUInt();
        Q_EMIT state->selectObject(objectId);
    });

    for (const auto &[_, lgb] : state->rootScene.lgbFiles) {
        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            for (uint32_t j = 0; j < lgb.chunks[i].num_layers; j++) {
                for (uint32_t l = 0; l < lgb.chunks[i].layers[j].num_objects; l++) {
                    const auto object = lgb.chunks[i].layers[j].objects[l];
                    if (object.data.tag == physis_LayerEntry::Tag::EventObject) {
                        const auto baseId = object.data.event_object._0.parent_data.base_id;

                        const auto row = physis_excel_get_row(&eobjSheet, baseId);
                        const auto eobjData = row.columns[9].u_int32._0;

                        const auto eventType = eobjData >> 16;
                        const auto eventId = eobjData & 0xFFFF;
                        // GimmickAccessor is 15
                        if (eventType == 15) {
                            const int row = tableWidget->rowCount();
                            tableWidget->insertRow(row);

                            auto gimmickItem = new QTableWidgetItem(QString::number(eventId));
                            gimmickItem->setData(Qt::UserRole, object.instance_id);

                            auto objectItem = new QTableWidgetItem(QString::number(object.instance_id));
                            objectItem->setData(Qt::UserRole, object.instance_id);

                            tableWidget->setItem(row, 0, gimmickItem);
                            tableWidget->setItem(row, 1, objectItem);
                        }
                    }
                }
            }
        }
    }

    layout->addWidget(tableWidget);
}

#include "moc_gimmicklistwidget.cpp"
