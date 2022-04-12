#pragma once

#include <QMainWindow>

#include "renderer.hpp"

class GameData;

class MainWindow : public QMainWindow {
public:
    MainWindow(GameData& data);

private:
    GameData& data;

    Renderer* renderer;
};