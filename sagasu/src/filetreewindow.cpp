// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetreewindow.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QTreeWidget>

FileTreeWindow::FileTreeWindow(const QString &gamePath, GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
    , m_gamePath(gamePath)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_searchModel = new QSortFilterProxyModel();
    m_searchModel->setRecursiveFilteringEnabled(true);
    m_searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    auto searchLayout = new QHBoxLayout();
    layout->addLayout(searchLayout);

    auto searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(QStringLiteral("Search..."));
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_searchModel->setFilterRegularExpression(text);
    });
    searchLayout->addWidget(searchEdit);

    m_unknownCheckbox = new QCheckBox();
    m_unknownCheckbox->setToolTip(QStringLiteral("Show unknown files and folders."));
    connect(m_unknownCheckbox, &QCheckBox::clicked, this, [this] {
        refreshModel();
    });
    searchLayout->addWidget(m_unknownCheckbox);

    auto treeWidget = new QTreeView();
    treeWidget->setModel(m_searchModel);
    layout->addWidget(treeWidget);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, &QTreeWidget::customContextMenuRequested, this, [this, treeWidget](const QPoint &pos) {
        auto index = treeWidget->indexAt(pos);

        if (index.isValid()) {
            auto path = m_searchModel->data(index, Qt::UserRole).toString();

            auto menu = new QMenu();

            auto extractAction = menu->addAction(QStringLiteral("Extract.."));
            connect(extractAction, &QAction::triggered, this, [=] {
                Q_EMIT extractFile(path);
            });

            menu->exec(treeWidget->mapToGlobal(pos));
        }
    });

    connect(treeWidget, &QTreeView::clicked, [this, treeWidget](const QModelIndex &item) {
        if (item.isValid()) {
            auto path = m_searchModel->data(item, Qt::UserRole).toString();
            Q_EMIT pathSelected(path);
        }
    });

    refreshModel();
}

void FileTreeWindow::refreshModel()
{
    // TODO: this should really be handled by the proxy
    m_fileModel = new FileTreeModel(m_unknownCheckbox->isChecked(), m_gamePath, data);
    m_searchModel->setSourceModel(m_fileModel);
}

#include "moc_filetreewindow.cpp"