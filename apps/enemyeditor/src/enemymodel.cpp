// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enemymodel.h"

#include "filecache.h"
#include "mdlpart.h"
#include "settings.h"
#include "vulkanwindow.h"

#include <QPainter>
#include <QThread>
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
    return QStringLiteral("chara/%1/m%2/obj/body/b%3/material/v%4")
        .arg(folderNameFor(type))
        .arg(model, 4, 10, QLatin1Char('0'))
        .arg(base, 4, 10, QLatin1Char('0'))
        .arg(variant, 4, 10, QLatin1Char('0'));
}

EnemyModel::EnemyModel(FileCache &cache)
    : m_cache(cache)
{
    m_imageCache = new QHash<QString, QPair<QImage, QList<QString>>>(); // TODO: haha this is so stupid

    m_part = new MDLPart(m_cache, false);
    m_part->minimumCameraDistance = 0.05f;
    m_part->vkWindow()->present = false; // We abuse this for off-screen rendering
    m_part->setFixedSize(128, 128);
    m_part->show();

    const auto bnpcBaseExhFile = m_cache.read(QStringLiteral("exd/BNpcBase.exh"));
    const auto bnpcBaseExh = physis_exh_parse(m_cache.platform(), bnpcBaseExhFile);

    const auto modelCharaExhFile = m_cache.read(QStringLiteral("exd/ModelChara.exh"));
    const auto modelCharaExh = physis_exh_parse(m_cache.platform(), modelCharaExhFile);

    const auto companionExhFile = m_cache.read(QStringLiteral("exd/Companion.exh"));
    const auto companionExh = physis_exh_parse(m_cache.platform(), companionExhFile);

    const auto bnpcBaseSheet = m_cache.readExcelSheet(QStringLiteral("BNpcBase"), &bnpcBaseExh, Language::None);
    const auto modelCharaSheet = m_cache.readExcelSheet(QStringLiteral("ModelChara"), &modelCharaExh, Language::None);
    const auto companionSheet = m_cache.readExcelSheet(QStringLiteral("Companion"), &companionExh, getLanguage());

    // Build a list of minions (internally called companions) so we don't show them with regular enemies
    QList<uint32_t> rejectedModelCharas;
    for (uint32_t i = 0; i < companionSheet.page_count; i++) {
        for (uint32_t j = 0; j < companionSheet.pages[i].entry_count; j++) {
            const auto entry = companionSheet.pages[i].entries[j];

            rejectedModelCharas.push_back(entry.subrows[0].columns[8].u_int16._0);
        }
    }

    for (uint32_t i = 0; i < bnpcBaseSheet.page_count; i++) {
        for (uint32_t j = 0; j < bnpcBaseSheet.pages[i].entry_count; j++) {
            const auto entry = bnpcBaseSheet.pages[i].entries[j];

            const auto modelCharaId = entry.subrows[0].columns[5].u_int16._0;
            if (rejectedModelCharas.contains(modelCharaId)) {
                continue;
            }

            const auto modelCharaRow = physis_excel_get_row(&modelCharaSheet, modelCharaId);

            const auto modelCharaType = static_cast<ModelCharaType>(modelCharaRow.columns[0].u_int8._0);
            if (modelCharaType == ModelCharaType::UnknownA || modelCharaType == ModelCharaType::UnknownB || modelCharaType == ModelCharaType::UnknownC) {
                continue;
            }

            const auto modelCharaModel = modelCharaRow.columns[1].u_int16._0;
            const auto modelCharaBase = modelCharaRow.columns[2].u_int8._0;
            const auto modelCharaVariant = modelCharaRow.columns[3].u_int8._0;

            const auto mdlPath = buildMdlPath(modelCharaType, modelCharaModel, modelCharaBase);
            if (!m_cache.exists(mdlPath)) {
                continue;
            }

            const auto &key = mdlPath;

            // Don't add duplicate models
            if (m_seenEnemies.contains(key)) {
                const auto it = std::ranges::find_if(m_enemies, [mdlPath](const auto &enemy) {
                    return enemy->mdlPath == mdlPath;
                });
                if (it != m_enemies.end()) {
                    (*it)->ids.push_back(entry.row_id);
                }
                continue;
            }

            m_enemies.push_back(new Enemy{.ids = {entry.row_id},
                                          .mdlPath = mdlPath,
                                          .baseMtrlPath = buildMtrlPath(modelCharaType, modelCharaModel, modelCharaBase, modelCharaVariant)});
            m_seenEnemies.push_back(key);
        }
    }
}

