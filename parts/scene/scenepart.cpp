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

ScenePart::ScenePart(physis_SqPackResource *data, bool fixedSize, QWidget *parent)
    : QWidget(parent)
    , m_data(data)
    , m_fileCache(*data) // TODO: re-use FileCache
    , m_appState(new SceneState(m_fileCache, this))
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    auto splitter = new QSplitter();
    splitter->setChildrenCollapsible(false);
    layout->addWidget(splitter);

    auto sidebarWidget = new QWidget();
    splitter->addWidget(sidebarWidget);

    auto sidebarLayout = new QVBoxLayout();
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

    m_mapView = new MapView(data, m_fileCache, m_appState);
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

void ScenePart::loadSgb(physis_Buffer file)
{
    physis_sgb_free(&m_sgb); // free any previous ones
    m_sgb = physis_sgb_parse(m_data->platform, file);
    if (m_sgb.sections) {
        // TODO: load more than one section?
        m_appState->load(m_fileCache, m_sgb.sections[0]);

        // Expand the first level of the SGB
        m_sceneListWidget->expandToDepth(1);

        // Show everything by default
        m_appState->showAll();
    } else {
        qWarning() << "Failed to parse SGB!";
    }
}

void ScenePart::loadLvb(physis_Buffer file)
{
    physis_lvb_free(&m_lvb); // free any previous ones
    m_lvb = physis_lvb_parse(m_data->platform, file);
    if (m_lvb.sections) {
        // TODO: read all sections?
        m_appState->load(m_fileCache, m_lvb.sections[0]);
    } else {
        qWarning() << "Failed to parse lvb";
    }
}

void ScenePart::focusSearchField()
{
    m_sceneListWidget->focusSearchField();
}

void ScenePart::selectObject(const uint32_t objectId)
{
    m_sceneListWidget->selectObject(objectId);
}

QString ScenePart::lookupObjectName(const uint32_t objectId)
{
    return m_sceneListWidget->lookupObjectName(objectId);
}

void ScenePart::clear()
{
    m_appState->clear();
    m_mapView->clear();
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
