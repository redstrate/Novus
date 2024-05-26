// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QNetworkAccessManager>

class PenumbraApi : public QObject
{
public:
    explicit PenumbraApi(QObject *parent = nullptr);

public Q_SLOTS:
    void redrawAll();
    void openWindow();

private:
    QNetworkAccessManager *m_mgr = nullptr;
};