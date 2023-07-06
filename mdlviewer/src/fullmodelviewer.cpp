#include "fullmodelviewer.h"

#include "magic_enum.hpp"
#include "boneeditor.h"
#include <QVBoxLayout>

FullModelViewer::FullModelViewer(GameData *data) : data(data) {
    setWindowTitle("Full Model Viewer");
    setMinimumWidth(640);
    setMinimumHeight(480);

    auto layout = new QVBoxLayout();
    setLayout(layout);

    gearView = new GearView(data);

    auto viewportLayout = new QHBoxLayout();
    viewportLayout->addWidget(gearView, 1);
    viewportLayout->addWidget(new BoneEditor(gearView));
    layout->addLayout(viewportLayout);

    auto controlLayout = new QHBoxLayout();
    layout->addLayout(controlLayout);

    raceCombo = new QComboBox();
    connect(raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        gearView->setRace((Race)index);
    });
    controlLayout->addWidget(raceCombo);

    for (auto [race, race_name] : magic_enum::enum_entries<Race>()) {
        raceCombo->addItem(race_name.data());
    }

    genderCombo = new QComboBox();
    connect(genderCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        gearView->setGender((Gender)index);
    });
    controlLayout->addWidget(genderCombo);

    for (auto [gender, gender_name] : magic_enum::enum_entries<Gender>()) {
        genderCombo->addItem(gender_name.data());
    }

    connect(this, &FullModelViewer::gearChanged, this, &FullModelViewer::reloadGear);

    reloadGear();
}

void FullModelViewer::clear() {
    topSlot.reset();
    bottomSlot.reset();

    Q_EMIT gearChanged();
}

void FullModelViewer::addGear(GearInfo &info) {
    switch(info.slot) {
        case Slot::Body:
            topSlot = info;
            break;
        case Slot::Legs:
            bottomSlot = info;
            break;
        default:
            break;
    }

    Q_EMIT gearChanged();
}

void FullModelViewer::reloadGear() {
    gearView->clear();

    if (topSlot.has_value()) {
        gearView->addGear(*topSlot);
    } else {
        // smallclothes body
        GearInfo info = {};
        info.name = "Smallclothes Body";
        info.slot = Slot::Body;

        gearView->addGear(info);
    }

    if (bottomSlot.has_value()) {
        gearView->addGear(*bottomSlot);
    } else {
        // smallclothes legs
        GearInfo info = {};
        info.name = "Smallclothes Legs";
        info.slot = Slot::Legs;

        gearView->addGear(info);
    }
}

#include "moc_fullmodelviewer.cpp"