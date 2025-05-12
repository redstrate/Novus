// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KActionCollection>
#include <QApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QSplitter>
#include <physis.hpp>

#include "maplistwidget.h"
#include "mapview.h"

MainWindow::MainWindow(GameData *data)
    : KXmlGuiWindow()
    , data(data)
    , cache(*data)
{
    setMinimumSize(1280, 720);

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    mapView = new MapView(data, cache);
    dummyWidget->addWidget(mapView);

    setupActions();
    setupGUI(Keys | Save | Create);

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));
}

void MainWindow::setupActions()
{
    KStandardAction::open(
        qApp,
        [this] {
            auto dialog = new QDialog();

            auto layout = new QVBoxLayout();
            layout->setContentsMargins({});
            dialog->setLayout(layout);

            auto listWidget = new MapListWidget(data);
            connect(listWidget, &MapListWidget::mapSelected, this, [this, dialog](const QString &basePath) {
                dialog->close();
                openMap(basePath);
            });
            layout->addWidget(listWidget);

            dialog->exec();
        },
        actionCollection());

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}

void MainWindow::openMap(const QString &basePath)
{
    QString base2Path = basePath.left(basePath.lastIndexOf(QStringLiteral("/level/")));
    QString bgPath = QStringLiteral("bg/%1/bgplate/").arg(base2Path);

    std::string bgPathStd = bgPath.toStdString() + "terrain.tera";

    auto tera_buffer = physis_gamedata_extract_file(data, bgPathStd.c_str());

    auto tera = physis_parse_tera(tera_buffer);
    mapView->addTerrain(bgPath, tera);

    setWindowTitle(basePath);
}

#include "moc_mainwindow.cpp"
