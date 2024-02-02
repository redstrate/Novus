// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mapview.h"

#include <QThreadPool>
#include <QVBoxLayout>

#include "filecache.h"

MapView::MapView(GameData *data, FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , data(data)
    , cache(cache)
{
    mdlPart = new MDLPart(data, cache);
    mdlPart->enableFreemode();

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mdlPart);
    setLayout(layout);
}

MDLPart &MapView::part() const
{
    return *mdlPart;
}

void MapView::addTerrain(QString basePath, physis_Terrain terrain)
{
    mdlPart->clear();

    for (int i = 0; i < terrain.num_plates; i++) {
        QString mdlPath = QStringLiteral("%1%2").arg(basePath, QString::fromStdString(terrain.plates[i].filename));
        std::string mdlPathStd = mdlPath.toStdString();

        auto plateMdlFile = physis_gamedata_extract_file(data, mdlPathStd.c_str());
        auto plateMdl = physis_mdl_parse(plateMdlFile);

        mdlPart->addModel(plateMdl, glm::vec3(terrain.plates[i].position[0], 0.0f, terrain.plates[i].position[1]), QStringLiteral("terapart%1").arg(i), {}, 0);
    }
}

#include "moc_mapview.cpp"
