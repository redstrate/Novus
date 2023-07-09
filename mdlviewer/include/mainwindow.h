#pragma once

#include <QMainWindow>

#include "filecache.h"

struct GameData;
class MDLPart;

class MainWindow : public QMainWindow {
public:
    MainWindow(GameData* data);

private:
    GameData* data = nullptr;
    MDLPart* part = nullptr;
    FileCache cache;
};