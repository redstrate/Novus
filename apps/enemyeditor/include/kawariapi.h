// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QNetworkAccessManager>

class KawariApi : public QObject
{
public:
    explicit KawariApi(QObject *parent = nullptr);

public Q_SLOTS:
    void spawnBattleNpc(uint32_t id) const;

private:
    QNetworkAccessManager *m_mgr = nullptr;
};
