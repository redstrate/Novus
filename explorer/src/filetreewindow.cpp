// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>
#include <QHBoxLayout>
#include <QMenu>
#include <QTreeWidget>

#include "filetreewindow.h"

FileTreeWindow::FileTreeWindow(GameData* data, QWidget* parent) : QWidget(parent), data(data) {
    setWindowTitle(QStringLiteral("File Tree"));

    auto layout = new QHBoxLayout();
    setLayout(layout);

    auto treeWidget = new QTreeWidget();
    treeWidget->setHeaderLabel(QStringLiteral("Name"));
    layout->addWidget(treeWidget);

    addPath(QStringLiteral("common/font/AXIS_12.fdt"));

    addPath(QStringLiteral("exd/root.exl"));

    auto sheetNames = physis_gamedata_get_all_sheet_names(data);

    for(int i = 0; i < sheetNames.name_count; i++) {
        auto sheetName = sheetNames.names[i];
        auto nameLowercase = QString::fromStdString(sheetName).toLower().toStdString();

        addPath(QStringLiteral("exd/") + QLatin1String(nameLowercase.c_str()) + QStringLiteral(".exh"));

        auto exh = physis_gamedata_read_excel_sheet_header(data, sheetName);
        for (int j = 0; j < exh->page_count; j++) {
            for (int z = 0; z < exh->language_count; z++) {
                std::string path = physis_gamedata_get_exd_filename(nameLowercase.c_str(), exh, exh->languages[z], j);

                addPath(QStringLiteral("exd/") + QString::fromStdString(path));
            }
        }
    }

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, &QTreeWidget::customContextMenuRequested, this, [this, treeWidget](const QPoint& pos) {
        auto *item = treeWidget->itemAt(pos);

        if (item != nullptr) {
            auto path = item->data(0, Qt::UserRole).toString();
            qInfo() << path;

            auto menu = new QMenu();

            QAction *propertiesAction = menu->addAction(QStringLiteral("Properties"));
            connect(propertiesAction, &QAction::triggered, this, [=] {
                Q_EMIT openFileProperties(path);
            });

            QPoint pt(pos);
            menu->exec(treeWidget->mapToGlobal(pos));
        }
    });

    addPaths(treeWidget);
}

void FileTreeWindow::addPath(QString path) {
    auto tokens = path.split(QStringLiteral("/"));
    auto nextToken = tokens[0];
    tokens.pop_front();

    traversePart(tokens, rootParts[nextToken], nextToken);
}

void FileTreeWindow::traversePart(QList<QString> tokens, PathPart& part, QString pathSoFar) {
    if(tokens.empty())
        return;

    auto nextToken = tokens[0];
    tokens.pop_front();

    pathSoFar = pathSoFar + QStringLiteral("/") + nextToken;
    part.children[nextToken].crcHash = physis_calculate_hash(pathSoFar.toStdString().c_str());

    traversePart(tokens, part.children[nextToken], pathSoFar);
}

void FileTreeWindow::addPaths(QTreeWidget *pWidget) {
    for(const auto& name : rootParts.keys()) {
        auto item = addPartAndChildren(name, rootParts.value(name), QStringLiteral(""));
        pWidget->addTopLevelItem(item);
    }
}

QTreeWidgetItem* FileTreeWindow::addPartAndChildren(const QString& qString, const PathPart& part, const QString& pathSoFar) {
    QString newPath = pathSoFar.isEmpty() ? qString : pathSoFar + QStringLiteral("/") + qString;

    auto item = new QTreeWidgetItem();
    item->setData(0, Qt::UserRole, newPath);
    item->setText(0, qString);

    for(const auto& name : part.children.keys()) {
        auto childItem = addPartAndChildren(name, part.children.value(name), newPath);
        item->addChild(childItem);
    }

    return item;
}

void FileTreeWindow::addUnknownPath(QString knownDirectory, uint32_t crcHash) {
    auto [found, path] = traverseUnknownPath(crcHash, rootParts[knownDirectory], knownDirectory);
    if(found)
        addPath(path);
    else
        addPath(knownDirectory + QStringLiteral("/Unknown File Hash ") + QString::number(crcHash));
}

std::tuple<bool, QString> FileTreeWindow::traverseUnknownPath(uint32_t crcHash, PathPart &part, QString pathSoFar) {
    if(part.crcHash == crcHash)
        return {true, pathSoFar};

    bool found = false;
    QString childPath = pathSoFar;
    for(auto path : part.children.keys()) {
        if (path.contains(QStringLiteral("Unknown")))
            continue;

        auto [childFound, newPath] = traverseUnknownPath(crcHash, part.children[path], pathSoFar + QStringLiteral("/") + path);
        found |= childFound;
        if(childFound)
            childPath = newPath;
    }

    return {found, childPath};
}

#include "moc_filetreewindow.cpp"