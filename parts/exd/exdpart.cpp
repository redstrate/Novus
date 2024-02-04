// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exdpart.h"

#include <QFile>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <physis.hpp>

EXDPart::EXDPart(GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // TODO: This information should really be somewhere else
    /*auto headerBox = new QGroupBox(QStringLiteral("Header"));
    layout->addWidget(headerBox);
    headerFormLayout = new QFormLayout();
    headerBox->setLayout(headerFormLayout);*/

    pageTabWidget = new QTabWidget();
    pageTabWidget->setTabPosition(QTabWidget::TabPosition::South);
    pageTabWidget->setDocumentMode(true); // hide borders
    layout->addWidget(pageTabWidget);
}

void EXDPart::loadSheet(const QString &name, physis_Buffer buffer, const QString &definitionPath)
{
    pageTabWidget->clear();

    QFile definitionFile(definitionPath);
    definitionFile.open(QIODevice::ReadOnly);

    qInfo() << definitionPath;

    QJsonArray definitionList;
    if (definitionFile.isOpen()) {
        auto document = QJsonDocument::fromJson(definitionFile.readAll());
        definitionList = document.object()[QLatin1String("definitions")].toArray();

        for (auto definition : definitionList) {
            if (definition.toObject().contains(QLatin1String("converter"))
                && definition.toObject()[QLatin1String("converter")].toObject()[QLatin1String("type")].toString() == QStringLiteral("link")) {
                auto linkName = definition.toObject()[QLatin1String("converter")].toObject()[QLatin1String("target")].toString();

                auto path = QStringLiteral("exd/%1.exh").arg(linkName.toLower());
                auto pathStd = path.toStdString();

                auto file = physis_gamedata_extract_file(data, pathStd.c_str());

                auto linkExh = physis_parse_excel_sheet_header(file);
                auto linkExd = physis_gamedata_read_excel_sheet(data, linkName.toStdString().c_str(), linkExh, getSuitableLanguage(linkExh), 0);

                if (linkExd.p_ptr != nullptr) {
                    cachedExcelSheets[linkName] = CachedExcel{linkExh, linkExd};
                }
            }
        }
    }

    auto exh = physis_parse_excel_sheet_header(buffer);

    // ditto
    /*QLayoutItem *child;
    while ((child = headerFormLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    headerFormLayout->addRow(QStringLiteral("Num Rows"), new QLabel(QString::number(exh->row_count)));
    headerFormLayout->addRow(QStringLiteral("Num Columns"), new QLabel(QString::number(exh->column_count)));
    headerFormLayout->addRow(QStringLiteral("Num Pages"), new QLabel(QString::number(exh->page_count)));
    headerFormLayout->addRow(QStringLiteral("Num Languages"), new QLabel(QString::number(exh->language_count)));*/

    for (uint32_t i = 0; i < exh->page_count; i++) {
        auto tableWidget = new QTableWidget();

        tableWidget->setColumnCount(exh->column_count);
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        auto exd = physis_gamedata_read_excel_sheet(data, name.toStdString().c_str(), exh, getSuitableLanguage(exh), i);

        tableWidget->setRowCount(exd.row_count);

        for (unsigned int z = 0; z < exd.column_count; z++) {
            auto columnData = exd.row_data[0].column_data[z];

            QString columnType;
            switch (columnData.tag) {
            case physis_ColumnData::Tag::String:
                columnType = QStringLiteral("String");
                break;
            case physis_ColumnData::Tag::Bool:
                columnType = QStringLiteral("Bool");
                break;
            case physis_ColumnData::Tag::Int8:
                columnType = QStringLiteral("Int8");
                break;
            case physis_ColumnData::Tag::UInt8:
                columnType = QStringLiteral("UInt8");
                break;
            case physis_ColumnData::Tag::Int16:
                columnType = QStringLiteral("Int16");
                break;
            case physis_ColumnData::Tag::UInt16:
                columnType = QStringLiteral("UInt16");
                break;
            case physis_ColumnData::Tag::Int32:
                columnType = QStringLiteral("Int32");
                break;
            case physis_ColumnData::Tag::UInt32:
                columnType = QStringLiteral("UInt32");
                break;
            case physis_ColumnData::Tag::Float32:
                columnType = QStringLiteral("Float32");
                break;
            case physis_ColumnData::Tag::Int64:
                columnType = QStringLiteral("Int64");
                break;
            case physis_ColumnData::Tag::UInt64:
                columnType = QStringLiteral("UInt64");
                break;
            }

            // TODO: index could be different
            if (z >= 0 && z < definitionList.size()) {
                columnType = definitionList[z].toObject()[QLatin1String("name")].toString();
            }

            auto headerItem = new QTableWidgetItem();
            headerItem->setText(columnType);

            tableWidget->setHorizontalHeaderItem(z, headerItem);
        }

        for (unsigned int j = 0; j < exd.row_count; j++) {
            for (unsigned int z = 0; z < exd.column_count; z++) {
                auto columnData = exd.row_data[j].column_data[z];

                auto [columnString, columnRow] = getColumnData(columnData);

                if (z >= 0 && z < definitionList.size()) {
                    auto definition = definitionList[z].toObject();
                    if (definition.contains(QLatin1String("converter"))
                        && definition[QLatin1String("converter")].toObject()[QLatin1String("type")].toString() == QLatin1String("link")) {
                        auto linkName = definition[QLatin1String("converter")].toObject()[QLatin1String("target")].toString();

                        if (cachedExcelSheets.contains(linkName)) {
                            auto cachedExcel = cachedExcelSheets[linkName];
                            if (static_cast<unsigned int>(columnRow) < cachedExcel.exd.row_count) {
                                auto [colString, _] = getColumnData(*cachedExcel.exd.row_data[columnRow].column_data);
                                columnString = colString;
                            }
                        }
                    }
                }

                auto newItem = new QTableWidgetItem(columnString);

                tableWidget->setItem(j, z, newItem);
            }
        }

        tableWidget->resizeColumnsToContents();

        pageTabWidget->addTab(tableWidget, QStringLiteral("Page %1").arg(i));
    }

    // Expand the tabs and hide the tab bar if there's only one page
    // (it effectively makes the tab bar useless, so why show it?)
    pageTabWidget->tabBar()->setExpanding(true);
    pageTabWidget->tabBar()->setVisible(exh->page_count > 1);
}

