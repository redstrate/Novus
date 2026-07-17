// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dicpart.h"

#include <KLocalizedString>
#include <QFile>
#include <QJsonObject>
#include <QVBoxLayout>
#include <physis.hpp>

DicPart::DicPart(QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    m_tableWidget = new QTableWidget();
    layout->addWidget(m_tableWidget);
    setLayout(layout);
}

void DicPart::load(const Platform platform, const physis_Buffer file) const
{
    const auto dic = physis_dictionary_parse(platform, file);
    if (dic.num_words > 0) {
        m_tableWidget->clear();
        m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        m_tableWidget->setColumnCount(1);
        m_tableWidget->setRowCount(dic.num_words);

        m_tableWidget->setHorizontalHeaderLabels({i18nc("@title:column", "Word")});

        for (int i = 0; i < dic.num_words; i++) {
            // TODO: it seems not all utf8 characters are parsed correctly?
            const auto item = new QTableWidgetItem(QString::fromUtf8(dic.words[i]));

            m_tableWidget->setItem(i, 0, item);
        }

        m_tableWidget->resizeColumnsToContents();
    }
}

#include "moc_dicpart.cpp"
