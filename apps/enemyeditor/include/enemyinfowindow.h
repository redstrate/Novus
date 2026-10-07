// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>

class FileCache;
class KawariApi;

class EnemyInfoWindow : public QDialog
{
public:
    explicit EnemyInfoWindow(FileCache &cache,
                             KawariApi *kawariApi,
                             QList<uint32_t> ids,
                             const QString &mdlPath,
                             const QList<QString> &mtrlPaths,
                             QWidget *parent);
};
