#pragma once

#include <QMainWindow>
#include <QMap>
#include <QTreeWidget>
#include <QMdiArea>

struct GameData;

class MainWindow : public QMainWindow {
public:
    MainWindow(GameData* data);

private:

    QMdiArea* mdiArea = nullptr;


    GameData* data;
};