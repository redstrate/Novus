// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hashdatabase.h"

#include <QFile>
#include <QSqlDriver>
#include <QSqlError>
#include <physis.hpp>

HashDatabase::HashDatabase(QObject *parent)
    : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    m_db.setDatabaseName(QStringLiteral("customdb.db"));

    if (!m_db.open()) {
        qFatal() << "Failed to open custom db!";
    }

    QSqlQuery query;
    query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS folder_hashes (hash INTEGER PRIMARY KEY, name TEXT NOT NULL)"));
    query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS file_hashes (hash INTEGER PRIMARY KEY, name TEXT NOT NULL)"));
}

void HashDatabase::addFolder(const QString &folder)
{
    std::string folderStd = folder.toStdString();

    QSqlQuery query;
    query.prepare(
        QStringLiteral("REPLACE INTO folder_hashes (hash, name) "
                       "VALUES (?, ?)"));
    query.addBindValue(physis_generate_partial_hash(folderStd.c_str()));
    query.addBindValue(folder);
    query.exec();
}

void HashDatabase::addFile(const QString &file)
{
    QString filename = file;
    if (file.contains(QStringLiteral("/"))) {
        int lastSlash = file.lastIndexOf(QStringLiteral("/"));
        filename = file.sliced(lastSlash + 1, file.length() - lastSlash - 1);
    }

    qInfo() << "Adding" << filename;

    std::string folderStd = filename.toStdString();

    QSqlQuery query;
    query.prepare(
        QStringLiteral("REPLACE INTO file_hashes (hash, name) "
                       "VALUES (?, ?)"));
    query.addBindValue(physis_generate_partial_hash(folderStd.c_str()));
    query.addBindValue(filename);
    query.exec();
}

QVector<QString> HashDatabase::getKnownFolders()
{
    QSqlQuery query;
    query.exec(QStringLiteral("SELECT name FROM folder_hashes"));

    QVector<QString> folders;
    while (query.next()) {
        QString country = query.value(0).toString();
        folders.push_back(country);
    }

    return folders;
}

bool HashDatabase::knowsFile(const uint32_t i)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT(1) FROM file_hashes WHERE hash = ?;"));
    query.addBindValue(i);
    query.exec();

    query.next();

    return query.value(0) == 1;
}

QString HashDatabase::getFilename(const uint32_t i)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT name FROM file_hashes WHERE hash = ?;"));
    query.addBindValue(i);
    query.exec();

    query.next();

    return query.value(0).toString();
}

void HashDatabase::importFileList(const QByteArray &file)
{
    QVariantList folderNames, folderHashes;
    QVariantList fileNames, fileHashes;

    m_db.transaction();

    QSqlQuery folderQuery;
    folderQuery.prepare(
        QStringLiteral("REPLACE INTO folder_hashes (hash, name) "
                       "VALUES (?, ?)"));

    QSqlQuery fileQuery;
    fileQuery.prepare(
        QStringLiteral("REPLACE INTO file_hashes (hash, name) "
                       "VALUES (?, ?)"));

    QTextStream stream(file);
    stream.readLine(); // skip header
    while (!stream.atEnd()) {
        const QStringList parts = stream.readLine().split(QLatin1Char(','));

        const QString &folderHash = parts[1];
        const QString &fileHash = parts[2];
        const QString &path = parts[4];

        QString filename;
        QString foldername;
        if (path.contains(QStringLiteral("/"))) {
            int lastSlash = path.lastIndexOf(QStringLiteral("/"));
            filename = path.sliced(lastSlash + 1, path.length() - lastSlash - 1);
            foldername = path.left(lastSlash);
        } else {
            filename = path;
        }

        // execBatch is too slow as the QSQLITE doesn't support batch operations
        if (!foldername.isEmpty()) {
            folderQuery.bindValue(0, folderHash.toUInt());
            folderQuery.bindValue(1, foldername);
            folderQuery.exec();
        }

        fileQuery.bindValue(0, fileHash.toUInt());
        fileQuery.bindValue(1, filename);
        fileQuery.exec();
    }

    m_db.commit();
}

#include "moc_hashdatabase.cpp"