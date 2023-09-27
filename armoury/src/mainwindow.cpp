// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QTimer>

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenuBar>
#include <QPushButton>
#include <QTreeWidget>
#include <glm/gtc/type_ptr.hpp>
#include <magic_enum.hpp>
#include <physis.hpp>

#include "cmpeditor.h"
#include "filecache.h"
#include "gearlistwidget.h"

MainWindow::MainWindow(GameData* in_data) : data(*in_data), cache(FileCache{*in_data}) {
    setWindowTitle(QStringLiteral("Armoury Editor"));
    setMinimumSize(QSize(800, 600));

    auto fileMenu = menuBar()->addMenu(QStringLiteral("File"));

    auto quitAction = fileMenu->addAction(QStringLiteral("Quit"));
    quitAction->setIcon(QIcon::fromTheme(QStringLiteral("gtk-quit")));
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    auto toolsMenu = menuBar()->addMenu(QStringLiteral("Tools"));

    auto cmpEditorMenu = toolsMenu->addAction(QStringLiteral("CMP Editor"));
    cmpEditorMenu->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    connect(cmpEditorMenu, &QAction::triggered, [=] {
        auto cmpEditor = new CmpEditor(in_data);
        cmpEditor->show();
    });

    auto helpMenu = menuBar()->addMenu(QStringLiteral("Help"));

    auto donateAction = helpMenu->addAction(QStringLiteral("Donate"));
    connect(donateAction, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://redstrate.com/fund")));
    });
    donateAction->setIcon(QIcon::fromTheme(QStringLiteral("help-donate")));

    helpMenu->addSeparator();

    auto aboutNovusAction = helpMenu->addAction(QStringLiteral("About Armoury Editor"));
    aboutNovusAction->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    connect(aboutNovusAction, &QAction::triggered, this, [this] {
        auto window = new KAboutApplicationDialog(KAboutData::applicationData(), this);
        window->show();
    });

    auto aboutQtAction = helpMenu->addAction(QStringLiteral("About Qt"));
    aboutQtAction->setIcon(QIcon(QStringLiteral(":/qt-project.org/qmessagebox/images/qtlogo-64.png")));
    connect(aboutQtAction, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto gearListWidget = new GearListWidget(&data);
    gearListWidget->setMaximumWidth(350);
    connect(gearListWidget, &GearListWidget::gearSelected, this, [=](const GearInfo& gear) {
        gearView->setGear(gear);
    });
    layout->addWidget(gearListWidget);

    gearView = new SingleGearView(&data, cache);
    connect(gearView, &SingleGearView::addToFullModelViewer, this, [=](GearInfo& info) {
        fullModelViewer->addGear(info);
    });

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(gearView, QStringLiteral("Models"));
    layout->addWidget(tabWidget);

    fullModelViewer = new FullModelViewer(&data, cache);
    connect(fullModelViewer, &FullModelViewer::loadingChanged, this, [=](const bool loading) {
        gearView->setFMVAvailable(!loading);
    });
    fullModelViewer->show();
}