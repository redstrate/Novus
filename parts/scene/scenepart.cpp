// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scenepart.h"

#include "filecache.h"
#include "objectpropertieswidget.h"
#include "scenelistwidget.h"
#include "scenestate.h"

#include <QHBoxLayout>

ScenePart::ScenePart(physis_SqPackResource *data, QWidget *parent)
    : QWidget(parent)
    , m_appState(new SceneState(data))
    , m_data(data)
{
    auto layout = new QHBoxLayout();
    setLayout(layout);

    m_sceneListWidget = new SceneListWidget(m_appState);
    layout->addWidget(m_sceneListWidget);

    m_objectPropertiesWidget = new ObjectPropertiesWidget(m_appState);
    layout->addWidget(m_objectPropertiesWidget);
}

void ScenePart::loadSgb(physis_Buffer file)
{
    auto sgb = physis_sgb_parse(m_data->platform, file);
    if (sgb.sections) {
        // TODO: load more than one section?
        m_appState->load(m_data, sgb.sections[0]);
    } else {
        qWarning() << "Failed to parse SGB!";
    }
}

#include "moc_scenepart.cpp"
