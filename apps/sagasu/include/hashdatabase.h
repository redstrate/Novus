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
    void importFileList(const QByteArray &file);

    QVector<QString> getKnownFolders();

    bool knowsFile(uint32_t i) const;
    bool knowsPath(uint32_t i) const;

    QString getFilename(uint32_t i) const;
    QString getPath(uint32_t i) const;

private:
    void cacheDatabase();

    QSqlDatabase m_db;

    // Database transactions are super slow, so we keep a copy in memory
    QHash<uint32_t, QString> m_fileHashes;
    QHash<uint32_t, QString> m_folderHashes;
    QHash<uint32_t, QString> m_pathHashes;
};
