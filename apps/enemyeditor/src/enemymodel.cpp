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
    default:
        break;
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
    default:
        break;
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

EnemyModel::EnemyModel(FileCache &cache)
    : m_cache(cache)
{
    m_part = new MDLPart(m_cache);
    m_part->minimumCameraDistance = 0.05f;
    // TODO: terrible hack WRT to DPI but it works
    m_part->setMinimumSize(128 / m_part->devicePixelRatio(), 128 / m_part->devicePixelRatio());
    m_part->show();

    auto bnpcBaseExhFile = m_cache.read(QStringLiteral("exd/BNpcBase.exh"));
    auto bnpcBaseExh = physis_exh_parse(m_cache.platform(), bnpcBaseExhFile);

    auto modelCharaExhFile = m_cache.read(QStringLiteral("exd/ModelChara.exh"));
    auto modelCharaExh = physis_exh_parse(m_cache.platform(), modelCharaExhFile);

    auto bnpcBaseSheet = m_cache.readExcelSheet(QStringLiteral("BNpcBase"), &bnpcBaseExh, Language::None);
    auto modelCharaSheet = m_cache.readExcelSheet(QStringLiteral("ModelChara"), &modelCharaExh, Language::None);

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
    Q_UNUSED(parent)
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
    if (role == CustomRole::IdRole) {
        return enemy->id;
    }
    if (role == CustomRole::MdlPath) {
        return enemy->mdlPath;
    }
    if (role == CustomRole::MtrlPath) {
        return enemy->mtrlPath;
    }
    return {};
}

QImage EnemyModel::renderModel(const uint32_t id, const QString &mdlPath, const QString &mtrlPath) const
{
    m_part->clear();

    const auto mdlFile = m_cache.read(mdlPath);
    if (mdlFile.size == 0) {
        return QImage{};
    }

    auto mdl = physis_mdl_parse(m_cache.platform(), mdlFile);
    if (mdl.p_ptr == nullptr) {
        qWarning() << "While processing" << id << "could not find" << mdlPath;
        return QImage{};
    }

    auto mtrlFile = m_cache.read(mtrlPath);
    if (mtrlFile.size == 0) {
        qWarning() << "While processing" << id << "could not find" << mtrlPath;
        return QImage{};
    }
    auto mtrl = physis_material_parse(m_cache.platform(), mtrlFile);

    const glm::vec3 boundsMin{mdl.bounding_box.min[0], mdl.bounding_box.min[1], mdl.bounding_box.min[2]};
    const glm::vec3 boundsMax{mdl.bounding_box.max[0], mdl.bounding_box.max[1], mdl.bounding_box.max[2]};

    const glm::vec3 size = boundsMax - boundsMin;
    const glm::vec3 center = (boundsMin + boundsMax) * 0.5f;
    const glm::vec3 normalizedCenter = -center * (1.0f / size);

    m_part->addModel(mdl,
                     false,
                     Transformation{
                         .translation = {normalizedCenter[0], normalizedCenter[1], normalizedCenter[2]},
                         .rotation = {},
                         // Normalize scale
                         .scale = {1.0f / size[0], 1.0f / size[1], 1.0f / size[2]},
                     },
                     QStringLiteral("enemy"),
                     {{mtrlPath.toStdString(), mtrl}});

    auto image = m_part->grab();
    m_part->clear();

    physis_mtrl_free(&mtrl);
    physis_mdl_free(&mdl);

    QPainter p(&image);
    p.setPen(Qt::red);
    p.drawText(QPoint(50, 50), QString::number(id));

    return image;
}