Language EXDPart::getSuitableLanguage(physis_EXH *pExh)
{
    for (uint32_t i = 0; i < pExh->language_count; i++) {
        if (pExh->languages[i] == Language::English) {
            return Language::English;
        }
    }

    return Language::None;
}

std::pair<QString, int> EXDPart::getColumnData(physis_ColumnData &columnData)
{
    QString columnString;
    int columnRow;
    switch (columnData.tag) {
    case physis_ColumnData::Tag::String:
        columnString = QString::fromStdString(columnData.string._0);
        break;
    case physis_ColumnData::Tag::Bool:
        columnString = columnData.bool_._0 ? QStringLiteral("True") : QStringLiteral("False");
        break;
    case physis_ColumnData::Tag::Int8:
        columnString = QString::number(columnData.int8._0);
        columnRow = columnData.int8._0;
        break;
    case physis_ColumnData::Tag::UInt8:
        columnString = QString::number(columnData.u_int8._0);
        columnRow = columnData.u_int8._0;
        break;
    case physis_ColumnData::Tag::Int16:
        columnString = QString::number(columnData.int16._0);
        columnRow = columnData.int16._0;
        break;
    case physis_ColumnData::Tag::UInt16:
        columnString = QString::number(columnData.u_int16._0);
        columnRow = columnData.u_int16._0;
        break;
    case physis_ColumnData::Tag::Int32:
        columnString = QString::number(columnData.int32._0);
        columnRow = columnData.int32._0;
        break;
    case physis_ColumnData::Tag::UInt32:
        columnString = QString::number(columnData.u_int32._0);
        columnRow = columnData.u_int32._0;
        break;
    case physis_ColumnData::Tag::Float32:
        columnString = QString::number(columnData.float32._0);
        break;
    case physis_ColumnData::Tag::Int64:
        columnString = QString::number(columnData.int64._0);
        columnRow = columnData.int64._0;
        break;
    case physis_ColumnData::Tag::UInt64:
        columnString = QString::number(columnData.u_int64._0);
        columnRow = columnData.u_int64._0;
        break;
    }

    return {columnString, columnRow};
}

#include "moc_exdpart.cpp"