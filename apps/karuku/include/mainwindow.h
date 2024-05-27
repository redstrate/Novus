// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>
#include <QNetworkAccessManager>

struct GameData;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(GameData *data);

private:
    void setupActions();

    GameData *data = nullptr;
    QNetworkAccessManager *mgr = nullptr;
};