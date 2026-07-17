// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scenepart.h"

#include "filecache.h"
#include "objectpropertieswidget.h"
#include "scenelistwidget.h"
#include "scenestate.h"

#include <QHBoxLayout>
#include <QSplitter>

#include "mapview.h"
#include "settings.h"

#include <QDir>

ScenePart::ScenePart(FileCache &cache, const bool fixedSize, QWidget *parent)
    : QWidget(parent)
    , m_cache(cache)
    , m_appState(new SceneState(m_cache, this))
{
    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    const auto splitter = new QSplitter();
    splitter->setChildrenCollapsible(false);
    layout->addWidget(splitter);

    const auto sidebarWidget = new QWidget();
    splitter->addWidget(sidebarWidget);

    const auto sidebarLayout = new QVBoxLayout();
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);
    sidebarWidget->setLayout(sidebarLayout);

    m_sceneListWidget = new SceneListWidget(m_appState);
    m_sceneListWidget->setMaximumWidth(400);
    if (fixedSize)
        m_sceneListWidget->setMinimumWidth(400);
    sidebarLayout->addWidget(m_sceneListWidget);

    m_animationTimeSlider = new QSlider(Qt::Orientation::Horizontal);
    m_animationTimeSlider->setMaximumWidth(400);
    m_animationTimeSlider->setEnabled(false);
    connect(m_animationTimeSlider, &QSlider::valueChanged, this, [this](const int value) {
        m_appState->updateAllAnimations(value);
        Q_EMIT m_appState->mapLoaded(); // FIXME: extreme solution to lack of a proper updatable scene graph
    });
    sidebarLayout->addWidget(m_animationTimeSlider);

    m_timeSlider = new QSlider(Qt::Orientation::Horizontal);
    m_timeSlider->setMaximumWidth(400);
    m_timeSlider->setMinimum(0);
    m_timeSlider->setMaximum(86400);
    connect(m_timeSlider, &QSlider::valueChanged, this, [this](const int value) {
        m_mapView->part().manager()->scene.time = value;
        Q_EMIT m_mapView->part().cameraMoved(); // FIXME: workaround because that's when lighting recalculations happen
    });
    sidebarLayout->addWidget(m_timeSlider);

    m_mapView = new MapView(m_cache, m_appState);
    splitter->addWidget(m_mapView);
    splitter->setStretchFactor(1, 1);

    m_objectPropertiesWidget = new ObjectPropertiesWidget(m_appState);
    m_objectPropertiesWidget->setMaximumWidth(400);
    if (fixedSize)
        m_objectPropertiesWidget->setMinimumWidth(400);
    splitter->addWidget(m_objectPropertiesWidget);

    connect(
        m_appState,
        &SceneState::mapLoaded,
        this,
        [this] {
            // Update animation slider maximum
            m_animationTimeSlider->setMaximum(m_appState->longestAnimationTime());
            m_animationTimeSlider->setEnabled(true);
        },
        Qt::SingleShotConnection);
    connect(m_appState, &SceneState::selectObject, this, &ScenePart::selectObject);
}

ScenePart::~ScenePart()
{
    physis_lvb_free(&m_lvb);
    physis_sgb_free(&m_sgb);
}

void ScenePart::loadSgb(const physis_Buffer file)
{
    physis_sgb_free(&m_sgb); // free any previous ones
    m_sgb = physis_sgb_parse(m_cache.platform(), file);
    if (m_sgb.sections) {
        // TODO: load more than one section?
        m_appState->load(m_cache, m_sgb.sections[0]);

        // Expand the first level of the SGB
        m_sceneListWidget->expandToDepth(1);

        // Show everything by default
        m_appState->showAll();
    } else {
        qWarning() << "Failed to parse SGB!";
    }
}

bool ScenePart::loadLvb(const physis_Buffer file, const int territoryTypeHint, const int contentFinderConditionHint)
{
    physis_lvb_free(&m_lvb); // free any previous ones
    m_lvb = physis_lvb_parse(m_cache.platform(), file);
    if (m_lvb.sections) {
        // TODO: read all sections?
        m_appState->load(m_cache, m_lvb.sections[0], territoryTypeHint, contentFinderConditionHint);

        return true;
    }

    return false;
}

void ScenePart::focusSearchField() const
{
    m_sceneListWidget->focusSearchField();
}

void ScenePart::selectObject(const uint32_t objectId) const
{
    m_sceneListWidget->selectObject(objectId);
}

QString ScenePart::lookupObjectName(const uint32_t objectId) const
{
    return m_sceneListWidget->lookupObjectName(objectId);
}

void ScenePart::clear() const
{
    m_appState->clear();
    m_mapView->clear();
}

void ScenePart::save() const
{
    const auto mods = getGameMods();
    if (mods.isEmpty()) {
        qWarning() << "No mod to write a file to!";
        return;
    }

    auto &scene = m_appState->rootScene;
    for (const auto &[path, lgb] : scene.lgbFiles) {
        qInfo() << "Saving LGB" << path;

        const QDir targetDir = mods.constFirst().path;
        const QString newPath = targetDir.absoluteFilePath(path);

        QFileInfo info(newPath);
        QDir().mkpath(info.dir().path()); // Create directory tree within the mod.

        const auto buffer = physis_lgb_write_to_buffer(m_cache.platform(), lgb);
        QFile file(newPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reinterpret_cast<const char *>(buffer.data), buffer.size);
        } else {
            qWarning() << "Failed to write LGB" << newPath << "because of:" << file.errorString();
        }
    }
}

SceneState *ScenePart::sceneState() const
{
    return m_appState;
}

MapView *ScenePart::mapView() const
{
    return m_mapView;
}

#include "moc_scenepart.cpp"
