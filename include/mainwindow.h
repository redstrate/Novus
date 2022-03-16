#pragma once

#include <QMainWindow>

class GameData;

class MainWindow : public QMainWindow {
public:
    MainWindow(GameData& data);

private:
    GameData& data;
};