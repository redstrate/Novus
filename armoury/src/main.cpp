// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <physis.hpp>

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    physis_initialize_logging();

    MainWindow w(physis_gamedata_initialize(argv[1]));
    w.show();

    return QApplication::exec();
}