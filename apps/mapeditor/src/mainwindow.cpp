// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <QApplication>
#include <QDesktopServices>
#include <QMenuBar>
#include <glm/gtc/type_ptr.hpp>
#include <physis.hpp>

#include "maplistwidget.h"
#include "mapview.h"
#include "scenepart.h"
#include "scenestate.h"

#include <QLabel>
#include <QStatusBar>

MainWindow::MainWindow(physis_SqPackResource data)
    : m_data(data)
    , cache(m_data)
{
    setMinimumSize(1280, 720);

    m_part = new ScenePart(&m_data, true);
    setCentralWidget(m_part);

    setupActions();
    setupGUI(Keys | Save | Create, QStringLiteral("mapeditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));

    connect(m_part->sceneState(), &SceneState::selectionChanged, this, &MainWindow::updateActionState);

    updateActionState();
}

void MainWindow::setupActions()
{
    KStandardAction::open(
        qApp,
        [this] {
            auto listWidget = new MapListWidget(&m_data, this);
            connect(listWidget, &MapListWidget::accepted, this, [this, listWidget] {
                openMap(listWidget->acceptedMap(), listWidget->acceptedContentFinderCondition());
            });
            listWidget->show();
        },
        actionCollection());

    m_centerObjectAction = new QAction(i18nc("@action:inmenu", "Center on Object"));
    m_centerObjectAction->setIcon(QIcon::fromTheme(QStringLiteral("camera-video-symbolic")));
    KActionCollection::setDefaultShortcut(m_centerObjectAction, QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_C));
    connect(m_centerObjectAction, &QAction::triggered, [this] {
        m_part->mapView()->centerOn(glm::make_vec3(m_part->sceneState()->selectedObject.value()->transform.translation));
    });
    actionCollection()->addAction(QStringLiteral("center_object"), m_centerObjectAction);

    auto focusSearch = new QAction(i18nc("@action:inmenu", "Search"));
    focusSearch->setIcon(QIcon::fromTheme(QStringLiteral("search-symbolic")));
    KActionCollection::setDefaultShortcut(focusSearch, QKeySequence(Qt::CTRL | Qt::Key_F));
    connect(focusSearch, &QAction::triggered, m_part, &ScenePart::focusSearchField);
    actionCollection()->addAction(QStringLiteral("search"), focusSearch);

    m_goToEntranceAction = new QAction(i18nc("@action:inmenu", "Go to Entrance"));
    m_goToEntranceAction->setEnabled(false);
    connect(m_goToEntranceAction, &QAction::triggered, m_part, [this] {
        auto entranceTransform = m_part->sceneState()->rootScene.locateGameObject(m_lgbEventRange);
        m_part->mapView()->centerOn(glm::make_vec3(entranceTransform.translation));
    });
    actionCollection()->addAction(QStringLiteral("duty_go_to_entrance"), m_goToEntranceAction);

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());

    m_cameraPosLabel = new QLabel(i18n("..."));
    statusBar()->addWidget(m_cameraPosLabel);
    connect(&m_part->mapView()->part(), &MDLPart::cameraMoved, this, [this] {
        m_cameraPosLabel->setText(i18n("X: %1 Y: %2 Z: %3")
                                      .arg(m_part->mapView()->part().position.x)
                                      .arg(m_part->mapView()->part().position.y)
                                      .arg(m_part->mapView()->part().position.z));
    });
}

void MainWindow::openMap(const QString &basePath, int contentFinderCondition)
{
    m_part->sceneState()->clear();

    QString lvbPath = QStringLiteral("bg/%1.lvb").arg(basePath);
    std::string lvbPathStd = lvbPath.toStdString();

    auto lvbFile = physis_sqpack_read(&m_data, lvbPathStd.c_str());
    if (lvbFile.size > 0) {
        auto lvb = physis_lvb_parse(m_data.platform, lvbFile);
        if (lvb.sections) {
            // TODO: read all sections?
            m_part->sceneState()->load(&m_data, lvb.sections[0]);
        } else {
            qWarning() << "Failed to parse lvb" << lvbPath;
        }
    } else {
        qWarning() << "Failed to find lvb" << lvbPath;
    }

    setWindowTitle(basePath);

    m_goToEntranceAction->setEnabled(contentFinderCondition != 0);

    if (contentFinderCondition != 0) {
        qInfo() << "This map contains a duty! CF:" << contentFinderCondition;

        auto cfcExh = physis_exh_parse(m_data.platform, physis_sqpack_read(&m_data, "exd/ContentFinderCondition.exh"));
        auto cfcSheet = physis_sqpack_read_excel_sheet(&m_data, "ContentFinderCondition", &cfcExh, Language::English);

        auto cfcRow = physis_excel_get_row(&cfcSheet, contentFinderCondition);
        auto instanceContentId = cfcRow.columns[3].u_int16._0;

        auto instanceContentExh = physis_exh_parse(m_data.platform, physis_sqpack_read(&m_data, "exd/InstanceContent.exh"));
        auto instanceContentSheet = physis_sqpack_read_excel_sheet(&m_data, "InstanceContent", &instanceContentExh, Language::None);

        auto instanceContentRow = physis_excel_get_row(&instanceContentSheet, instanceContentId);

        m_lgbEventRange = instanceContentRow.columns[7].u_int32._0;
    }
}

void MainWindow::updateActionState()
{
    m_centerObjectAction->setEnabled(m_part->sceneState()->selectedObject.has_value());
}

#include "moc_mainwindow.cpp"