int EnemyModel::rowCount(const QModelIndex &parent) const
{
    return m_enemies.size() / columnCount(parent);
}

int EnemyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 8;
}

QVariant EnemyModel::data(const QModelIndex &index, const int role) const
{
    const int realRow = index.row() * columnCount(index.parent()) + index.column();
    auto &enemy = m_enemies[realRow];
    const auto &key = enemy->mdlPath;
    if (role == Qt::DecorationRole) {
        if (!m_imageCache->contains(key)) {
            (*m_imageCache)[key] = renderModel(enemy->mdlPath, enemy->baseMtrlPath);
        }
        return (*m_imageCache)[key].first.scaledToHeight(128, Qt::SmoothTransformation);
    }
    if (role == IdsRole) {
        return QVariant::fromValue(enemy->ids);
    }
    if (role == MdlPath) {
        return enemy->mdlPath;
    }
    if (role == MtrlPaths) {
        return (*m_imageCache)[key].second;
    }
    return {};
}

QPair<QImage, QList<QString>> EnemyModel::renderModel(const QString &mdlPath, const QString &baseMtrlPath) const
{
    const auto mdlFile = m_cache.read(mdlPath);
    if (mdlFile.size == 0) {
        qWarning() << "Could not find MDL file for" << mdlPath;
        return {};
    }

    const auto mdl = physis_mdl_parse(m_cache.platform(), mdlFile);
    if (mdl.p_ptr == nullptr) {
        qWarning() << "While processing could not find" << mdlPath;
        return {};
    }

    QList<QString> mtrlPaths;
    for (uint32_t z = 0; z < mdl.num_material_names; z++) {
        mtrlPaths.push_back(QStringLiteral("%1%2").arg(baseMtrlPath, QString::fromStdString(mdl.material_names[z])));
    }

    std::vector<std::pair<std::string, physis_Material>> mtrls;
    for (const auto &path : mtrlPaths) {
        const auto mtrlFile = m_cache.read(path);
        if (mtrlFile.size == 0) {
            qWarning() << "While processing could not find" << path << "Skipping!";
        }
        mtrls.emplace_back(path.toStdString(), physis_material_parse(m_cache.platform(), mtrlFile));
    }

    const glm::vec3 boundsMin{mdl.bounding_box.min[0], mdl.bounding_box.min[1], mdl.bounding_box.min[2]};
    const glm::vec3 boundsMax{mdl.bounding_box.max[0], mdl.bounding_box.max[1], mdl.bounding_box.max[2]};

    const glm::vec3 size = boundsMax - boundsMin;
    const glm::vec3 center = (boundsMin + boundsMax) * 0.5f;
    const glm::vec3 normalizedCenter = -center * (1.0f / size);
    const float longest = glm::max(size.x, glm::max(size.y, size.z));

    m_part->addModel(mdl,
                     false,
                     Transformation{
                         .translation = {normalizedCenter[0], normalizedCenter[1], normalizedCenter[2]},
                         .rotation = {},
                         // Normalize scale
                         .scale = {1.0f / longest, 1.0f / longest, 1.0f / longest},
                     },
                     QStringLiteral("enemy"),
                     mtrls);

    auto image = m_part->grab();
    m_part->clear();
    m_part->addThreePointLighting();

    for (const auto &mtrl : mtrls | std::views::values) {
        physis_mtrl_free(&mtrl);
    }
    physis_mdl_free(&mdl);

    return {image, mtrlPaths};
}
