// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetreewindow.h"

#include <QHBoxLayout>
#include <QMenu>
#include <QTreeWidget>

FileTreeWindow::FileTreeWindow(QString gamePath, GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    setWindowTitle(QStringLiteral("File Tree"));

    auto layout = new QHBoxLayout();
    setLayout(layout);

    m_fileModel = new FileTreeModel(gamePath, data);

    auto treeWidget = new QTreeView();
    treeWidget->setModel(m_fileModel);
    layout->addWidget(treeWidget);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, &QTreeWidget::customContextMenuRequested, this, [this, treeWidget](const QPoint &pos) {
        auto index = treeWidget->indexAt(pos);

        if (index.isValid()) {
            auto path = m_fileModel->data(index, Qt::UserRole).toString();

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
            auto path = m_fileModel->data(item, Qt::UserRole).toString();
            Q_EMIT pathSelected(path);
        }
    });
}

#include "moc_filetreewindow.cpp"