#pragma once

#include <QMainWindow>

struct GameData;

class MainWindow : public QMainWindow {
public:
    MainWindow(GameData* data);

private:
    GameData* data = nullptr;
};