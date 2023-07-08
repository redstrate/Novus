#pragma once

#include <QComboBox>
#include <QMainWindow>
#include <fmt/format.h>
#include <physis.hpp>
#include <unordered_map>

#include "fullmodelviewer.h"
#include "gearview.h"
#include "singlegearview.h"

struct GameData;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(GameData* data);

private:
    SingleGearView* gearView = nullptr;
    FullModelViewer* fullModelViewer = nullptr;

    GameData& data;
};