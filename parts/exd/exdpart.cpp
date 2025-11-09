// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exdpart.h"

#include "excelmodel.h"

#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <physis.hpp>

#include "magic_enum.hpp"
#include "schema.h"

#include <QStandardPaths>

EXDPart::EXDPart(SqPackResource *data, AbstractExcelResolver *resolver, QWidget *parent)
    : QWidget(parent)
    , data(data)
    , m_resolver(resolver)
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

void EXDPart::loadSheet(const QString &name, physis_Buffer buffer)
{
    this->name = name;
    exh = physis_parse_excel_sheet_header(buffer);

    languageComboBox->clear();
    for (unsigned int i = 0; i < exh->language_count; i++) {
        // Don't add None to the combo box, the reason for this is because
        // many localized sheets *report* this language but it's usually empty and useless.
        const auto language = exh->languages[i];
        if (language == Language::None) {
            continue;
        }

        const auto itemText = QString::fromUtf8(magic_enum::enum_name(language));
        // Don't add duplicates
        if (languageComboBox->findText(itemText) == -1) {
            languageComboBox->addItem(itemText, static_cast<int>(language));
        }
    }

    // Show the current language as the selected item
    const auto itemText = QString::fromUtf8(magic_enum::enum_name(getSuitableLanguage(exh)));
    languageComboBox->setCurrentText(itemText);

    loadTables();
}

void EXDPart::goToRow(const QString &query)
{
    for (uint32_t i = 0; i < exh->page_count; i++) {
        const auto tableWidget = qobject_cast<QTableView *>(pageTabWidget->widget(i));
        Q_ASSERT(tableWidget);

        for (int row = 0; row < tableWidget->model()->rowCount(); row++) {
            const auto headerItem = tableWidget->model()->headerData(row, Qt::Vertical).toString();
            if (headerItem == query) {
                pageTabWidget->setCurrentIndex(i);
                tableWidget->selectRow(row);
                return;
            }
        }
    }
}

void EXDPart::loadTables()
{
    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QDir schemaDir = dataDir.absoluteFilePath(QStringLiteral("schema"));

    Schema schema(schemaDir.absoluteFilePath(QStringLiteral("%1.yml").arg(name)));

    pageTabWidget->clear();

    for (uint32_t i = 0; i < exh->page_count; i++) {
        auto tableWidget = new QTableView();

        auto exd = physis_gamedata_read_excel_sheet(data, name.toStdString().c_str(), exh, getSuitableLanguage(exh), i);
        if (exd.p_ptr == nullptr) {
            continue;
        }

        tableWidget->setModel(new ExcelModel(*exh, exd, schema, m_resolver, getSuitableLanguage(exh)));
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->resizeColumnsToContents();
        tableWidget->setAlternatingRowColors(true);

        pageTabWidget->addTab(tableWidget, i18nc("@title:tab", "Page %1", i));
    }

    // Expand the tabs and hide the tab bar if there's only one page
    // (it effectively makes the tab bar useless, so why show it?)
    pageTabWidget->tabBar()->setExpanding(true);
    pageTabWidget->tabBar()->setVisible(exh->page_count > 1);
}

Language EXDPart::getSuitableLanguage(physis_EXH *pExh) const
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

#include "moc_exdpart.cpp"
