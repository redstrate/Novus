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
    std::vector<Subrace> subRaces;
};

const std::vector<RaceTree> raceTree = {{Race::Hyur, {Subrace::Midlander, Subrace::Highlander}},
                                        {Race::Elezen, {Subrace::Wildwood, Subrace::Duskwight}},
                                        {Race::Miqote, {Subrace::Seeker, Subrace::Keeper}},
                                        {Race::Roegadyn, {Subrace::SeaWolf, Subrace::Hellion}},
                                        {Race::Lalafell, {Subrace::Plainsfolk, Subrace::Dunesfolk}},
                                        {Race::AuRa, {Subrace::Raen, Subrace::Xaela}},
                                        {Race::Hrothgar, {Subrace::Hellion, Subrace::Lost}},
                                        {Race::Viera, {Subrace::Rava, Subrace::Veena}}};

CmpPart::CmpPart(GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    layout = new QHBoxLayout();
    setLayout(layout);
}

void CmpPart::load(physis_Buffer file)
{
    cmp = physis_cmp_parse(file);

    auto raceListWidget = new QTreeWidget();
    raceListWidget->setMaximumWidth(200);
    raceListWidget->setHeaderLabel(i18nc("@title:column", "Race"));
    layout->addWidget(raceListWidget);

    for (const auto &race : raceTree) {
        auto item = new QTreeWidgetItem();
        item->setText(0, QLatin1String(magic_enum::enum_name(race.baseRace).data()));
        raceListWidget->addTopLevelItem(item);

        for (auto subrace : race.subRaces) {
            auto subItem = new QTreeWidgetItem();
            subItem->setText(0, QLatin1String(magic_enum::enum_name(subrace).data()));
            subItem->setData(0, Qt::UserRole, QVariant::fromValue(new RaceTreeData(race.baseRace, subrace)));
            item->addChild(subItem);
        }
    }

    raceListWidget->expandAll();

    connect(raceListWidget, &QTreeWidget::itemClicked, [this](QTreeWidgetItem *item, int column) {
        Q_UNUSED(column)
        if (auto treeData = qvariant_cast<RaceTreeData *>(item->data(0, Qt::UserRole))) {
            loadRaceData(treeData->race, treeData->subrace);
        }
    });

    auto detailBox = new QGroupBox();
    layout->addWidget(detailBox);
    auto detailBoxLayout = new QFormLayout();
    detailBox->setLayout(detailBoxLayout);

    maleMinSize = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Male Min Size"), maleMinSize);

    maleMaxSize = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Male Max Size"), maleMaxSize);

    maleMinTail = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Male Min Tail"), maleMinTail);

    maleMaxTail = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Male Max Tail"), maleMaxTail);

    femaleMinSize = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Female Min Size"), femaleMinSize);

    femaleMaxSize = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Female Max Size"), femaleMaxSize);

    femaleMinTail = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Female Min Tail"), femaleMinTail);

    femaleMaxTail = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Female Max Tail"), femaleMaxTail);

    bustMinX = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Min X"), bustMinX);

    bustMinY = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Min Y"), bustMinY);

    bustMinZ = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Min Z"), bustMinZ);

    bustMaxX = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Max X"), bustMaxX);

    bustMaxY = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Max Y"), bustMaxY);

    bustMaxZ = new QDoubleSpinBox();
    detailBoxLayout->addRow(i18n("Bust Max Z"), bustMaxZ);

    loadRaceData(Race::Hyur, Subrace::Midlander);
}

void CmpPart::loadRaceData(Race race, Subrace subrace)
{
    auto raceData = physis_cmp_get_racial_scaling_parameters(cmp, race, subrace);

    maleMinSize->setValue(raceData.male_min_size);
    maleMaxSize->setValue(raceData.male_max_size);

    maleMinTail->setValue(raceData.male_min_tail);
    maleMaxTail->setValue(raceData.male_max_tail);

    femaleMinSize->setValue(raceData.female_min_size);
    femaleMaxSize->setValue(raceData.female_max_size);

    femaleMinTail->setValue(raceData.female_min_tail);
    femaleMaxTail->setValue(raceData.female_max_tail);

    bustMinX->setValue(raceData.bust_min_x);
    bustMinY->setValue(raceData.bust_min_y);
    bustMinZ->setValue(raceData.bust_min_z);

    bustMaxX->setValue(raceData.bust_max_x);
    bustMaxY->setValue(raceData.bust_max_y);
    bustMaxZ->setValue(raceData.bust_max_z);
}

#include "moc_cmppart.cpp"