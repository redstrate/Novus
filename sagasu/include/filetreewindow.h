// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMdiSubWindow>
#include <physis.hpp>

struct PathPart {
    uint32_t crcHash;
    QMap<QString, PathPart> children;
};

class FileTreeWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FileTreeWindow(GameData *data, QWidget *parent = nullptr);

private:
    GameData *data = nullptr;

    void addPath(QString path);
    void addUnknownPath(QString knownDirectory, uint32_t crcHash);
    void traversePart(QList<QString> tokens, PathPart &part, QString pathSoFar);
    std::tuple<bool, QString> traverseUnknownPath(uint32_t crcHash, PathPart &part, QString pathSoFar);

    QMap<QString, PathPart> rootParts;

    void addPaths(QTreeWidget *pWidget);

    QTreeWidgetItem *addPartAndChildren(const QString &qString, const PathPart &part, const QString &pathSoFar);

Q_SIGNALS:
    void openFileProperties(QString path);
};