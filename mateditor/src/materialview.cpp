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

    mdlPart->addModel(m_mdl, false, glm::vec3(), QStringLiteral(""), {material}, 0);
}

#include "moc_materialview.cpp"
