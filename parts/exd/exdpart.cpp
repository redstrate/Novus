// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exdpart.h"

#include <KLocalizedString>
#include <QFile>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <physis.hpp>

#include "magic_enum.hpp"

EXDPart::EXDPart(GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    languageComboBox = new QComboBox();
    connect(languageComboBox, &QComboBox::activated, this, [this](const int index) {
        auto selectedLanguage = languageComboBox->itemData(index);
        preferredLanguage = (Language)selectedLanguage.toInt();

        loadTables();
    });
    layout->addWidget(languageComboBox);

    pageTabWidget = new QTabWidget();
    pageTabWidget->setTabPosition(QTabWidget::TabPosition::South);
    pageTabWidget->setDocumentMode(true); // hide borders
    layout->addWidget(pageTabWidget);
}

void EXDPart::loadSheet(const QString &name, physis_Buffer buffer, const QString &definitionPath)
{
    QFile definitionFile(definitionPath);
    definitionFile.open(QIODevice::ReadOnly);

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

    this->name = name;
    exh = physis_parse_excel_sheet_header(buffer);

    languageComboBox->clear();
    for (int i = 0; i < exh->language_count; i++) {
        const auto itemText = QString::fromUtf8(magic_enum::enum_name(exh->languages[i]));
        // Don't add duplicates
        if (languageComboBox->findText(itemText) == -1) {
            languageComboBox->addItem(itemText, static_cast<int>(exh->languages[i]));
        }
    }

    loadTables();
}

void EXDPart::loadTables()
{
    pageTabWidget->clear();

    for (uint32_t i = 0; i < exh->page_count; i++) {
        auto tableWidget = new QTableWidget();

        tableWidget->setColumnCount(exh->column_count);
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        auto exd = physis_gamedata_read_excel_sheet(data, name.toStdString().c_str(), exh, getSuitableLanguage(exh), i);

        tableWidget->setRowCount(exh->row_count);

        for (unsigned int z = 0; z < exd.column_count; z++) {
            auto columnData = exh->column_types[z];

            QString columnType;
            switch (columnData) {
            case ColumnDataType::String:
                columnType = i18n("String");
                break;
            case ColumnDataType::Bool:
                columnType = i18n("Bool");
                break;
            case ColumnDataType::Int8:
                columnType = i18n("Int8");
                break;
            case ColumnDataType::UInt8:
                columnType = i18n("UInt8");
                break;
            case ColumnDataType::Int16:
                columnType = i18n("Int16");
                break;
            case ColumnDataType::UInt16:
                columnType = i18n("UInt16");
                break;
            case ColumnDataType::Int32:
                columnType = i18n("Int32");
                break;
            case ColumnDataType::UInt32:
                columnType = i18n("UInt32");
                break;
            case ColumnDataType::Float32:
                columnType = i18n("Float32");
                break;
            case ColumnDataType::Int64:
                columnType = i18n("Int64");
                break;
            case ColumnDataType::UInt64:
                columnType = i18n("UInt64");
                break;
            }

            // TODO: index could be different
            if (z < definitionList.size()) {
                columnType = definitionList[z].toObject()[QLatin1String("name")].toString();
            }

            auto headerItem = new QTableWidgetItem();
            headerItem->setText(columnType);

            tableWidget->setHorizontalHeaderItem(z, headerItem);
        }

        for (unsigned int j = 0; j < exh->row_count; j++) {
            auto rows = physis_exd_read_row(&exd, exh, j); // TODO: free, use other rows

            for (unsigned int z = 0; z < exd.column_count; z++) {
                auto columnData = rows.row_data[0].column_data[z];

                auto [columnString, columnRow] = getColumnData(columnData);

                if (z < definitionList.size()) {
                    auto definition = definitionList[z].toObject();
                    if (definition.contains(QLatin1String("converter"))
                        && definition[QLatin1String("converter")].toObject()[QLatin1String("type")].toString() == QLatin1String("link")) {
                        auto linkName = definition[QLatin1String("converter")].toObject()[QLatin1String("target")].toString();

                        if (cachedExcelSheets.contains(linkName)) {
                            auto cachedExcel = cachedExcelSheets[linkName];
                            if (static_cast<unsigned int>(columnRow) < cachedExcel.exh->row_count) {
                                // TODO: add back
                                // auto [colString, _] = getColumnData(*cachedExcel.exh->row_data[columnRow].column_data);
                                // columnString = colString;
                            }
                        }
                    }
                }

                auto newItem = new QTableWidgetItem(columnString);

                tableWidget->setItem(j, z, newItem);
            }
        }

        tableWidget->resizeColumnsToContents();

        pageTabWidget->addTab(tableWidget, i18nc("@title:tab", "Page %1", i));
    }

    // Expand the tabs and hide the tab bar if there's only one page
    // (it effectively makes the tab bar useless, so why show it?)
    pageTabWidget->tabBar()->setExpanding(true);
    pageTabWidget->tabBar()->setVisible(exh->page_count > 1);
}

Language EXDPart::getSuitableLanguage(physis_EXH *pExh)
{
    // Find the preferred language first
    for (uint32_t i = 0; i < pExh->language_count; i++) {
        if (pExh->languages[i] == preferredLanguage) {
            return preferredLanguage;
        }
    }

    // Fallback to None
    for (uint32_t i = 0; i < pExh->language_count; i++) {
        if (pExh->languages[i] == Language::None) {
            return Language::None;
        }
    }

    // Then English
    return Language::English;
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
        columnString = columnData.bool_._0 ? i18nc("Value is true", "True") : i18nc("Value is false", "False");
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