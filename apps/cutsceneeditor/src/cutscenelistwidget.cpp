// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cutscenelistwidget.h"

#include "filecache.h"
#include "magic_enum.hpp"
#include "settings.h"

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

CutsceneListWidget::CutsceneListWidget(FileCache &cache, QWidget *parent)
    : QDialog(parent)
    , m_cache(cache)
{
    setModal(true);
    setMinimumSize(QSize(640, 480));

    const auto layout = new QVBoxLayout();
    setLayout(layout);

    m_searchModel = new QSortFilterProxyModel(this);
    m_searchModel->setRecursiveFilteringEnabled(true);
    m_searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    const auto searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_searchModel->setFilterFixedString(text);
    });
    layout->addWidget(searchEdit);

    const auto originalModel = new QStandardItemModel(this);
    m_searchModel->setSourceModel(originalModel);

    const auto cutsceneExhFile = m_cache.read(QStringLiteral("exd/Cutscene.exh"));
    if (cutsceneExhFile.size == 0) {
        qWarning() << "Could not load Cutscene Excel header!";
    }
    const auto cutsceneExh = physis_exh_parse(m_cache.platform(), cutsceneExhFile);
    const auto cutsceneSheet = m_cache.readExcelSheet(QStringLiteral("Cutscene"), &cutsceneExh, Language::None);

    for (uint32_t i = 0; i < cutsceneExh.pages[0].row_count; i++) {
        auto cutsceneRow = physis_excel_get_row(&cutsceneSheet, i);
        if (cutsceneRow.columns) {
            const char *path = cutsceneRow.columns[0].string._0;
            if (strlen(path) == 0) {
                physis_free_row(&cutsceneRow, cutsceneExh.column_count);
                continue;
            }

            QString qPath = QString::fromStdString(path);

            const auto item = new QStandardItem();
            item->setText(qPath);

            originalModel->insertRow(originalModel->rowCount(), item);

            physis_free_row(&cutsceneRow, cutsceneExh.column_count);
        }
    }

    physis_sqpack_free_excel_sheet(&cutsceneSheet);

    physis_exh_free(&cutsceneExh);

    m_listWidget = new QListView();
    m_listWidget->setEditTriggers(QListView::EditTrigger::NoEditTriggers);
    m_listWidget->setModel(m_searchModel);

    connect(m_listWidget, &QListView::activated, this, &CutsceneListWidget::accept);

    layout->addWidget(m_listWidget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CutsceneListWidget::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CutsceneListWidget::reject);
    layout->addWidget(buttonBox);

    // Disable when there's no selection
    connect(m_listWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, buttonBox] {
        buttonBox->button(QDialogButtonBox::Open)->setEnabled(m_listWidget->selectionModel()->hasSelection());
    });

    // And it should be disabled by default
    buttonBox->button(QDialogButtonBox::Open)->setEnabled(false);
}

QString CutsceneListWidget::acceptedCutscene() const
{
    return m_acceptedCutscene;
}

void CutsceneListWidget::accept()
{
    // Figure out the selection first
    const auto index = m_listWidget->selectionModel()->selectedIndexes().constFirst();
    if (index.isValid()) {
        m_acceptedCutscene = m_searchModel->mapToSource(index).data(Qt::DisplayRole).toString();
    }

    QDialog::accept();
}

#include "moc_cutscenelistwidget.cpp"
