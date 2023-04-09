#include "mainwindow.h"
#include "filetreewindow.h"
#include "filepropertieswindow.h"

#include <QHBoxLayout>
#include <QTableWidget>
#include <QTreeWidget>
#include <QDebug>

MainWindow::MainWindow(GameData* data) : data(data) {
    setWindowTitle("explorer");

    mdiArea = new QMdiArea();
    setCentralWidget(mdiArea);

    auto tree = new FileTreeWindow(data);
    connect(tree, &FileTreeWindow::openFileProperties, this, [=](QString path) {
        qInfo() << "opening properties window for " << path;
        auto window = mdiArea->addSubWindow(new FilePropertiesWindow(data, path));
        window->show();
    });

    mdiArea->addSubWindow(tree);
}

