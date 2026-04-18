// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"

#include <QAbstractListModel>
#include <QImage>

class MDLPart;
struct physis_SqPackResource;

class EnemyModel : public QAbstractTableModel
{
public:
    explicit EnemyModel(physis_SqPackResource *resource);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QImage renderModel(uint32_t id, const QString &mdlPath, const QString &mtrlPath) const;

    struct Enemy {
        uint32_t id;
        QImage image;
        QString mdlPath;
        QString mtrlPath;
    };
    QList<Enemy *> m_enemies;

    physis_SqPackResource *m_resource;
    MDLPart *m_part;
    FileCache *m_cache;
};
