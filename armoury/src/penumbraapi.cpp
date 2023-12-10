// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "penumbraapi.h"

PenumbraApi::PenumbraApi(QObject *parent)
    : QObject(parent)
    , m_mgr(new QNetworkAccessManager(this))
{
}

void PenumbraApi::redrawAll()
{
    m_mgr->post(QNetworkRequest(QUrl(QStringLiteral("http://localhost:42069/api/redrawAll"))), QByteArray{});
}
