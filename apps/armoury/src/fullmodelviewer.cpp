// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fullmodelviewer.h"

#include <KLocalizedString>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QRadioButton>
#include <QVBoxLayout>

#include "boneeditor.h"
#include "magic_enum.hpp"
#include "settings.h"

FullModelViewer::FullModelViewer(FileCache &cache, QWidget *parent)
    : QMainWindow(parent)
    , m_cache(cache)
{
    setWindowTitle(i18nc("@title:window", "Full Model Viewer"));
    setMinimumWidth(1280);
    setMinimumHeight(720);
    setAttribute(Qt::WA_DeleteOnClose, false);

    const auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    const auto layout = new QVBoxLayout();
    dummyWidget->setLayout(layout);

    const auto fileMenu = menuBar()->addMenu(i18nc("@title:menu", "File"));

    const auto datOpenAction = fileMenu->addAction(i18nc("@action:inmenu DAT is an abbreviation", "Load Character DAT…"));
    datOpenAction->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(datOpenAction, &QAction::triggered, [this] {
        const auto fileName = getOpenFileName(this,
                                              QStringLiteral("ArmouryCharacterDatFile"),
                                              i18nc("@title:window DAT is an abbreviation", "Open DAT File"),
                                              {},
                                              i18nc("DAT is an abbreviation", "FFXIV Character DAT File (*.dat)"));

        const auto buffer = physis_read_file(fileName.toStdString().c_str());

        const auto charDat = physis_chardat_parse(buffer);

        m_gearView->setRace(charDat.customize.race);
        m_gearView->setGender(charDat.customize.gender);
        // gearView->setTribe(charDat.subrace);
        m_gearView->setFace(charDat.customize.face);
        m_gearView->setHair(charDat.customize.hair);
        updateBustScaling(static_cast<float>(charDat.customize.bust) / 100.0f);
        updateHeightScaling(static_cast<float>(charDat.customize.height) / 100.0f);
    });

    m_cmp = physis_cmp_parse(m_cache.platform(), m_cache.read(QStringLiteral("chara/xls/charamake/human.cmp")));

    m_gearView = new GearView(m_cache);

    connect(m_gearView, &GearView::modelReloaded, this, &FullModelViewer::updateCharacterParameters);
    connect(m_gearView, &GearView::raceChanged, this, &FullModelViewer::updateRaceData);
    connect(m_gearView, &GearView::subraceChanged, this, &FullModelViewer::updateRaceData);
    connect(m_gearView, &GearView::genderChanged, this, &FullModelViewer::updateRaceData);

    const auto viewportLayout = new QHBoxLayout();
    viewportLayout->setContentsMargins(0, 0, 0, 0);
    viewportLayout->addWidget(m_gearView, 1);
    layout->addLayout(viewportLayout);

    const auto characterEditorWidget = new QWidget();
    const auto characterEditorLayout = new QFormLayout();
    characterEditorWidget->setLayout(characterEditorLayout);

    const auto characterHeight = new QSlider();
    characterHeight->setOrientation(Qt::Horizontal);
    characterHeight->setSliderPosition(50);
    connect(characterHeight, &QSlider::sliderMoved, this, [this](const int position) {
        const float scale = static_cast<float>(position) / 100.0f;
        updateHeightScaling(scale);
    });
    characterEditorLayout->addRow(i18nc("@label:slider Character height", "Height"), characterHeight);

    const auto bustSize = new QSlider();
    bustSize->setOrientation(Qt::Horizontal);
    bustSize->setSliderPosition(50);
    connect(bustSize, &QSlider::sliderMoved, this, [this](const int position) {
        const float scale = static_cast<float>(position) / 100.0f;
        updateBustScaling(scale);
    });
    characterEditorLayout->addRow(i18nc("@label:slider Character breast size", "Bust Size"), bustSize);

    characterEditorLayout->addWidget(addFaceGroup());
    characterEditorLayout->addWidget(addHairGroup());
    characterEditorLayout->addWidget(addEarGroup());
    characterEditorLayout->addWidget(addTailGroup());

    m_boneEditor = new BoneEditor(m_gearView);

    const auto debugWidget = new QWidget();
    const auto debugLayout = new QFormLayout();
    debugWidget->setLayout(debugLayout);

    const auto racialTransformsBox = new QCheckBox();
    racialTransformsBox->setChecked(true);
    connect(racialTransformsBox, &QCheckBox::clicked, this, [this](const bool checked) {
        m_gearView->part().enableRacialDeform = checked;
        m_gearView->part().reloadRenderer();
    });
    debugLayout->addRow(i18n("Enable Racial Deforms"), racialTransformsBox);

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(m_boneEditor, i18nc("@title:tab", "Bone Editor"));
    tabWidget->addTab(characterEditorWidget, i18nc("@title:tab", "Character Editor"));
    tabWidget->addTab(debugWidget, i18nc("@title:tab", "Debug"));
    viewportLayout->addWidget(tabWidget);

    const auto controlLayout = new QHBoxLayout();
    layout->addLayout(controlLayout);

    m_raceCombo = new QComboBox();
    controlLayout->addWidget(m_raceCombo);

    for (auto [race, race_name] : magic_enum::enum_entries<Race>()) {
        m_raceCombo->addItem(QLatin1String(race_name.data()), static_cast<int>(race));
    }

    m_subraceCombo = new QComboBox();
    connect(m_subraceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](const int index) {
        m_gearView->setTribe(static_cast<Tribe>(m_subraceCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(m_subraceCombo);

    connect(m_raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](const int index) {
        m_gearView->setRace(static_cast<Race>(m_raceCombo->itemData(index).toInt()));

        updateSupportedTribes();
    });
    updateSupportedTribes();

    m_genderCombo = new QComboBox();
    connect(m_genderCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](const int index) {
        m_gearView->setGender(static_cast<Gender>(m_genderCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(m_genderCombo);

    for (auto [gender, gender_name] : magic_enum::enum_entries<Gender>()) {
        m_genderCombo->addItem(QLatin1String(gender_name.data()), static_cast<int>(gender));
    }

    connect(this, &FullModelViewer::gearChanged, this, &FullModelViewer::reloadGear);
    connect(m_gearView, &GearView::loadingChanged, this, &FullModelViewer::loadingChanged);
    connect(this, &FullModelViewer::loadingChanged, this, [this, tabWidget](const bool loading) {
        m_raceCombo->setEnabled(!loading);
        m_subraceCombo->setEnabled(!loading);
        m_genderCombo->setEnabled(!loading);
        tabWidget->setEnabled(!loading);
    });

    updateCharacterParameters();
    updateRaceData();
    reloadGear();
}

void FullModelViewer::showEvent(QShowEvent *event)
{
    Q_EMIT visibleChanged();
    QWidget::showEvent(event);
}

void FullModelViewer::hideEvent(QHideEvent *event)
{
    Q_EMIT visibleChanged();
    QWidget::hideEvent(event);
}

void FullModelViewer::closeEvent(QCloseEvent *event)
{
    event->setAccepted(true);
    hide();
    QWidget::closeEvent(event);
}

void FullModelViewer::clear()
{
    m_topSlot.reset();
    m_bottomSlot.reset();

    Q_EMIT gearChanged();
}

void FullModelViewer::addGear(GearInfo &info)
{
    switch (info.slot) {
    case EquipSlotCategory::Body:
        if (!m_topSlot || *m_topSlot != info) {
            m_topSlot = info;
        }
        break;
    case EquipSlotCategory::Legs:
        if (!m_bottomSlot || *m_bottomSlot != info) {
            m_bottomSlot = info;
        }
        break;
    default:
        break;
    }

    Q_EMIT gearChanged();
}

void FullModelViewer::reloadGear()
{
    if (m_topSlot.has_value()) {
        m_gearView->addGear(*m_topSlot);
    } else {
        // smallclothes body
        GearInfo info = {};
        info.name = i18n("SmallClothes Body");
        info.slot = EquipSlotCategory::Body;

        m_gearView->addGear(info);
    }

    if (m_bottomSlot.has_value()) {
        m_gearView->addGear(*m_bottomSlot);
    } else {
        // smallclothes legs
        GearInfo info = {};
        info.name = i18n("SmallClothes Legs");
        info.slot = EquipSlotCategory::Legs;

        m_gearView->addGear(info);
    }

    // smallclothes hands
    {
        GearInfo info = {};
        info.name = i18n("SmallClothes Hands");
        info.slot = EquipSlotCategory::Hands;

        m_gearView->addGear(info);
    }

    // smallclothes hands
    {
        GearInfo info = {};
        info.name = i18n("SmallClothes Feet");
        info.slot = EquipSlotCategory::Feet;

        m_gearView->addGear(info);
    }
}

void FullModelViewer::updateHeightScaling(const float scale)
{
    const auto &boneData = *m_gearView->part().skeleton;
    for (uint32_t i = 0; i < boneData.num_bones; i++) {
        const std::string_view name{boneData.bones[i].name};
        if (name == "n_root") {
            auto racialScaling = physis_cmp_get_racial_scaling_parameters(m_cmp, m_gearView->currentRace, m_gearView->currentTribe);

            const float minSize = m_gearView->currentGender == Gender::Male ? racialScaling.male_min_size : racialScaling.female_min_size;
            const float maxSize = m_gearView->currentGender == Gender::Male ? racialScaling.male_max_size : racialScaling.female_max_size;

            const float size = glm::mix(minSize, maxSize, scale);

            boneData.bones[i].scale[0] = size;
            boneData.bones[i].scale[1] = size;
            boneData.bones[i].scale[2] = size;

            m_gearView->part().reloadRenderer();
        }
    }

    m_heightScale = scale;
}

void FullModelViewer::updateBustScaling(const float scale)
{
    const auto &boneData = *m_gearView->part().skeleton;
    for (uint32_t i = 0; i < boneData.num_bones; i++) {
        const std::string_view name{boneData.bones[i].name};
        if (name == "j_mune_l" || name == "j_mune_r") {
            const auto racialScaling = physis_cmp_get_racial_scaling_parameters(m_cmp, m_gearView->currentRace, m_gearView->currentTribe);

            const float rangeX = glm::mix(racialScaling.bust_min_x, racialScaling.bust_max_x, scale);
            const float rangeY = glm::mix(racialScaling.bust_min_y, racialScaling.bust_max_y, scale);
            const float rangeZ = glm::mix(racialScaling.bust_min_z, racialScaling.bust_max_z, scale);

            boneData.bones[i].scale[0] = rangeX;
            boneData.bones[i].scale[1] = rangeY;
            boneData.bones[i].scale[2] = rangeZ;

            m_gearView->part().reloadRenderer();
        }
    }

    m_bustScale = scale;
}

void FullModelViewer::updateCharacterParameters()
{
    updateHeightScaling(m_heightScale);
    updateBustScaling(m_bustScale);
}

void FullModelViewer::updateSupportedTribes() const
{
    m_subraceCombo->clear();
    for (auto subrace : physis_get_supported_tribes(m_gearView->currentRace).subraces) {
        m_subraceCombo->addItem(QLatin1String(magic_enum::enum_name(subrace).data()), static_cast<int>(subrace));
    }
}

void FullModelViewer::updateRaceData() const
{
    m_boneEditor->load_pbd(m_gearView->part().pbd,
                           physis_get_race_code(Race::Hyur, Tribe::Midlander, m_gearView->currentGender),
                           physis_get_race_code(m_gearView->currentRace, m_gearView->currentTribe, m_gearView->currentGender));
}

QGroupBox *FullModelViewer::addFaceGroup()
{
    const auto faceGroup = new QGroupBox(i18nc("@title:group", "Face"));
    const auto faceGroupLayout = new QVBoxLayout();
    faceGroup->setLayout(faceGroupLayout);

    const auto faceRadio1 = new QRadioButton(i18nc("@option:radio", "Face 1"));
    connect(faceRadio1, &QRadioButton::clicked, this, [this] {
        m_gearView->setFace(1);
    });
    faceGroupLayout->addWidget(faceRadio1);

    const auto faceRadio2 = new QRadioButton(i18nc("@option:radio", "Face 2"));
    connect(faceRadio2, &QRadioButton::clicked, this, [this] {
        m_gearView->setFace(2);
    });
    faceGroupLayout->addWidget(faceRadio2);

    const auto faceRadio3 = new QRadioButton(i18nc("@option:radio", "Face 3"));
    connect(faceRadio3, &QRadioButton::clicked, this, [this] {
        m_gearView->setFace(3);
    });
    faceGroupLayout->addWidget(faceRadio3);

    return faceGroup;
}

QGroupBox *FullModelViewer::addHairGroup()
{
    const auto hairGroup = new QGroupBox(i18nc("@title:group", "Hair"));
    const auto hairGroupLayout = new QVBoxLayout();
    hairGroup->setLayout(hairGroupLayout);

    const auto hairRadio1 = new QRadioButton(i18nc("@option:radio", "Hair 1"));
    connect(hairRadio1, &QRadioButton::clicked, this, [this] {
        m_gearView->setHair(1);
    });
    hairGroupLayout->addWidget(hairRadio1);

    const auto hairRadio2 = new QRadioButton(i18nc("@option:radio", "Hair 2"));
    connect(hairRadio2, &QRadioButton::clicked, this, [this] {
        m_gearView->setHair(2);
    });
    hairGroupLayout->addWidget(hairRadio2);

    const auto hairRadio3 = new QRadioButton(i18nc("@option:radio", "Hair 3"));
    connect(hairRadio3, &QRadioButton::clicked, this, [this] {
        m_gearView->setHair(3);
    });
    hairGroupLayout->addWidget(hairRadio3);

    return hairGroup;
}

QGroupBox *FullModelViewer::addEarGroup()
{
    const auto earGroup = new QGroupBox(i18nc("@title:group", "Ears"));
    const auto earGroupLayout = new QVBoxLayout();
    earGroup->setLayout(earGroupLayout);

    const auto earRadio1 = new QRadioButton(i18nc("@option:radio", "Ears 1"));
    connect(earRadio1, &QRadioButton::clicked, this, [this] {
        m_gearView->setEar(1);
    });
    earGroupLayout->addWidget(earRadio1);

    const auto earRadio2 = new QRadioButton(i18nc("@option:radio", "Ears 2"));
    connect(earRadio2, &QRadioButton::clicked, this, [this] {
        m_gearView->setEar(2);
    });
    earGroupLayout->addWidget(earRadio2);

    const auto earRadio3 = new QRadioButton(i18nc("@option:radio", "Ears 3"));
    connect(earRadio3, &QRadioButton::clicked, this, [this] {
        m_gearView->setEar(3);
    });
    earGroupLayout->addWidget(earRadio3);

    return earGroup;
}

QGroupBox *FullModelViewer::addTailGroup()
{
    const auto tailGroup = new QGroupBox(i18nc("@title:group", "Tail"));
    const auto tailGroupLayout = new QVBoxLayout();
    tailGroup->setLayout(tailGroupLayout);

    const auto tailRadio1 = new QRadioButton(i18nc("@option:radio", "Tail 1"));
    connect(tailRadio1, &QRadioButton::clicked, this, [this] {
        m_gearView->setTail(1);
    });
    tailGroupLayout->addWidget(tailRadio1);

    const auto tailRadio2 = new QRadioButton(i18nc("@option:radio", "Tail 2"));
    connect(tailRadio2, &QRadioButton::clicked, this, [this] {
        m_gearView->setTail(2);
    });
    tailGroupLayout->addWidget(tailRadio2);

    const auto tailRadio3 = new QRadioButton(i18nc("@option:radio", "Tail 3"));
    connect(tailRadio3, &QRadioButton::clicked, this, [this] {
        m_gearView->setTail(3);
    });
    tailGroupLayout->addWidget(tailRadio3);

    return tailGroup;
}

#include "moc_fullmodelviewer.cpp"
