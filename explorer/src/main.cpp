#include <QApplication>

#include "mainwindow.h"
#include "gamedata.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    GameData data(argv[1]);

    MainWindow w(data);
    w.show();

    return app.exec();
}