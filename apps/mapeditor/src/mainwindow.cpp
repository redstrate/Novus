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

MainWindow::MainWindow(physis_SqPackResource data)
    : m_data(data)
    , cache(m_data)
{
    setMinimumSize(1280, 720);

    m_part = new ScenePart(&m_data);
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
                openMap(listWidget->acceptedMap());
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

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}

void MainWindow::openMap(const QString &basePath)
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
}

void MainWindow::updateActionState()
{
    m_centerObjectAction->setEnabled(m_part->sceneState()->selectedObject.has_value());
}

#include "moc_mainwindow.cpp"
