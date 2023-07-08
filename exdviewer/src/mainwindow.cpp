#include "mainwindow.h"

#include <QHBoxLayout>
#include <QTableWidget>
#include <fmt/core.h>
#include <QListWidget>
#include <physis.hpp>
#include <QDesktopServices>
#include <QAction>
#include <QMenuBar>
#include <QUrl>
#include <QApplication>

#include "exdpart.h"
#include "aboutwindow.h"

MainWindow::MainWindow(GameData* data) : data(data) {
    setWindowTitle("exdviewer");

    auto helpMenu = menuBar()->addMenu("Help");

    auto donateAction = helpMenu->addAction("Donate");
    connect(donateAction, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl("https://redstrate.com/fund"));
    });
    donateAction->setIcon(QIcon::fromTheme("help-donate"));

    helpMenu->addSeparator();

    auto aboutNovusAction = helpMenu->addAction("About exdviewer");
    aboutNovusAction->setIcon(QIcon::fromTheme("help-about"));
    connect(aboutNovusAction, &QAction::triggered, this, [this] {
        auto window = new AboutWindow(this);
        window->show();
    });

    auto aboutQtAction = helpMenu->addAction("About Qt");
    aboutQtAction->setIcon(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"));
    connect(aboutQtAction, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto listWidget = new QListWidget();

    auto names = physis_gamedata_get_all_sheet_names(data);
    for (int i = 0; i < names.name_count; i++) {
        listWidget->addItem(names.names[i]);
    }

    listWidget->setMaximumWidth(200);
    listWidget->sortItems();
    layout->addWidget(listWidget);

    auto exdPart = new EXDPart(data);
    layout->addWidget(exdPart);

    connect(listWidget, &QListWidget::itemClicked, this, [exdPart](QListWidgetItem* item) {
        auto name = item->text().toStdString();
        auto nameLowercase = item->text().toLower().toStdString();

        exdPart->loadSheet(name.c_str());
    });
}