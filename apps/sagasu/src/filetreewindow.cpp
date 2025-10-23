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

FileTreeWindow::FileTreeWindow(HashDatabase &database, const QString &gamePath, SqPackResource *data, QWidget *parent)
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

    auto treeWidget = new QTreeView();
    treeWidget->setModel(m_searchModel);
    layout->addWidget(treeWidget);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, &QTreeWidget::customContextMenuRequested, this, [this, treeWidget](const QPoint &pos) {
        auto index = treeWidget->indexAt(pos);

        if (index.isValid()) {
            const auto path = m_searchModel->data(index, FileTreeModel::CustomRoles::PathRole).toString();
            const auto isUnknown = m_searchModel->data(index, FileTreeModel::CustomRoles::IsUnknownRole).toBool();
            const auto isFolder = m_searchModel->data(index, FileTreeModel::CustomRoles::IsFolderRole).toBool();

            auto menu = new QMenu();

            // It doesn't make sense to extract folders
            if (!isFolder) {
                auto extractAction = menu->addAction(i18nc("@action:inmenu", "Extract…"));
                extractAction->setIcon(QIcon::fromTheme(QStringLiteral("archive-extract-symbolic")));
                connect(extractAction, &QAction::triggered, this, [this, path] {
                    Q_EMIT extractFile(path);
                });
            }

            // It doesn't make sense to copy file paths for... a file or folder that doesn't have a path.
            if (!isUnknown) {
                auto copyFilePathAction = menu->addAction(i18nc("@action:inmenu", "Copy file path"));
                copyFilePathAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy-symbolic")));
                connect(copyFilePathAction, &QAction::triggered, this, [this, path] {
                    QClipboard *clipboard = QGuiApplication::clipboard();
                    clipboard->setText(path);
                });
            }

            menu->exec(treeWidget->mapToGlobal(pos));
        }
    });

    connect(treeWidget, &QTreeView::clicked, [this, treeWidget](const QModelIndex &item) {
        if (item.isValid()) {
            const auto isFolder = m_searchModel->data(item, FileTreeModel::CustomRoles::IsFolderRole).toBool();
            if (isFolder) {
                return;
            }

            const auto path = m_searchModel->data(item, FileTreeModel::CustomRoles::PathRole).toString();
            const auto indexPath = m_searchModel->data(item, FileTreeModel::CustomRoles::IndexPathRole).toString();
            const auto hash = m_searchModel->data(item, FileTreeModel::CustomRoles::HashRole).value<Hash>();

            Q_EMIT pathSelected(indexPath, hash, path);
        }
    });

    refreshModel();
}

void FileTreeWindow::refreshModel()
{
    // TODO: this should really be handled by the proxy
    m_fileModel = new FileTreeModel(m_database, m_showUnknown, m_gamePath, data);
    m_searchModel->setSourceModel(m_fileModel);
}

void FileTreeWindow::setShowUnknown(bool show)
{
    m_showUnknown = show;
    refreshModel();
}

#include "moc_filetreewindow.cpp"
