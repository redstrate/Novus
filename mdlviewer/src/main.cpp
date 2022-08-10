#include <QApplication>
#include <physis.hpp>

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow w(physis_gamedata_initialize(argv[1]));
    w.show();

    return app.exec();
}