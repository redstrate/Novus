// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exdpart.h"

#include <QDebug>
#include <QTableWidget>
#include <QVBoxLayout>
#include <physis.hpp>

EXDPart::EXDPart(GameData* data) : data(data) {
    pageTabWidget = new QTabWidget();

    auto layout = new QVBoxLayout();
    layout->addWidget(pageTabWidget);
    setLayout(layout);
}

void EXDPart::loadSheet(const QString& name) {
    qDebug() << "Loading" << name;

    pageTabWidget->clear();

    auto exh = physis_gamedata_read_excel_sheet_header(data, name.toStdString().c_str());

    for(int i = 0; i < exh->page_count; i++) {
        QTableWidget* tableWidget = new QTableWidget();

        tableWidget->setColumnCount(exh->column_count);

        auto exd = physis_gamedata_read_excel_sheet(data, name.toStdString().c_str(), exh, exh->languages[0], i);

        tableWidget->setRowCount(exd.row_count);

        for (int j = 0; j < exd.row_count; j++) {
            for(int z = 0; z < exd.column_count; z++) {
                auto data = exd.row_data[j].column_data[z];

                QString columnString;
                QString columnType;
                switch (data.tag) {
                    case physis_ColumnData::Tag::String:
                        columnString = QString(data.string._0);
                        columnType = "String";
                        break;
                    case physis_ColumnData::Tag::Bool:
                        columnString = data.bool_._0 ? "True" : "False";
                        columnType = "Bool";
                        break;
                    case physis_ColumnData::Tag::Int8:
                        columnString = QString::number(data.int8._0);
                        columnType = "Int8";
                        break;
                    case physis_ColumnData::Tag::UInt8:
                        columnString = QString::number(data.u_int8._0);
                        columnType = "UInt8";
                        break;
                    case physis_ColumnData::Tag::Int16:
                        columnString = QString::number(data.int16._0);
                        columnType = "Int16";
                        break;
                    case physis_ColumnData::Tag::UInt16:
                        columnString = QString::number(data.u_int16._0);
                        columnType = "UInt16";
                        break;
                    case physis_ColumnData::Tag::Int32:
                        columnString = QString::number(data.int32._0);
                        columnType = "Int32";
                        break;
                    case physis_ColumnData::Tag::UInt32:
                        columnString = QString::number(data.u_int32._0);
                        columnType = "UInt32";
                        break;
                    case physis_ColumnData::Tag::Float32:
                        columnString = QString::number(data.float32._0);
                        columnType = "Float32";
                        break;
                    case physis_ColumnData::Tag::Int64:
                        columnString = QString::number(data.int64._0);
                        columnType = "Int64";
                        break;
                    case physis_ColumnData::Tag::UInt64:
                        columnString = QString::number(data.u_int64._0);
                        columnType = "UInt64";
                        break;
                }

                auto newItem = new QTableWidgetItem(columnString);

                tableWidget->setItem(i, j, newItem);

                QTableWidgetItem* headerItem = new QTableWidgetItem();
                headerItem->setText(columnType);
                tableWidget->setHorizontalHeaderItem(j, headerItem);
            }
        }

        pageTabWidget->addTab(tableWidget, QString("Page %1").arg(i));
    }
}