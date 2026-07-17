// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "effectlistwidget.h"
#include "gimmicklistwidget.h"
#include "settingswindow.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <QApplication>
#include <QDesktopServices>
#include <glm/gtc/type_ptr.hpp>
#include <physis.hpp>

#include "maplistwidget.h"
#include "mapview.h"
#include "openinwidget.h"
#include "scenepart.h"
#include "scenestate.h"
#include "settings.h"

#include <KConfig>
#include <KConfigGroup>
#include <QDirIterator>
#include <QInputDialog>
#include <QLabel>
#include <QStatusBar>

MainWindow::MainWindow(const physis_SqPackResource data)
    : m_cache(data)
{
    setMinimumSize(1280, 720);

    m_part = new ScenePart(m_cache, true);
    setCentralWidget(m_part);

    setupActions();
    setupGUI(Default, QStringLiteral("mapeditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));

    connect(m_part->sceneState(), &SceneState::selectionChanged, this, &MainWindow::updateActionState);
    connect(m_part->sceneState(), &SceneState::mapLoaded, this, &MainWindow::updateActionState);

    updateActionState();

    const auto openInWidget = new OpenInWidget(this);
    menuBar()->setCornerWidget(openInWidget);
}

MainWindow::~MainWindow() = default;

void MainWindow::configure()
{
    const auto settingsWindow = new SettingsWindow();
    settingsWindow->show();
}

void MainWindow::setupActions()
{
    KStandardAction::preferences(this, &MainWindow::configure, actionCollection());
    KStandardAction::open(
        qApp,
        [this] {
            auto listWidget = new MapListWidget(m_cache, this);
            connect(listWidget, &MapListWidget::accepted, this, [this, listWidget] {
                openMap(listWidget->acceptedMap(), listWidget->acceptedTerritoryType(), listWidget->acceptedContentFinderCondition());
            });
            listWidget->show();
        },
        actionCollection());
    m_saveAction = KStandardAction::save(
        qApp,
        [this] {
            m_part->sceneState()->saveDropIns();
            m_part->save();
        },
        actionCollection());
    m_closeAction = KStandardAction::close(
        qApp,
        [this] {
            setPlainCaption({});
            m_part->clear();
            Q_EMIT m_part->sceneState()->mapLoaded();
        },
        actionCollection());

    m_centerObjectAction = new QAction(i18nc("@action:inmenu", "Center on Object"), this);
    m_centerObjectAction->setIcon(QIcon::fromTheme(QStringLiteral("camera-video-symbolic")));
    KActionCollection::setDefaultShortcut(m_centerObjectAction, QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_C));
    connect(m_centerObjectAction, &QAction::triggered, [this] {
        if (const auto selectedObject = m_part->sceneState()->selectedObject) {
            m_part->mapView()->centerOn(glm::make_vec3(selectedObject.value()->transform.translation));
        }
        if (const auto selectedDropInObject = m_part->sceneState()->selectedDropInObject) {
            m_part->mapView()->centerOn(glm::make_vec3(selectedDropInObject.value()->position));
        }
    });
    actionCollection()->addAction(QStringLiteral("center_object"), m_centerObjectAction);

    const auto focusSearch = new QAction(i18nc("@action:inmenu", "Search"), this);
    focusSearch->setIcon(QIcon::fromTheme(QStringLiteral("search-symbolic")));
    KActionCollection::setDefaultShortcut(focusSearch, QKeySequence(Qt::CTRL | Qt::Key_F));
    connect(focusSearch, &QAction::triggered, m_part, &ScenePart::focusSearchField);
    actionCollection()->addAction(QStringLiteral("search"), focusSearch);

    m_goToEntranceAction = new QAction(i18nc("@action:inmenu", "Go to Entrance"), this);
    m_goToEntranceAction->setEnabled(false);
    connect(m_goToEntranceAction, &QAction::triggered, this, [this] {
        const auto entranceTransform = m_part->sceneState()->rootScene.locateGameObject(m_lgbEventRange);
        m_part->mapView()->centerOn(glm::make_vec3(entranceTransform.translation));
    });
    actionCollection()->addAction(QStringLiteral("duty_go_to_entrance"), m_goToEntranceAction);

    m_goToExitAction = new QAction(i18nc("@action:inmenu", "Go to Exit"), this);
    m_goToExitAction->setEnabled(false);
    connect(m_goToExitAction, &QAction::triggered, this, [this] {
        // TODO: what if there are multiple exits, is that a thing?

        const auto exitTransform = m_part->sceneState()->rootScene.locateGameObjectByBaseId(2000139); // TODO: extract into a constant
        m_part->mapView()->centerOn(glm::make_vec3(exitTransform.translation));
    });
    actionCollection()->addAction(QStringLiteral("duty_go_to_exit"), m_goToExitAction);

    m_gimmickListAction = new QAction(i18nc("@action:inmenu", "Gimmicks"), this);
    m_gimmickListAction->setEnabled(false);
    connect(m_gimmickListAction, &QAction::triggered, this, [this] {
        // TODO: only pass m_part I guess
        const auto listWidget = new GimmickListWidget(m_part, m_part->sceneState(), this);
        listWidget->show();
    });
    actionCollection()->addAction(QStringLiteral("duty_gimmicks"), m_gimmickListAction);

    m_effectListAction = new QAction(i18nc("@action:inmenu", "Effects"), this);
    m_effectListAction->setEnabled(false);
    connect(m_effectListAction, &QAction::triggered, this, [this] {
        const auto listWidget = new EffectListWidget(m_part->sceneState(), m_mapEffects, this);
        listWidget->show();
    });
    actionCollection()->addAction(QStringLiteral("duty_effects"), m_effectListAction);

    m_goToObjectAction = new QAction(i18nc("@action:inmenu", "To Object…"), this);
    m_goToObjectAction->setIcon(QIcon::fromTheme(QStringLiteral("go-jump-symbolic")));
    KActionCollection::setDefaultShortcut(m_goToObjectAction, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_G));
    connect(m_goToObjectAction, &QAction::triggered, this, [this] {
        bool ok = false;
        const QString text = QInputDialog::getText(this, i18n("Go To…"), i18n("Object ID:"), QLineEdit::Normal, QString{}, &ok);
        if (ok && !text.isEmpty()) {
            m_part->selectObject(text.toUInt());
        }
    });
    actionCollection()->addAction(QStringLiteral("goto_object"), m_goToObjectAction);

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());

    m_cameraPosLabel = new QLabel(i18n("..."));
    statusBar()->addWidget(m_cameraPosLabel);
    connect(&m_part->mapView()->part(), &MDLPart::cameraMoved, this, [this] {
        m_cameraPosLabel->setText(i18n("X: %1 Y: %2 Z: %3 Culled Objects: %5 Culled Lights: %6")
                                      .arg(m_part->mapView()->part().position.x)
                                      .arg(m_part->mapView()->part().position.y)
                                      .arg(m_part->mapView()->part().position.z)
                                      .arg(m_part->mapView()->part().manager()->scene.culledObjects)
                                      .arg(m_part->mapView()->part().manager()->scene.culledLights));
    });

    actionCollection()->addAction(QStringLiteral("wireframe"), m_part->mapView()->part().wireframeAction());
    actionCollection()->addAction(QStringLiteral("frustum_culling"), m_part->mapView()->part().frustumCullingAction());
    actionCollection()->addAction(QStringLiteral("debug_frustum_culling"), m_part->mapView()->part().debugFrustumCullingAction());
}

