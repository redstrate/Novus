// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetreewindow.h"

#include <KLocalizedString>
#include <QClipboard>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QTimer>
#include <QTreeWidget>

FileTreeWindow::FileTreeWindow(HashDatabase &database, const QString &gamePath, GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
    , m_gamePath(gamePath)
    , m_database(database)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    m_searchModel = new QSortFilterProxyModel();
    m_searchModel->setRecursiveFilteringEnabled(true);
    m_searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    /*auto searchLayout = new QHBoxLayout();
    layout->addLayout(searchLayout);*/

    auto searchEdit = new QLineEdit();

    auto searchTimer = new QTimer();
    searchTimer->setSingleShot(true);
    connect(searchTimer, &QTimer::timeout, m_searchModel, [this, searchEdit] {
        m_searchModel->setFilterFixedString(searchEdit->text());
    });

    searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(searchEdit, &QLineEdit::textChanged, searchTimer, [searchTimer] {
        searchTimer->start();
    });
    layout->addWidget(searchEdit);

    // TODO Restore as an action, later. it's currently pretty useless as-is as it's a "please slow down and crash" checkbox
    /*m_unknownCheckbox = new QCheckBox();
    m_unknownCheckbox->setToolTip(QStringLiteral("Show unknown files and folders."));
    connect(m_unknownCheckbox, &QCheckBox::clicked, this, [this] {
        refreshModel();
    });
    searchLayout->addWidget(m_unknownCheckbox);*/

    auto treeWidget = new QTreeView();
    treeWidget->setModel(m_searchModel);
    layout->addWidget(treeWidget);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, &QTreeWidget::customContextMenuRequested, this, [this, treeWidget](const QPoint &pos) {
        auto index = treeWidget->indexAt(pos);

        if (index.isValid()) {
            auto path = m_searchModel->data(index, Qt::UserRole).toString();

            auto menu = new QMenu();

            auto extractAction = menu->addAction(i18nc("@action:inmenu", "Extract…"));
            extractAction->setIcon(QIcon::fromTheme(QStringLiteral("archive-extract-symbolic")));
            connect(extractAction, &QAction::triggered, this, [this, path] {
                Q_EMIT extractFile(path);
            });

            auto copyFilePathAction = menu->addAction(i18nc("@action:inmenu", "Copy file path"));
            copyFilePathAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy-symbolic")));
            connect(copyFilePathAction, &QAction::triggered, this, [this, path] {
                QClipboard *clipboard = QGuiApplication::clipboard();
                clipboard->setText(path);
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
    m_fileModel = new FileTreeModel(m_database, false, m_gamePath, data);
    m_searchModel->setSourceModel(m_fileModel);
}

#include "moc_filetreewindow.cpp"