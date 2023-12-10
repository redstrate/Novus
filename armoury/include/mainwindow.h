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
class PenumbraApi;

class MainWindow : public NovusMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(GameData *data);

protected:
    void setupAdditionalMenus(QMenuBar *menuBar) override;

private:
    SingleGearView *gearView = nullptr;
    FullModelViewer *fullModelViewer = nullptr;

    GameData &data;
    FileCache cache;
    PenumbraApi *m_api = nullptr;
};