void MainWindow::openMap(const QString &basePath, const int territoryType, const int contentFinderCondition)
{
    m_part->clear();
    m_mapEffects.clear();
    m_lgbEventRange = 0;

    const QString lvbPath = QStringLiteral("bg/%1.lvb").arg(basePath);
    const auto lvbFile = m_cache.read(lvbPath);
    if (lvbFile.size > 0) {
        if (!m_part->loadLvb(lvbFile, territoryType, contentFinderCondition)) {
            qWarning() << "Failed to parse LVB:" << lvbPath;
        }

        KConfig config(QStringLiteral("novusrc"));
        const KConfigGroup game = config.group(QStringLiteral("MapEditor"));

        const auto dropInsPath = game.readEntry("DropInsPath");
        if (!dropInsPath.isEmpty()) {
            QDirIterator it(dropInsPath);
            while (it.hasNext()) {
                m_part->sceneState()->loadDropIn(it.next());
            }
        }

        Q_EMIT m_part->sceneState()->mapLoaded();
    } else {
        qWarning() << "Failed to find LVB" << lvbPath;
    }

    setPlainCaption(basePath);

    m_goToEntranceAction->setEnabled(contentFinderCondition != 0);
    m_goToExitAction->setEnabled(contentFinderCondition != 0);
    m_gimmickListAction->setEnabled(contentFinderCondition != 0);
    m_effectListAction->setEnabled(contentFinderCondition != 0);

    if (contentFinderCondition != 0) {
        qInfo() << "This map contains a duty! CF:" << contentFinderCondition;

        const auto buffer = m_cache.read(QStringLiteral("exd/ContentFinderCondition.exh"));
        if (buffer.size > 0) {
            const auto cfcExh = physis_exh_parse(m_cache.platform(), buffer);
            if (cfcExh.p_ptr) {
                const auto cfcSheet = m_cache.readExcelSheet(QStringLiteral("ContentFinderCondition"), &cfcExh, getLanguage());

                const auto cfcRow = physis_excel_get_row(&cfcSheet, contentFinderCondition);
                const auto instanceContentId = cfcRow.columns[3].u_int16._0;

                const auto instanceContentExh = physis_exh_parse(m_cache.platform(), m_cache.read(QStringLiteral("exd/InstanceContent.exh")));
                const auto instanceContentSheet = m_cache.readExcelSheet(QStringLiteral("InstanceContent"), &instanceContentExh, Language::None);

                const auto instanceContentRow = physis_excel_get_row(&instanceContentSheet, instanceContentId);

                m_lgbEventRange = instanceContentRow.columns[7].u_int32._0;

                const auto mapEffectId = instanceContentRow.columns[64].u_int16._0;

                const auto mapEffectExh = physis_exh_parse(m_cache.platform(), m_cache.read(QStringLiteral("exd/ContentDirectorManagedSG.exh")));
                const auto mapEffectSheet = m_cache.readExcelSheet(QStringLiteral("ContentDirectorManagedSG"), &mapEffectExh, Language::None);

                const auto effectCount = physis_excel_get_subrow_count(&mapEffectSheet, mapEffectId);
                for (size_t i = 0; i < effectCount; i++) {
                    const auto effectRow = physis_excel_get_subrow(&mapEffectSheet, mapEffectId, i);
                    if (effectRow.columns) {
                        m_mapEffects.push_back(effectRow.columns[0].int32._0);
                    }
                }
            }
        }
    }
}

void MainWindow::updateActionState() const
{
    m_centerObjectAction->setEnabled(m_part->sceneState()->selectedObject.has_value() || m_part->sceneState()->selectedDropInObject.has_value());
    m_saveAction->setEnabled(!m_part->sceneState()->rootScene.lgbFiles.empty());
    m_closeAction->setEnabled(!m_part->sceneState()->rootScene.lgbFiles.empty());
    m_goToObjectAction->setEnabled(!m_part->sceneState()->rootScene.lgbFiles.empty());
}

#include "moc_mainwindow.cpp"
