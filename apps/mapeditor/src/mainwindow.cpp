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

#include "appstate.h"
#include "maplistwidget.h"
#include "mapview.h"
#include "objectlistwidget.h"

MainWindow::MainWindow(GameData *data)
    : KXmlGuiWindow()
    , data(data)
    , cache(*data)
{
    setMinimumSize(1280, 720);

    m_appState = new AppState(this);

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    objectListWidget = new ObjectListWidget(m_appState);
    objectListWidget->setMaximumWidth(400);
    dummyWidget->addWidget(objectListWidget);

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

    const auto loadLgb = [this, base2Path](const QString &name) {
        QString lgbPath = QStringLiteral("bg/%1/level/%2.lgb").arg(base2Path, name);
        std::string bgLgbPathStd = lgbPath.toStdString();

        auto bg_buffer = physis_gamedata_extract_file(data, bgLgbPathStd.c_str());
        auto lgb = physis_layergroup_read(bg_buffer);
        if (lgb.num_chunks > 0) {
            m_appState->lgbFiles.push_back({name, lgb});
        }
    };

    loadLgb(QStringLiteral("planevent"));
    loadLgb(QStringLiteral("vfx"));
    loadLgb(QStringLiteral("planmap"));
    loadLgb(QStringLiteral("planner"));
    loadLgb(QStringLiteral("bg"));
    loadLgb(QStringLiteral("sound"));
    loadLgb(QStringLiteral("planlive"));

    Q_EMIT m_appState->mapLoaded();
}

#include "moc_mainwindow.cpp"
