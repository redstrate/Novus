// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cutbpart.h"

#include <KLocalizedString>
#include <QFormLayout>
#include <QListWidget>
#include <physis.hpp>

#include "magic_enum.hpp"
#include "scenepart.h"

CutbPart::CutbPart(FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , m_cache(cache)
{
    m_layout = new QHBoxLayout();
    setLayout(m_layout);

    m_sceneListWidget = new QListWidget();
    m_sceneListWidget->setMaximumWidth(200);
    m_layout->addWidget(m_sceneListWidget);

    connect(m_sceneListWidget, &QListWidget::itemClicked, [this](const QListWidgetItem *item) {
        const QString lvbPath = QStringLiteral("bg/%1.lvb").arg(item->data(Qt::DisplayRole).toString());
        const auto lvbFile = m_cache.read(lvbPath);
        if (lvbFile.size > 0) {
            m_part->loadLvb(lvbFile);
        } else {
            qWarning() << "Failed to load scene" << lvbPath;
        }
    });

    m_part = new ScenePart(m_cache);
    m_layout->addWidget(m_part);
}

void CutbPart::load(const Platform platform, const physis_Buffer file) const
{
    m_sceneListWidget->clear();

    const auto cutb = physis_cutb_parse(platform, file);
    if (cutb.num_nodes > 0) {
        for (uint32_t i = 0; i < cutb.num_nodes; i++) {
            const auto node = cutb.nodes[i];
            switch (node.tag) {
            case physis_CutsceneNode::Tag::Ctds: {
                const auto item = new QListWidgetItem();
                item->setText(QString::fromStdString(node.ctds._0.level_name));
                m_sceneListWidget->addItem(item);
            } break;
            case physis_CutsceneNode::Tag::Unknown:
                break;
            }
        }
    } else {
        qWarning() << "Empty or invalid cutb file!";
    }
}

#include "moc_cutbpart.cpp"
