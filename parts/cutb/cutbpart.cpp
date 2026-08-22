// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cutbpart.h"

#include <KLocalizedString>
#include <QFormLayout>
#include <QGroupBox>
#include <QTreeWidget>
#include <physis.hpp>

#include "magic_enum.hpp"

CutbPart::CutbPart(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout();
    setLayout(m_layout);
}

void CutbPart::load(const Platform platform, const physis_Buffer file)
{
    m_cmp = physis_cmp_parse(platform, file);

    const auto raceListWidget = new QTreeWidget();
    raceListWidget->setMaximumWidth(200);
    raceListWidget->setHeaderLabel(i18nc("@title:column", "Scenes"));
    m_layout->addWidget(raceListWidget);

    // for (const auto &race : raceTree) {
    //     const auto item = new QTreeWidgetItem();
    //     item->setText(0, QLatin1String(magic_enum::enum_name(race.baseRace).data()));
    //     raceListWidget->addTopLevelItem(item);
    //
    //     for (const auto subrace : race.subRaces) {
    //         const auto subItem = new QTreeWidgetItem();
    //         subItem->setText(0, QLatin1String(magic_enum::enum_name(subrace).data()));
    //         subItem->setData(0, Qt::UserRole, QVariant::fromValue(new RaceTreeData(race.baseRace, subrace)));
    //         item->addChild(subItem);
    //     }
    // }

    raceListWidget->expandAll();

    // connect(raceListWidget, &QTreeWidget::itemClicked, [this](const QTreeWidgetItem *item, const int column) {
    //     Q_UNUSED(column)
    //     if (const auto treeData = qvariant_cast<RaceTreeData *>(item->data(0, Qt::UserRole))) {
    //         loadRaceData(treeData->race, treeData->subrace);
    //     }
    // });

    const auto detailBox = new QGroupBox();
    m_layout->addWidget(detailBox);
    const auto detailBoxLayout = new QFormLayout();
    detailBox->setLayout(detailBoxLayout);
}

#include "moc_cutbpart.cpp"
