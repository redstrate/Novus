#include "fullmodelviewer.h"

#include "boneeditor.h"
#include "magic_enum.hpp"
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>

FullModelViewer::FullModelViewer(GameData* data) : data(data) {
    setWindowTitle("Full Model Viewer");
    setMinimumWidth(640);
    setMinimumHeight(480);

    auto layout = new QVBoxLayout();
    setLayout(layout);

    cmp = physis_cmp_parse(physis_gamedata_extract_file(data, "chara/xls/charamake/human.cmp"));

    gearView = new GearView(data);
    updateCharacterParameters();

    connect(gearView, &GearView::modelReloaded, this, &FullModelViewer::updateCharacterParameters);

    auto viewportLayout = new QHBoxLayout();
    viewportLayout->addWidget(gearView, 1);
    layout->addLayout(viewportLayout);

    auto characterEditorWidget = new QWidget();
    auto characterEditorLayout = new QFormLayout();
    characterEditorWidget->setLayout(characterEditorLayout);

    auto characterHeight = new QSlider();
    characterHeight->setOrientation(Qt::Horizontal);
    characterHeight->setSliderPosition(50);
    connect(characterHeight, &QSlider::sliderMoved, this, [this](int position) {
        const float scale = (float)position / 100.0f;
        updateHeightScaling(scale);
    });
    characterEditorLayout->addRow("Height", characterHeight);

    auto bustSize = new QSlider();
    bustSize->setOrientation(Qt::Horizontal);
    bustSize->setSliderPosition(50);
    connect(bustSize, &QSlider::sliderMoved, this, [this](int position) {
        const float scale = (float)position / 100.0f;
        updateBustScaling(scale);
    });
    characterEditorLayout->addRow("Bust Size", bustSize);

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(new BoneEditor(gearView), "Bone Editor");
    tabWidget->addTab(characterEditorWidget, "Character Editor");
    viewportLayout->addWidget(tabWidget);

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

void FullModelViewer::addGear(GearInfo& info) {
    switch (info.slot) {
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

void FullModelViewer::updateHeightScaling(float scale) {
    auto& boneData = *gearView->part().skeleton;
    for (int i = 0; i < boneData.num_bones; i++) {
        const std::string_view name{boneData.bones[i].name};
        if (name == "n_root") {
            auto racialScaling =
                physis_cmp_get_racial_scaling_parameters(cmp, gearView->currentRace, gearView->currentSubrace);

            const float minSize =
                gearView->currentGender == Gender::Male ? racialScaling.male_min_size : racialScaling.female_min_size;
            const float maxSize =
                gearView->currentGender == Gender::Male ? racialScaling.male_max_size : racialScaling.female_max_size;

            const float size = glm::mix(minSize, maxSize, scale);

            boneData.bones[i].scale[0] = size;
            boneData.bones[i].scale[1] = size;
            boneData.bones[i].scale[2] = size;

            gearView->part().reloadRenderer();
        }
    }

    heightScale = scale;
}

void FullModelViewer::updateBustScaling(float scale) {
    auto& boneData = *gearView->part().skeleton;
    for (int i = 0; i < boneData.num_bones; i++) {
        const std::string_view name{boneData.bones[i].name};
        if (name == "j_mune_l" || name == "j_mune_r") {
            auto racialScaling =
                physis_cmp_get_racial_scaling_parameters(cmp, gearView->currentRace, gearView->currentSubrace);

            const float rangeX = glm::mix(racialScaling.bust_min_x, racialScaling.bust_max_x, scale);
            const float rangeY = glm::mix(racialScaling.bust_min_y, racialScaling.bust_max_y, scale);
            const float rangeZ = glm::mix(racialScaling.bust_min_z, racialScaling.bust_max_z, scale);

            boneData.bones[i].scale[0] = rangeX;
            boneData.bones[i].scale[1] = rangeY;
            boneData.bones[i].scale[2] = rangeZ;

            gearView->part().reloadRenderer();
        }
    }

    bustScale = scale;
}

void FullModelViewer::updateCharacterParameters() {
    updateHeightScaling(heightScale);
    updateBustScaling(bustScale);
}

#include "moc_fullmodelviewer.cpp"