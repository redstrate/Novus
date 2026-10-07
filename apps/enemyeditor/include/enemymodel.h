// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"

#include <QAbstractListModel>
#include <QImage>

class MDLPart;

class EnemyModel : public QAbstractTableModel
{
public:
    explicit EnemyModel(FileCache &cache);

    enum CustomRole {
        IdsRole = Qt::UserRole,
        MdlPath,
        MtrlPaths,
    };

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QPair<QImage, QList<QString>> renderModel(const QString &mdlPath, const QString &baseMtrlPath) const;

    struct Enemy {
        QList<uint32_t> ids;
        QString mdlPath;
        QString baseMtrlPath;
    };
    QList<Enemy *> m_enemies;
    QList<QString> m_seenEnemies;

    MDLPart *m_part;
    FileCache &m_cache;
    QHash<QString, QPair<QImage, QList<QString>>> *m_imageCache = nullptr;
};
