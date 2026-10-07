// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enemyinfowindow.h"

#include "filecache.h"
#include "kawariapi.h"
#include "mdlpart.h"
#include "pathedit.h"

#include <KLocalizedString>
#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>

EnemyInfoWindow::EnemyInfoWindow(FileCache &cache,
                                 KawariApi *kawariApi,
                                 const QList<uint32_t> ids,
                                 const QString &mdlPath,
                                 const QList<QString> &mtrlPaths,
                                 QWidget *parent)
    : QDialog(parent)
{
    setMinimumSize(600, 400);

    const auto layout = new QHBoxLayout();
    setLayout(layout);

    const auto mdlPart = new MDLPart(cache, false, this);
    layout->addWidget(mdlPart, 1);

    // TODO: de-duplicate with EnemyModel please!!
    mdlPart->clear();
    mdlPart->addThreePointLighting();

    const auto mdlFile = cache.read(mdlPath);
    auto mdl = physis_mdl_parse(cache.platform(), mdlFile);

    std::vector<std::pair<std::string, physis_Material>> mtrls;
    for (const auto &path : mtrlPaths) {
        const auto mtrlFile = cache.read(path);
        mtrls.emplace_back(path.toStdString(), physis_material_parse(cache.platform(), mtrlFile));
    }

    const glm::vec3 boundsMin{mdl.bounding_box.min[0], mdl.bounding_box.min[1], mdl.bounding_box.min[2]};
    const glm::vec3 boundsMax{mdl.bounding_box.max[0], mdl.bounding_box.max[1], mdl.bounding_box.max[2]};

    const glm::vec3 size = boundsMax - boundsMin;
    const glm::vec3 center = (boundsMin + boundsMax) * 0.5f;
    const glm::vec3 normalizedCenter = -center * (1.0f / size);
    const float longest = glm::max(size.x, glm::max(size.y, size.z));

    mdlPart->addModel(mdl,
                      false,
                      Transformation{
                          .translation = {normalizedCenter[0], normalizedCenter[1], normalizedCenter[2]},
                          .rotation = {},
                          // Normalize scale
                          .scale = {1.0f / longest, 1.0f / longest, 1.0f / longest},
                      },
                      QStringLiteral("enemy"),
                      mtrls);

    const auto formLayoutWidget = new QWidget();
    layout->addWidget(formLayoutWidget);

    const auto formLayout = new QFormLayout();
    formLayoutWidget->setLayout(formLayout);

    const auto idsList = new QListWidget();
    for (const auto id : ids) {
        idsList->addItem(QString::number(id));
    }

    formLayout->addRow(i18n("IDs"), idsList);

    const auto mdlPathEdit = new PathEdit();
    mdlPathEdit->setPath(mdlPath);
    mdlPathEdit->setReadOnly(true);

    formLayout->addRow(i18n("MDL"), mdlPathEdit);

    for (int i = 0; i < mtrlPaths.size(); i++) {
        const auto mtrlPathEdit = new PathEdit();
        mtrlPathEdit->setPath(mtrlPaths[i]);
        mtrlPathEdit->setReadOnly(true);

        formLayout->addRow(i18n("MTRL %1").arg(i), mtrlPathEdit);
    }

    // TODO: allow selecting the ID?
    auto spawnButton = new QPushButton(i18n("Spawn"));
    connect(spawnButton, &QPushButton::clicked, this, [kawariApi, ids] {
        kawariApi->spawnBattleNpc(ids.constFirst());
    });
    formLayout->addWidget(spawnButton);
}
