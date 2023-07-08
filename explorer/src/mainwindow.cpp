#include "mainwindow.h"

#include <QDesktopServices>
#include <QAction>
#include <QMenuBar>
#include <QUrl>
#include <QApplication>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTreeWidget>
#include <QDebug>

#include "filetreewindow.h"
#include "filepropertieswindow.h"
#include "aboutwindow.h"

MainWindow::MainWindow(GameData* data) : data(data) {
    setWindowTitle("explorer");

    auto helpMenu = menuBar()->addMenu("Help");

    auto donateAction = helpMenu->addAction("Donate");
    connect(donateAction, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl("https://redstrate.com/fund"));
    });
    donateAction->setIcon(QIcon::fromTheme("help-donate"));

    helpMenu->addSeparator();

    auto aboutNovusAction = helpMenu->addAction("About explorer");
    aboutNovusAction->setIcon(QIcon::fromTheme("help-about"));
    connect(aboutNovusAction, &QAction::triggered, this, [this] {
        auto window = new AboutWindow(this);
        window->show();
    });

    auto aboutQtAction = helpMenu->addAction("About Qt");
    aboutQtAction->setIcon(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"));
    connect(aboutQtAction, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);

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

