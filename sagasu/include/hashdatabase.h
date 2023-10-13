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
    HashDatabase(QObject *parent = nullptr);

    void addFolder(QString folder);
    void addFile(QString file);

    QVector<QString> getKnownFolders();

    bool knowsFile(const uint32_t i);

    QString getFilename(const uint32_t i);

private:
    QSqlDatabase m_db;
};