// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <QApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QSplitter>
#include <glm/gtc/type_ptr.hpp>
#include <physis.hpp>

#include "appstate.h"
#include "maplistwidget.h"
#include "mapview.h"
#include "objectlistwidget.h"
#include "objectpropertieswidget.h"

MainWindow::MainWindow(physis_SqPackResource data)
    : m_data(data)
    , cache(m_data)
{
    setMinimumSize(1280, 720);

    m_appState = new AppState(&m_data, this);

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    objectListWidget = new ObjectListWidget(m_appState);
    objectListWidget->setMaximumWidth(400);
    dummyWidget->addWidget(objectListWidget);

    mapView = new MapView(&m_data, cache, m_appState);
    dummyWidget->addWidget(mapView);

    objectPropertiesWidget = new ObjectPropertiesWidget(m_appState);
    objectPropertiesWidget->setMaximumWidth(400);
    dummyWidget->addWidget(objectPropertiesWidget);

    setupActions();
    setupGUI(Keys | Save | Create, QStringLiteral("mapeditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));

    connect(m_appState, &AppState::selectionChanged, this, &MainWindow::updateActionState);

    updateActionState();
}

void MainWindow::setupActions()
{
    KStandardAction::open(
        qApp,
        [this] {
            auto listWidget = new MapListWidget(&m_data, this);
            connect(listWidget, &MapListWidget::accepted, this, [this, listWidget] {
                openMap(listWidget->acceptedMap());
            });
            listWidget->show();
        },
        actionCollection());

    m_centerObjectAction = new QAction(i18nc("@action:inmenu", "Center on Object"));
    m_centerObjectAction->setIcon(QIcon::fromTheme(QStringLiteral("camera-video-symbolic")));
    KActionCollection::setDefaultShortcut(m_centerObjectAction, QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_C));
    connect(m_centerObjectAction, &QAction::triggered, [this] {
        mapView->centerOn(glm::make_vec3(m_appState->selectedObject.value()->transform.translation));
    });
    actionCollection()->addAction(QStringLiteral("center_object"), m_centerObjectAction);

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}

void MainWindow::openMap(const QString &basePath)
{
    QString base2Path = basePath.left(basePath.lastIndexOf(QStringLiteral("/level/")));

    m_appState->basePath = basePath;

    setWindowTitle(basePath);

    QString bgPath = QStringLiteral("bg/%1/bgplate/").arg(base2Path);

    std::string bgPathStd = bgPath.toStdString() + "terrain.tera";

    auto tera_buffer = physis_sqpack_read(&m_data, bgPathStd.c_str());
    if (tera_buffer.size > 0) {
        m_appState->terrain = physis_terrain_parse(m_data.platform, tera_buffer);
    } else {
        qWarning() << "Failed to load terrain" << bgPathStd;
    }

    const auto loadLgb = [this, base2Path](const QString &name) {
        QString lgbPath = QStringLiteral("bg/%1/level/%2.lgb").arg(base2Path, name);
        std::string bgLgbPathStd = lgbPath.toStdString();

        auto bg_buffer = physis_sqpack_read(&m_data, bgLgbPathStd.c_str());
        if (bg_buffer.size > 0) {
            auto lgb = physis_layergroup_parse(m_data.platform, bg_buffer);
            if (lgb.num_chunks > 0) {
                m_appState->lgbFiles.push_back({name, lgb});
            }
        }
    };

    loadLgb(QStringLiteral("planevent"));
    loadLgb(QStringLiteral("vfx"));
    loadLgb(QStringLiteral("planmap"));
    loadLgb(QStringLiteral("planner"));
    loadLgb(QStringLiteral("bg"));
    loadLgb(QStringLiteral("sound"));
    loadLgb(QStringLiteral("planlive"));

    // Load terrain and bg by default
    for (int i = 0; i < m_appState->terrain.num_plates; i++) {
        m_appState->visibleTerrainPlates.push_back(i);
    }

    for (const auto &[name, lgb] : m_appState->lgbFiles) {
        if (name == QStringLiteral("bg")) {
            for (uint32_t i = 0; i < lgb.num_chunks; i++) {
                for (uint32_t j = 0; j < lgb.chunks[i].num_layers; j++) {
                    // Skip festival-specific layers
                    if (lgb.chunks[i].layers[j].festival_id == 0) {
                        m_appState->visibleLayerIds.push_back(lgb.chunks[i].layers[j].id);
                    }
                }
            }
            break;
        }
    }

    Q_EMIT m_appState->mapLoaded();
}

void MainWindow::updateActionState()
{
    m_centerObjectAction->setEnabled(m_appState->selectedObject.has_value());
}

#include "moc_mainwindow.cpp"
