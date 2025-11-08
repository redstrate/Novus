// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hashdatabase.h"

#include <QDir>
#include <QFile>
#include <QSqlDriver>
#include <QSqlError>
#include <QStandardPaths>
#include <physis.hpp>

HashDatabase::HashDatabase(QObject *parent)
    : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));

    const QDir appDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDir.exists())
        appDir.mkpath(appDir.absolutePath());

    m_db.setDatabaseName(appDir.absoluteFilePath(QStringLiteral("paths.db")));

    if (!m_db.open()) {
        qFatal() << "Failed to open custom db!";
    }

    QSqlQuery query;
    query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS folder_hashes (hash INTEGER PRIMARY KEY, name TEXT NOT NULL)"));
    query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS file_hashes (hash INTEGER PRIMARY KEY, name TEXT NOT NULL)"));
    query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS path_hashes (hash INTEGER PRIMARY KEY, path TEXT NOT NULL)"));

    cacheDatabase();
}

void HashDatabase::addFolder(const QString &folder)
{
    const std::string folderStd = folder.toStdString();
    const auto hash = physis_generate_partial_hash(folderStd.c_str());

    QSqlQuery query;
    query.prepare(
        QStringLiteral("REPLACE INTO folder_hashes (hash, name) "
                       "VALUES (?, ?)"));
    query.addBindValue(hash);
    query.addBindValue(folder);
    query.exec();

    m_folderHashes[hash] = folder;
}

void HashDatabase::addFile(const QString &file)
{
    QString filename = file;
    if (file.contains(QStringLiteral("/"))) {
        int lastSlash = file.lastIndexOf(QStringLiteral("/"));
        filename = file.sliced(lastSlash + 1, file.length() - lastSlash - 1);
    }

    qInfo() << "Adding" << filename;

    const std::string folderStd = filename.toStdString();
    const auto hash = physis_generate_partial_hash(folderStd.c_str());

    QSqlQuery query;
    query.prepare(
        QStringLiteral("REPLACE INTO file_hashes (hash, name) "
                       "VALUES (?, ?)"));
    query.addBindValue(hash);
    query.addBindValue(filename);
    query.exec();

    m_fileHashes[hash] = filename;
}

QVector<QString> HashDatabase::getKnownFolders()
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.exec(QStringLiteral("SELECT name FROM folder_hashes"));

    QVector<QString> folders;
    while (query.next()) {
        QString country = query.value(0).toString();
        folders.push_back(country);
    }

    return folders;
}

bool HashDatabase::knowsFile(const uint32_t i) const
{
    return m_fileHashes.contains(i);
}

bool HashDatabase::knowsPath(const uint32_t i) const
{
    return m_pathHashes.contains(i);
}

QString HashDatabase::getFilename(const uint32_t i) const
{
    return m_fileHashes.value(i);
}

QString HashDatabase::getPath(const uint32_t i) const
{
    return m_pathHashes.value(i);
}

void HashDatabase::importFileList(const QByteArray &file)
{
    QVariantList folderNames, folderHashes;
    QVariantList fileNames, fileHashes;

    QSqlQuery folderQuery;
    folderQuery.prepare(
        QStringLiteral("REPLACE INTO folder_hashes (hash, name) "
                       "VALUES (?, ?)"));

    QSqlQuery fileQuery;
    fileQuery.prepare(
        QStringLiteral("REPLACE INTO file_hashes (hash, name) "
                       "VALUES (?, ?)"));

    QSqlQuery pathQuery;
    pathQuery.prepare(
        QStringLiteral("REPLACE INTO path_hashes (hash, path) "
                       "VALUES (?, ?)"));

    struct PreparedRow {
        uint folderHash;
        QString folderName;
        uint fileHash;
        QString fileName;
        uint pathHash;
        QString path;
    };
    std::vector<PreparedRow> preparedRows;

    QTextStream stream(file);
    stream.readLine(); // skip header
    while (!stream.atEnd()) {
        const QStringList parts = stream.readLine().split(QLatin1Char(','));

        const QString &folderHash = parts[1];
        const QString &fileHash = parts[2];
        const QString &fullHash = parts[3];
        const QString &path = parts[4];

        QString filename;
        QString foldername;
        if (path.contains(QStringLiteral("/"))) {
            int lastSlash = path.lastIndexOf(QStringLiteral("/"));
            filename = path.sliced(lastSlash + 1, path.length() - lastSlash - 1);
            foldername = path.left(lastSlash);
        } else {
            Q_UNREACHABLE(); // root files don't exist in FFXIV
        }

        preparedRows.push_back(PreparedRow{
            .folderHash = folderHash.toUInt(),
            .folderName = foldername,
            .fileHash = fileHash.toUInt(),
            .fileName = filename,
            .pathHash = fullHash.toUInt(),
            .path = path,
        });
    }

    qInfo() << "Finished preparing the rows! Now inserting into the database...";

    m_db.transaction();
    for (const auto &row : preparedRows) {
        // execBatch is too slow as the QSQLITE doesn't support batch operations
        if (!row.folderName.isEmpty()) {
            folderQuery.bindValue(0, row.folderHash);
            folderQuery.bindValue(1, row.folderName);
            folderQuery.exec();
        }

        fileQuery.bindValue(0, row.fileHash);
        fileQuery.bindValue(1, row.fileName);
        fileQuery.exec();

        pathQuery.bindValue(0, row.pathHash);
        pathQuery.bindValue(1, row.path);
        pathQuery.exec();
    }
    m_db.commit();

    qInfo() << "Finished database import!";

    // reload cache
    cacheDatabase();
}

void HashDatabase::cacheDatabase()
{
    qInfo() << "Caching database...";

    m_fileHashes.clear();
    m_folderHashes.clear();

    // file hashes
    {
        QSqlQuery query;
        query.setForwardOnly(true);
        query.prepare(QStringLiteral("SELECT hash, name FROM file_hashes;"));
        query.exec();

        while (query.next()) {
            m_fileHashes.insert(query.value(0).toUInt(), query.value(1).toString());
        }
    }

    // folder hashes
    {
        QSqlQuery query;
        query.setForwardOnly(true);
        query.prepare(QStringLiteral("SELECT hash, name FROM folder_hashes;"));
        query.exec();

        while (query.next()) {
            m_folderHashes.insert(query.value(0).toUInt(), query.value(1).toString());
        }
    }

    // path hashes
    {
        QSqlQuery query;
        query.setForwardOnly(true);
        query.prepare(QStringLiteral("SELECT hash, path FROM path_hashes;"));
        query.exec();

        while (query.next()) {
            m_pathHashes.insert(query.value(0).toUInt(), query.value(1).toString());
        }
    }

    qInfo() << "Finished caching!";
}

#include "moc_hashdatabase.cpp"
