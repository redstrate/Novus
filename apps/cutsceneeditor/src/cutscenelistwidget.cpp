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

    m_cutsceneSearchModel = new QSortFilterProxyModel(this);
    m_cutsceneSearchModel->setRecursiveFilteringEnabled(true);
    m_cutsceneSearchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    m_questSearchModel = new QSortFilterProxyModel(this);
    m_questSearchModel->setRecursiveFilteringEnabled(true);
    m_questSearchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    const auto searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_cutsceneSearchModel->setFilterFixedString(text);
        m_questSearchModel->setFilterFixedString(text);
    });
    layout->addWidget(searchEdit);

    m_tabWidget = new QTabWidget();
    layout->addWidget(m_tabWidget);

    const auto originalModel = new QStandardItemModel(this);
    m_cutsceneSearchModel->setSourceModel(originalModel);

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
            item->setData(qPath);

            originalModel->insertRow(originalModel->rowCount(), item);

            physis_free_row(&cutsceneRow, cutsceneExh.column_count);
        }
    }

    m_cutsceneListWidget = new QListView();
    m_cutsceneListWidget->setEditTriggers(QListView::EditTrigger::NoEditTriggers);
    m_cutsceneListWidget->setModel(m_cutsceneSearchModel);

    connect(m_cutsceneListWidget, &QListView::activated, this, &CutsceneListWidget::accept);

    m_tabWidget->addTab(m_cutsceneListWidget, i18n("Cutscene"));

    const auto originalJournalModel = new QStandardItemModel(this);
    m_questSearchModel->setSourceModel(originalJournalModel);

    const auto completeJournalExhFile = m_cache.read(QStringLiteral("exd/CompleteJournal.exh"));
    if (completeJournalExhFile.size == 0) {
        qWarning() << "Could not load CompleteJournal Excel header!";
    }
    const auto completeJournalExh = physis_exh_parse(m_cache.platform(), completeJournalExhFile);
    const auto completeJournalSheet = m_cache.readExcelSheet(QStringLiteral("CompleteJournal"), &completeJournalExh, getLanguage());

    for (uint32_t i = 0; i < completeJournalExh.pages[0].row_count; i++) {
        auto journalRow = physis_excel_get_row(&completeJournalSheet, i);
        if (journalRow.columns) {
            const char *name = journalRow.columns[5].string._0;
            if (strlen(name) == 0) {
                physis_free_row(&journalRow, completeJournalExh.column_count);
                continue;
            }

            QString qName = QString::fromStdString(name);

            int z = 1;
            for (uint32_t j = 6; j < 30; j++) {
                const auto cutsceneRowId = journalRow.columns[j].int32._0;
                if (cutsceneRowId > 0) {
                    auto cutsceneRow = physis_excel_get_row(&cutsceneSheet, cutsceneRowId);
                    const char *path = cutsceneRow.columns[0].string._0;
                    const QString qPath = QString::fromStdString(path);
                    physis_free_row(&cutsceneRow, cutsceneExh.column_count);

                    const auto item = new QStandardItem();
                    item->setText(i18n("%1 (Cutscene #%2)", qName, z++));
                    item->setData(qPath);

                    originalJournalModel->insertRow(originalJournalModel->rowCount(), item);
                }
            }

            physis_free_row(&journalRow, completeJournalExh.column_count);
        }
    }

    physis_sqpack_free_excel_sheet(&cutsceneSheet);
    physis_exh_free(&cutsceneExh);

    physis_sqpack_free_excel_sheet(&completeJournalSheet);
    physis_exh_free(&completeJournalExh);

    m_questListWidget = new QListView();
    m_questListWidget->setEditTriggers(QListView::EditTrigger::NoEditTriggers);
    m_questListWidget->setModel(m_questSearchModel);

    connect(m_questListWidget, &QListView::activated, this, &CutsceneListWidget::accept);

    m_tabWidget->addTab(m_questListWidget, i18n("Quest"));

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CutsceneListWidget::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CutsceneListWidget::reject);
    layout->addWidget(buttonBox);

    // Disable when there's no selection
    connect(m_cutsceneListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, buttonBox] {
        const QListView *listView = nullptr;
        if (m_tabWidget->currentIndex() == 0) {
            listView = m_cutsceneListWidget;
        } else {
            listView = m_questListWidget;
        }
        buttonBox->button(QDialogButtonBox::Open)->setEnabled(listView->selectionModel()->hasSelection());
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
    const QListView *listView = nullptr;
    const QSortFilterProxyModel *searchModel = nullptr;
    if (m_tabWidget->currentIndex() == 0) {
        listView = m_cutsceneListWidget;
        searchModel = m_cutsceneSearchModel;
    } else {
        listView = m_questListWidget;
        searchModel = m_questSearchModel;
    }

    // Figure out the selection first
    const auto index = listView->selectionModel()->selectedIndexes().constFirst();
    if (index.isValid()) {
        m_acceptedCutscene = searchModel->mapToSource(index).data(Qt::UserRole + 1).toString();
    }

    QDialog::accept();
}

#include "moc_cutscenelistwidget.cpp"
