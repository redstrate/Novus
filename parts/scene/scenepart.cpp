// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scenepart.h"

#include "filecache.h"
#include "objectpropertieswidget.h"
#include "scenelistwidget.h"
#include "scenestate.h"

#include <QHBoxLayout>

#include "animation.h"
#include "mapview.h"

ScenePart::ScenePart(physis_SqPackResource *data, QWidget *parent)
    : QWidget(parent)
    , m_appState(new SceneState(data))
    , m_data(data)
    , m_fileCache(*data) // TODO: re-use FileCache
{
    auto layout = new QHBoxLayout();
    setLayout(layout);

    auto sidebarLayout = new QVBoxLayout();
    layout->addLayout(sidebarLayout);

    m_sceneListWidget = new SceneListWidget(m_appState);
    m_sceneListWidget->setMaximumWidth(400);
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
    layout->addWidget(m_mapView);

    m_objectPropertiesWidget = new ObjectPropertiesWidget(m_appState);
    m_objectPropertiesWidget->setMaximumWidth(400);
    layout->addWidget(m_objectPropertiesWidget);
}

void ScenePart::loadSgb(physis_Buffer file)
{
    auto sgb = physis_sgb_parse(m_data->platform, file);
    if (sgb.sections) {
        // TODO: load more than one section?
        m_appState->load(m_data, sgb.sections[0]);

        // Update animation slider maximum
        m_animationTimeSlider->setMaximum(m_appState->longestAnimationTime());
        m_animationTimeSlider->setEnabled(true);

        qInfo() << "Longest animation time:" << m_appState->longestAnimationTime();

        // Expand the first level of the SGB
        m_sceneListWidget->expandToDepth(1);

        // Show everything by default
        m_appState->showAll();
    } else {
        qWarning() << "Failed to parse SGB!";
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
