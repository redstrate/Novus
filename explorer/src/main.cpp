#include <QApplication>

#include <physis.hpp>
#include <QStyle>

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    physis_initialize_logging();

    app.setStyle("Windows");

    MainWindow w(physis_gamedata_initialize(argv[1]));
    w.show();

    return app.exec();
}