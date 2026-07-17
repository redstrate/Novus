// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmppart.h"

#include <KLocalizedString>
#include <QFormLayout>
#include <QGroupBox>
#include <QTreeWidget>
#include <physis.hpp>

#include "magic_enum.hpp"

// TODO: move this to physis
struct RaceTree {
    Race baseRace;
    std::vector<Tribe> subRaces;
};

const std::vector<RaceTree> raceTree = {{Race::Hyur, {Tribe::Midlander, Tribe::Highlander}},
                                        {Race::Elezen, {Tribe::Wildwood, Tribe::Duskwight}},
                                        {Race::Miqote, {Tribe::Seeker, Tribe::Keeper}},
                                        {Race::Roegadyn, {Tribe::SeaWolf, Tribe::Hellion}},
                                        {Race::Lalafell, {Tribe::Plainsfolk, Tribe::Dunesfolk}},
                                        {Race::AuRa, {Tribe::Raen, Tribe::Xaela}},
                                        {Race::Hrothgar, {Tribe::Hellion, Tribe::Lost}},
                                        {Race::Viera, {Tribe::Rava, Tribe::Veena}}};

CmpPart::CmpPart(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout();
    setLayout(m_layout);
}

void CmpPart::load(const Platform platform, const physis_Buffer file)
{
    m_cmp = physis_cmp_parse(platform, file);

    const auto raceListWidget = new QTreeWidget();
    raceListWidget->setMaximumWidth(200);
    raceListWidget->setHeaderLabel(i18nc("@title:column", "Race"));
    m_layout->addWidget(raceListWidget);

    for (const auto &race : raceTree) {
        const auto item = new QTreeWidgetItem();
        item->setText(0, QLatin1String(magic_enum::enum_name(race.baseRace).data()));
        raceListWidget->addTopLevelItem(item);

        for (const auto subrace : race.subRaces) {
            const auto subItem = new QTreeWidgetItem();
            subItem->setText(0, QLatin1String(magic_enum::enum_name(subrace).data()));
            subItem->setData(0, Qt::UserRole, QVariant::fromValue(new RaceTreeData(race.baseRace, subrace)));
            item->addChild(subItem);
        }
    }

    raceListWidget->expandAll();

    connect(raceListWidget, &QTreeWidget::itemClicked, [this](const QTreeWidgetItem *item, const int column) {
        Q_UNUSED(column)
        if (const auto treeData = qvariant_cast<RaceTreeData *>(item->data(0, Qt::UserRole))) {
            loadRaceData(treeData->race, treeData->subrace);
        }
    });

    const auto detailBox = new QGroupBox();
    m_layout->addWidget(detailBox);
    const auto detailBoxLayout = new QFormLayout();
    detailBox->setLayout(detailBoxLayout);

    m_maleMinSize = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Male Min Size"), m_maleMinSize);

    m_maleMaxSize = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Male Max Size"), m_maleMaxSize);

    m_maleMinTail = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Male Min Tail"), m_maleMinTail);

    m_maleMaxTail = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Male Max Tail"), m_maleMaxTail);

    m_femaleMinSize = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Female Min Size"), m_femaleMinSize);

    m_femaleMaxSize = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Female Max Size"), m_femaleMaxSize);

    m_femaleMinTail = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Female Min Tail"), m_femaleMinTail);

    m_femaleMaxTail = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Female Max Tail"), m_femaleMaxTail);

    m_bustMinX = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Min X"), m_bustMinX);

    m_bustMinY = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Min Y"), m_bustMinY);

    m_bustMinZ = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Min Z"), m_bustMinZ);

    m_bustMaxX = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Max X"), m_bustMaxX);

    m_bustMaxY = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Max Y"), m_bustMaxY);

    m_bustMaxZ = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Max Z"), m_bustMaxZ);

    loadRaceData(Race::Hyur, Tribe::Midlander);
}

void CmpPart::loadRaceData(const Race race, const Tribe subrace) const
{
    const auto raceData = physis_cmp_get_racial_scaling_parameters(m_cmp, race, subrace);

    m_maleMinSize->setValue(raceData.male_min_size);
    m_maleMaxSize->setValue(raceData.male_max_size);

    m_maleMinTail->setValue(raceData.male_min_tail);
    m_maleMaxTail->setValue(raceData.male_max_tail);

    m_femaleMinSize->setValue(raceData.female_min_size);
    m_femaleMaxSize->setValue(raceData.female_max_size);

    m_femaleMinTail->setValue(raceData.female_min_tail);
    m_femaleMaxTail->setValue(raceData.female_max_tail);

    m_bustMinX->setValue(raceData.bust_min_x);
    m_bustMinY->setValue(raceData.bust_min_y);
    m_bustMinZ->setValue(raceData.bust_min_z);

    m_bustMaxX->setValue(raceData.bust_max_x);
    m_bustMaxY->setValue(raceData.bust_max_y);
    m_bustMaxZ->setValue(raceData.bust_max_z);
}

#include "moc_cmppart.cpp"
