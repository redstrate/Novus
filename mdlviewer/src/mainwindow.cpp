#include "mainwindow.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QTimer>

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QPushButton>
#include <QTreeWidget>
#include <glm/gtc/type_ptr.hpp>
#include <magic_enum.hpp>
#include <physis.hpp>
#include <QApplication>
#include <QDesktopServices>

#include "aboutwindow.h"
#include "cmpeditor.h"
#include "gearlistwidget.h"

MainWindow::MainWindow(GameData* in_data) : data(*in_data) {
    setWindowTitle("mdlviewer");
    setMinimumSize(QSize(800, 600));

    auto fileMenu = menuBar()->addMenu("File");

    // TODO: move to a dedicated mdlview?
    /*auto openMDLFile = fileMenu->addAction("Open MDL...");
    connect(openMDLFile, &QAction::triggered, [=] {
        auto fileName = QFileDialog::getOpenFileName(nullptr,
                                                     "Open MDL File",
                                                     "~",
                                                     "FFXIV Model File (*.mdl)");

        auto buffer = physis_read_file(fileName.toStdString().c_str());

        loadedGear.model = physis_mdl_parse(buffer.size, buffer.data);

        reloadGearAppearance();
    });*/

    auto toolsMenu = menuBar()->addMenu("Tools");

    auto cmpEditorMenu = toolsMenu->addAction("CMP Editor");
    connect(cmpEditorMenu, &QAction::triggered, [=] {
        auto cmpEditor = new CmpEditor(in_data);
        cmpEditor->show();
    });

    auto helpMenu = menuBar()->addMenu("Help");

    auto donateAction = helpMenu->addAction("Donate");
    connect(donateAction, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl("https://redstrate.com/fund"));
    });
    donateAction->setIcon(QIcon::fromTheme("help-donate"));

    helpMenu->addSeparator();

    auto aboutNovusAction = helpMenu->addAction("About mdlviewer");
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

    auto gearListWidget = new GearListWidget(&data);
    gearListWidget->setMaximumWidth(350);
    connect(gearListWidget, &GearListWidget::gearSelected, this, [=](const GearInfo& gear) {
        gearView->setGear(gear);
    });
    layout->addWidget(gearListWidget);

    gearView = new SingleGearView(&data);
    connect(gearView, &SingleGearView::addToFullModelViewer, this, [=](GearInfo& info) {
        fullModelViewer->addGear(info);
    });
    layout->addWidget(gearView);

    fullModelViewer = new FullModelViewer(&data);
    fullModelViewer->show();
}