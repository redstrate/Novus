// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enemymodel.h"

#include "filecache.h"
#include "mdlpart.h"

#include <QPainter>
#include <physis.hpp>

enum class ModelCharaType {
    UnknownA = 0,
    Human = 1,
    DemiHuman = 2,
    Monster = 3,
    UnknownB = 4,
    UnknownC = 5,
};

QString folderNameFor(const ModelCharaType type)
{
    switch (type) {
    case ModelCharaType::Human:
        return QStringLiteral("human");
    case ModelCharaType::DemiHuman:
        return QStringLiteral("demihuman");
    case ModelCharaType::Monster:
        return QStringLiteral("monster");
    }

    Q_UNREACHABLE();
}

QString prefixNameFor(const ModelCharaType type)
{
    switch (type) {
    case ModelCharaType::Human:
        return QStringLiteral("h");
    case ModelCharaType::DemiHuman:
        return QStringLiteral("d");
    case ModelCharaType::Monster:
        return QStringLiteral("m");
    }

    Q_UNREACHABLE();
}

QString buildMdlPath(const ModelCharaType type, const uint16_t model, const uint16_t base)
{
    return QStringLiteral("chara/%1/m%2/obj/body/b%3/model/m%2b%3.mdl")
        .arg(folderNameFor(type))
        .arg(model, 4, 10, QLatin1Char('0'))
        .arg(base, 4, 10, QLatin1Char('0'));
}

QString buildMtrlPath(const ModelCharaType type, const uint16_t model, const uint16_t base, const uint8_t variant)
{
    return QStringLiteral("chara/%1/m%2/obj/body/b%3/material/v%4/mt_m%2b%3_a.mtrl")
        .arg(folderNameFor(type))
        .arg(model, 4, 10, QLatin1Char('0'))
        .arg(base, 4, 10, QLatin1Char('0'))
        .arg(variant, 4, 10, QLatin1Char('0'));
}

EnemyModel::EnemyModel(physis_SqPackResource *resource)
    : m_resource(resource)
    , m_cache(new FileCache(*m_resource))
{
    m_part = new MDLPart(m_resource, *m_cache);
    m_part->minimumCameraDistance = 0.05f;
    // TODO: terrible hack WRT to DPI but it works
    m_part->setMinimumSize(128 / m_part->devicePixelRatio(), 128 / m_part->devicePixelRatio());
    m_part->show();

    auto bnpcBaseExhFile = physis_sqpack_read(resource, "exd/BNpcBase.exh");
    auto bnpcBaseExh = physis_exh_parse(resource->platform, bnpcBaseExhFile);
    physis_free_file(&bnpcBaseExhFile);

    auto modelCharaExhFile = physis_sqpack_read(resource, "exd/ModelChara.exh");
    auto modelCharaExh = physis_exh_parse(resource->platform, modelCharaExhFile);
    physis_free_file(&modelCharaExhFile);

    auto bnpcBaseSheet = physis_sqpack_read_excel_sheet(resource, "BNpcBase", &bnpcBaseExh, Language::None);
    auto modelCharaSheet = physis_sqpack_read_excel_sheet(resource, "ModelChara", &modelCharaExh, Language::None);

    for (uint32_t i = 0; i < bnpcBaseSheet.page_count; i++) {
        for (uint32_t j = 0; j < bnpcBaseSheet.pages[i].entry_count; j++) {
            const auto entry = bnpcBaseSheet.pages[i].entries[j];

            const auto modelCharaId = entry.subrows[0].columns[5].u_int16._0;
            auto modelCharaRow = physis_excel_get_row(&modelCharaSheet, modelCharaId);

            const auto modelCharaType = static_cast<ModelCharaType>(modelCharaRow.columns[0].u_int8._0);
            if (modelCharaType == ModelCharaType::UnknownA || modelCharaType == ModelCharaType::UnknownB || modelCharaType == ModelCharaType::UnknownC) {
                continue;
            }

            const auto modelCharaModel = modelCharaRow.columns[1].u_int16._0;
            const auto modelCharaBase = modelCharaRow.columns[2].u_int8._0;
            // TODO: some assumption about this is wrong...
            const auto modelCharaVariant = 1; // modelCharaRow.columns[3].u_int8._0;

            m_enemies.push_back(new Enemy{.id = entry.row_id,
                                          .image = {},
                                          .mdlPath = buildMdlPath(modelCharaType, modelCharaModel, modelCharaBase),
                                          .mtrlPath = buildMtrlPath(modelCharaType, modelCharaModel, modelCharaBase, modelCharaVariant)});
        }
    }
}

int EnemyModel::rowCount(const QModelIndex &parent) const
{
    return m_enemies.size() / 8;
}

int EnemyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 8;
}

QVariant EnemyModel::data(const QModelIndex &index, int role) const
{
    const int realRow = index.row() * 8 + index.column();
    auto &enemy = m_enemies[realRow];
    if (role == Qt::DecorationRole) {
        if (enemy->image.isNull()) {
            enemy->image = renderModel(enemy->id, enemy->mdlPath, enemy->mtrlPath);
        }
        return enemy->image;
    }
    return {};
}

QImage EnemyModel::renderModel(const uint32_t id, const QString &mdlPath, const QString &mtrlPath) const
{
    m_part->clear();

    const auto mdlFile = physis_sqpack_read(m_resource, mdlPath.toStdString().c_str());
    if (mdlFile.size == 0) {
        return QImage{};
    }

    auto mdl = physis_mdl_parse(m_resource->platform, mdlFile);
    physis_free_file(&mdlFile);
    if (mdl.p_ptr == nullptr) {
        qWarning() << "While processing" << id << "could not find" << mdlPath;
        return QImage{};
    }

    auto mtrlFile = physis_sqpack_read(m_resource, mtrlPath.toStdString().c_str());
    if (mtrlFile.size == 0) {
        qWarning() << "While processing" << id << "could not find" << mtrlPath;
        physis_free_file(&mtrlFile);
        return QImage{};
    }
    auto mtrl = physis_material_parse(m_resource->platform, mtrlFile);
    physis_free_file(&mtrlFile);

    m_part->addModel(mdl,
                     false,
                     Transformation{
                         .translation = {},
                         .rotation = {},
                         .scale = {1, 1, 1},
                     },
                     QStringLiteral("enemy"),
                     {{mtrlPath.toStdString(), mtrl}},
                     0);

    auto image = m_part->grab();
    m_part->clear();

    physis_mtrl_free(&mtrl);
    physis_mdl_free(&mdl);

    QPainter p(&image);
    p.setPen(Qt::red);
    p.drawText(QPoint(50, 50), QString::number(id));

    return image;
}

#include "moc_enemymodel.cpp"
