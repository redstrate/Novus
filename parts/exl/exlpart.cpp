// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exlpart.h"

#include <KLocalizedString>
#include <QFile>
#include <QJsonObject>
#include <QVBoxLayout>
#include <physis.hpp>

EXLPart::EXLPart(QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    m_tableWidget = new QTableWidget();
    layout->addWidget(m_tableWidget);
    setLayout(layout);
}

void EXLPart::load(const Platform platform, const physis_Buffer file) const
{
    const auto exl = physis_exl_parse(platform, file);
    if (exl.entry_count > 0) {
        m_tableWidget->clear();
        m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        m_tableWidget->setColumnCount(2);
        m_tableWidget->setRowCount(exl.entry_count);

        m_tableWidget->setHorizontalHeaderLabels({i18nc("@title:column", "Key"), i18nc("@title:column", "Value")});

        for (int i = 0; i < exl.entry_count; i++) {
            const auto keyItem = new QTableWidgetItem(QLatin1String(exl.entry_keys[i]));
            const auto valueItem = new QTableWidgetItem(QString::number(exl.entry_values[i]));

            m_tableWidget->setItem(i, 0, keyItem);
            m_tableWidget->setItem(i, 1, valueItem);
        }

        m_tableWidget->resizeColumnsToContents();
    }
}

#include "moc_exlpart.cpp"
