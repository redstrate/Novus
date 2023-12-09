// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

class HashDatabase : public QObject
{
    Q_OBJECT

public:
    explicit HashDatabase(QObject *parent = nullptr);

    void addFolder(const QString &folder);
    void addFile(const QString &file);

    QVector<QString> getKnownFolders();

    bool knowsFile(uint32_t i);

    QString getFilename(uint32_t i);

private:
    QSqlDatabase m_db;
};