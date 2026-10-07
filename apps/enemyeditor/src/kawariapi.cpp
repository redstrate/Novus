// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kawariapi.h"

#include <QUrlQuery>

KawariApi::KawariApi(QObject *parent)
    : QObject(parent)
    , m_mgr(new QNetworkAccessManager(this))
{
}

void KawariApi::spawnBattleNpc(const uint32_t id) const
{
    // TODO: hardcoded to this port
    QUrl url(QStringLiteral("http://localhost:21057/spawn_bnpc"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("bnpc_base_id"), QString::number(id));
    url.setQuery(query);
    m_mgr->post(QNetworkRequest(url), QByteArray{});
}
