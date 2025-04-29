// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "materialview.h"

#include <QThreadPool>
#include <QVBoxLayout>

#include "filecache.h"

MaterialView::MaterialView(GameData *data, FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , data(data)
    , cache(cache)
{
    mdlPart = new MDLPart(data, cache);

    auto plateMdlFile = physis_gamedata_extract_file(data, "chara/equipment/e0028/model/c0101e0028_top.mdl");
    m_mdl = physis_mdl_parse(plateMdlFile);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mdlPart);
    setLayout(layout);
}

MDLPart &MaterialView::part() const
{
    return *mdlPart;
}

void MaterialView::addSphere(physis_Material material)
{
    mdlPart->clear();

    const int raceCode = physis_get_race_code(Race::Hyur, Tribe::Midlander, Gender::Male);

    QString skelName = QStringLiteral("chara/human/c%1/skeleton/base/b0001/skl_c%1b0001.sklb").arg(raceCode, 4, 10, QLatin1Char{'0'});
    std::string skelNameStd = skelName.toStdString();
    mdlPart->setSkeleton(physis_parse_skeleton(physis_gamedata_extract_file(data, skelNameStd.c_str())));

    mdlPart->addModel(m_mdl, true, glm::vec3(), QStringLiteral(""), {material}, 0);
}

#include "moc_materialview.cpp"
