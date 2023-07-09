#include "cmpeditor.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTreeWidget>

#include "magic_enum.hpp"

// TODO: move this to physis
struct RaceTree {
    Race baseRace;
    std::vector<Subrace> subRaces;
};

std::vector<RaceTree> raceTree = {
    {Race::Hyur, {Subrace::Midlander, Subrace::Highlander}},
    {Race::Elezen, {Subrace::Wildwood, Subrace::Duskwight}},
    {Race::Miqote, {Subrace::Seeker, Subrace::Keeper}},
    {Race::Roegadyn, {Subrace::SeaWolf, Subrace::Hellion}},
    {Race::Lalafell, {Subrace::Plainsfolk, Subrace::Dunesfolk}},
    {Race::AuRa, {Subrace::Raen, Subrace::Xaela}},
    {Race::Hrothgar, {Subrace::Hellion, Subrace::Lost}},
    {Race::Viera, {Subrace::Rava, Subrace::Veena}}};

CmpEditor::CmpEditor(GameData* data) : data(data) {
    setWindowTitle("CMP Editor");

    auto layout = new QHBoxLayout();
    setLayout(layout);

    cmp = physis_cmp_parse(physis_gamedata_extract_file(data, "chara/xls/charamake/human.cmp"));

    auto raceListWidget = new QTreeWidget();
    raceListWidget->setMaximumWidth(200);
    layout->addWidget(raceListWidget);

    for (auto race : raceTree) {
        auto item = new QTreeWidgetItem();
        item->setText(0, magic_enum::enum_name(race.baseRace).data());
        raceListWidget->addTopLevelItem(item);

        for (auto subrace : race.subRaces) {
            auto subItem = new QTreeWidgetItem();
            subItem->setText(0, magic_enum::enum_name(subrace).data());
            subItem->setData(0, Qt::UserRole, QVariant::fromValue(new RaceTreeData(race.baseRace, subrace)));
            item->addChild(subItem);
        }
    }

    raceListWidget->expandAll();

    connect(raceListWidget, &QTreeWidget::itemClicked, [this](QTreeWidgetItem* item, int column) {
        if (auto treeData = qvariant_cast<RaceTreeData*>(item->data(0, Qt::UserRole))) {
            loadRaceData(treeData->race, treeData->subrace);
        }
    });

    auto detailBox = new QGroupBox();
    layout->addWidget(detailBox);
    auto detailBoxLayout = new QFormLayout();
    detailBox->setLayout(detailBoxLayout);

    maleMinSize = new QDoubleSpinBox();
    detailBoxLayout->addRow("Male Min Size", maleMinSize);

    maleMaxSize = new QDoubleSpinBox();
    detailBoxLayout->addRow("Male Max Size", maleMaxSize);

    maleMinTail = new QDoubleSpinBox();
    detailBoxLayout->addRow("Male Min Tail", maleMinTail);

    maleMaxTail = new QDoubleSpinBox();
    detailBoxLayout->addRow("Male Max Tail", maleMaxTail);

    femaleMinSize = new QDoubleSpinBox();
    detailBoxLayout->addRow("Female Min Size", femaleMinSize);

    femaleMaxSize = new QDoubleSpinBox();
    detailBoxLayout->addRow("Female Max Size", femaleMaxSize);

    femaleMinTail = new QDoubleSpinBox();
    detailBoxLayout->addRow("Female Min Tail", femaleMinTail);

    femaleMaxTail = new QDoubleSpinBox();
    detailBoxLayout->addRow("Female Max Tail", femaleMaxTail);

    bustMinX = new QDoubleSpinBox();
    detailBoxLayout->addRow("Bust Min X", bustMinX);

    bustMinY = new QDoubleSpinBox();
    detailBoxLayout->addRow("Bust Min Y", bustMinY);

    bustMinZ = new QDoubleSpinBox();
    detailBoxLayout->addRow("Bust Min Z", bustMinZ);

    bustMaxX = new QDoubleSpinBox();
    detailBoxLayout->addRow("Bust Max X", bustMaxX);

    bustMaxY = new QDoubleSpinBox();
    detailBoxLayout->addRow("Bust Max Y", bustMaxY);

    bustMaxZ = new QDoubleSpinBox();
    detailBoxLayout->addRow("Bust Max Z", bustMaxZ);

    loadRaceData(Race::Hyur, Subrace::Midlander);
}

void CmpEditor::loadRaceData(Race race, Subrace subrace) {
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

#include "moc_cmpeditor.cpp"