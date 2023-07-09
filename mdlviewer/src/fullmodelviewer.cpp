#include "fullmodelviewer.h"

#include "boneeditor.h"
#include "magic_enum.hpp"
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QRadioButton>
#include <QVBoxLayout>

FullModelViewer::FullModelViewer(GameData* data, FileCache& cache) : data(data) {
    setWindowTitle("Full Model Viewer");
    setMinimumWidth(640);
    setMinimumHeight(480);

    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto menuBar = new QMenuBar();
    layout->addWidget(menuBar);

    auto fileMenu = menuBar->addMenu("File");

    auto datOpenAction = fileMenu->addAction("Load character DAT...");
    connect(datOpenAction, &QAction::triggered, [=] {
        auto fileName = QFileDialog::getOpenFileName(nullptr,
                                                     "Open DAT File",
                                                     "~",
                                                     "FFXIV Character DAT File (*.dat)");

        auto buffer = physis_read_file(fileName.toStdString().c_str());

        auto charDat = physis_chardat_parse(buffer);

        gearView->setRace(charDat.race);
        gearView->setGender(charDat.gender);
        //gearView->setSubrace(charDat.subrace);
        gearView->setFace(charDat.head);
        gearView->setHair(charDat.hair);
        updateBustScaling((float)charDat.bust / 100.0f);
        updateHeightScaling((float)charDat.height / 100.0f);
    });

    cmp = physis_cmp_parse(physis_gamedata_extract_file(data, "chara/xls/charamake/human.cmp"));

    gearView = new GearView(data, cache);
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

    characterEditorLayout->addWidget(addFaceGroup());
    characterEditorLayout->addWidget(addHairGroup());
    characterEditorLayout->addWidget(addEarGroup());
    characterEditorLayout->addWidget(addTailGroup());

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(new BoneEditor(gearView), "Bone Editor");
    tabWidget->addTab(characterEditorWidget, "Character Editor");
    viewportLayout->addWidget(tabWidget);

    auto controlLayout = new QHBoxLayout();
    layout->addLayout(controlLayout);

    raceCombo = new QComboBox();
    controlLayout->addWidget(raceCombo);

    for (auto [race, race_name] : magic_enum::enum_entries<Race>()) {
        raceCombo->addItem(race_name.data(), (int)race);
    }

    subraceCombo = new QComboBox();
    connect(subraceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        gearView->setSubrace((Subrace)subraceCombo->itemData(index).toInt());
    });
    controlLayout->addWidget(subraceCombo);

    connect(raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        gearView->setRace((Race)raceCombo->itemData(index).toInt());

        updateSupportedSubraces();
    });
    updateSupportedSubraces();

    genderCombo = new QComboBox();
    connect(genderCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        gearView->setGender((Gender)genderCombo->itemData(index).toInt());
    });
    controlLayout->addWidget(genderCombo);

    for (auto [gender, gender_name] : magic_enum::enum_entries<Gender>()) {
        genderCombo->addItem(gender_name.data(), (int)gender);
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

    // smallclothes hands
    {
        GearInfo info = {};
        info.name = "Smallclothes Hands";
        info.slot = Slot::Hands;

        gearView->addGear(info);
    }

    // smallclothes hands
    {
        GearInfo info = {};
        info.name = "Smallclothes Feet";
        info.slot = Slot::Feet;

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

void FullModelViewer::updateSupportedSubraces() {
    subraceCombo->clear();
    for (auto subrace : physis_get_supported_subraces(gearView->currentRace).subraces) {
        subraceCombo->addItem(magic_enum::enum_name(subrace).data(), (int)subrace);
    }
}

QGroupBox* FullModelViewer::addFaceGroup() {
    auto faceGroup = new QGroupBox("Face");
    auto faceGroupLayout = new QVBoxLayout();
    faceGroup->setLayout(faceGroupLayout);

    auto faceRadio1 = new QRadioButton("Face 1");
    connect(faceRadio1, &QRadioButton::clicked, this, [=] {
        gearView->setFace(1);
    });
    faceGroupLayout->addWidget(faceRadio1);

    auto faceRadio2 = new QRadioButton("Face 2");
    connect(faceRadio2, &QRadioButton::clicked, this, [=] {
        gearView->setFace(2);
    });
    faceGroupLayout->addWidget(faceRadio2);

    auto faceRadio3 = new QRadioButton("Face 3");
    connect(faceRadio3, &QRadioButton::clicked, this, [=] {
        gearView->setFace(3);
    });
    faceGroupLayout->addWidget(faceRadio3);

    return faceGroup;
}

QGroupBox* FullModelViewer::addHairGroup() {
    auto hairGroup = new QGroupBox("Hair");
    auto hairGroupLayout = new QVBoxLayout();
    hairGroup->setLayout(hairGroupLayout);

    auto hairRadio1 = new QRadioButton("Hair 1");
    connect(hairRadio1, &QRadioButton::clicked, this, [=] {
        gearView->setHair(1);
    });
    hairGroupLayout->addWidget(hairRadio1);

    auto hairRadio2 = new QRadioButton("Hair 2");
    connect(hairRadio2, &QRadioButton::clicked, this, [=] {
        gearView->setHair(2);
    });
    hairGroupLayout->addWidget(hairRadio2);

    auto hairRadio3 = new QRadioButton("Hair 3");
    connect(hairRadio3, &QRadioButton::clicked, this, [=] {
        gearView->setHair(3);
    });
    hairGroupLayout->addWidget(hairRadio3);

    return hairGroup;
}

QGroupBox* FullModelViewer::addEarGroup() {
    auto earGroup = new QGroupBox("Ears");
    auto earGroupLayout = new QVBoxLayout();
    earGroup->setLayout(earGroupLayout);

    auto earRadio1 = new QRadioButton("Ears 1");
    connect(earRadio1, &QRadioButton::clicked, this, [=] {
        gearView->setEar(1);
    });
    earGroupLayout->addWidget(earRadio1);

    auto earRadio2 = new QRadioButton("Ears 2");
    connect(earRadio2, &QRadioButton::clicked, this, [=] {
        gearView->setEar(2);
    });
    earGroupLayout->addWidget(earRadio2);

    auto earRadio3 = new QRadioButton("Ears 3");
    connect(earRadio3, &QRadioButton::clicked, this, [=] {
        gearView->setEar(3);
    });
    earGroupLayout->addWidget(earRadio3);

    return earGroup;
}

QGroupBox* FullModelViewer::addTailGroup() {
    auto tailGroup = new QGroupBox("Tail");
    auto tailGroupLayout = new QVBoxLayout();
    tailGroup->setLayout(tailGroupLayout);

    auto tailRadio1 = new QRadioButton("Tail 1");
    connect(tailRadio1, &QRadioButton::clicked, this, [=] {
        gearView->setTail(1);
    });
    tailGroupLayout->addWidget(tailRadio1);

    auto tailRadio2 = new QRadioButton("Tail 2");
    connect(tailRadio2, &QRadioButton::clicked, this, [=] {
        gearView->setTail(2);
    });
    tailGroupLayout->addWidget(tailRadio2);

    auto tailRadio3 = new QRadioButton("Tail 3");
    connect(tailRadio3, &QRadioButton::clicked, this, [=] {
        gearView->setTail(3);
    });
    tailGroupLayout->addWidget(tailRadio3);

    return tailGroup;
}

#include "moc_fullmodelviewer.cpp"