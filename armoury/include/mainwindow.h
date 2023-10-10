// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QComboBox>
#include <physis.hpp>
#include <unordered_map>

#include "fullmodelviewer.h"
#include "gearview.h"
#include "novusmainwindow.h"
#include "singlegearview.h"

struct GameData;
class FileCache;

class MainWindow : public NovusMainWindow
{
public:
    explicit MainWindow(GameData* data);

private:
    SingleGearView* gearView = nullptr;
    FullModelViewer* fullModelViewer = nullptr;

    GameData& data;
    FileCache cache;
};