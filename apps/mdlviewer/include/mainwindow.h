// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"

#include <KXmlGuiWindow>
#include <QFormLayout>

struct GameData;
class MDLPart;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(GameData *data);

private:
    void setupActions();

    GameData *data = nullptr;
    MDLPart *part = nullptr;
    FileCache cache;
    QFormLayout *m_detailsLayout = nullptr;
};