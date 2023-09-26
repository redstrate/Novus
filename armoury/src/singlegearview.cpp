// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singlegearview.h"

#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>

#include "filecache.h"
#include "magic_enum.hpp"

SingleGearView::SingleGearView(GameData* data, FileCache& cache) : data(data) {
    gearView = new GearView(data, cache);

    // We don't want to see the face in this view
    gearView->setHair(-1);
    gearView->setEar(-1);
    gearView->setFace(-1);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(gearView);
    setLayout(layout);

    auto controlLayout = new QHBoxLayout();
    layout->addLayout(controlLayout);

    raceCombo = new QComboBox();
    connect(raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        if (loadingComboData)
            return;

        setRace(static_cast<Race>(raceCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(raceCombo);

    subraceCombo = new QComboBox();
    connect(subraceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        if (loadingComboData)
            return;

        setSubrace(static_cast<Subrace>(raceCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(subraceCombo);

    genderCombo = new QComboBox();
    connect(genderCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        if (loadingComboData)
            return;

        setGender(static_cast<Gender>(genderCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(genderCombo);

    lodCombo = new QComboBox();
    connect(lodCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        if (loadingComboData)
            return;

        setLevelOfDetail(index);
    });
    controlLayout->addWidget(lodCombo);

    addToFMVButton = new QPushButton(QStringLiteral("Add to FMV"));
    connect(addToFMVButton, &QPushButton::clicked, this, [this](bool) {
        if (currentGear.has_value()) {
            Q_EMIT addToFullModelViewer(*currentGear);
        }
    });
    controlLayout->addWidget(addToFMVButton);

    exportButton = new QPushButton(QStringLiteral("Export..."));
    connect(exportButton, &QPushButton::clicked, this, [this](bool) {
        if (currentGear.has_value()) {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save Model"), QStringLiteral("model.glb"), tr("glTF Binary File (*.glb)"));

            gearView->exportModel(fileName);
        }
    });
    controlLayout->addWidget(exportButton);

    connect(this, &SingleGearView::gearChanged, this, &SingleGearView::reloadGear);
    connect(this, &SingleGearView::raceChanged, this, [=] {
        gearView->setRace(currentRace);
    });
    connect(this, &SingleGearView::subraceChanged, this, [=] {
        gearView->setSubrace(currentSubrace);
    });
    connect(this, &SingleGearView::genderChanged, this, [=] {
        gearView->setGender(currentGender);
    });
    connect(this, &SingleGearView::levelOfDetailChanged, this, [=] {
        gearView->setLevelOfDetail(currentLod);
    });

    reloadGear();
}

void SingleGearView::clear() {
    if (currentGear) {
        gearView->removeGear(*currentGear);
    }
    currentGear.reset();

    Q_EMIT gearChanged();
}

void SingleGearView::setGear(const GearInfo& info) {
    if (info != currentGear) {
        if (currentGear) {
            gearView->removeGear(*currentGear);
        }

        currentGear = info;
        gearView->addGear(*currentGear);

        Q_EMIT gearChanged();
    }
}

void SingleGearView::setRace(Race race) {
    if (currentRace == race) {
        return;
    }

    currentRace = race;
    Q_EMIT raceChanged();
}

void SingleGearView::setSubrace(Subrace subrace) {
    if (currentSubrace == subrace) {
        return;
    }

    currentSubrace = subrace;
    Q_EMIT subraceChanged();
}

void SingleGearView::setGender(Gender gender) {
    if (currentGender == gender) {
        return;
    }

    currentGender = gender;
    Q_EMIT genderChanged();
}

void SingleGearView::setLevelOfDetail(int lod) {
    if (currentLod == lod) {
        return;
    }

    currentLod = lod;
    Q_EMIT levelOfDetailChanged();
}

void SingleGearView::reloadGear()
{
    raceCombo->setEnabled(currentGear.has_value());
    subraceCombo->setEnabled(currentGear.has_value());
    genderCombo->setEnabled(currentGear.has_value());
    lodCombo->setEnabled(currentGear.has_value());
    addToFMVButton->setEnabled(currentGear.has_value() && fmvAvailable);
    exportButton->setEnabled(currentGear.has_value());

    if (currentGear.has_value()) {
        loadingComboData = true;

        const auto oldRace = static_cast<Race>(raceCombo->itemData(raceCombo->currentIndex()).toInt());
        const auto oldSubrace = static_cast<Subrace>(subraceCombo->itemData(subraceCombo->currentIndex()).toInt());
        const auto oldGender = static_cast<Gender>(genderCombo->itemData(genderCombo->currentIndex()).toInt());
        const auto oldLod = lodCombo->itemData(lodCombo->currentIndex()).toInt();

        raceCombo->clear();
        subraceCombo->clear();
        raceCombo->setCurrentIndex(0);
        subraceCombo->setCurrentIndex(0);

        const auto supportedRaces = gearView->supportedRaces();
        QList<Race> addedRaces;
        for (auto [race, subrace] : supportedRaces) {
            // TODO: supportedRaces should be designed better
            if (!addedRaces.contains(race)) {
                raceCombo->addItem(QLatin1String(magic_enum::enum_name(race).data(), static_cast<int>(race)));
                addedRaces.push_back(race);
            }
            subraceCombo->addItem(QLatin1String(magic_enum::enum_name(subrace).data(), static_cast<int>(subrace)));
        }

        if (auto it = std::find_if(supportedRaces.begin(), supportedRaces.end(), [oldRace](auto p) { return std::get<0>(p) == oldRace; }); it != supportedRaces.end()) {
            raceCombo->setCurrentIndex(std::distance(supportedRaces.begin(), it));
        }

        if (auto it = std::find_if(supportedRaces.begin(), supportedRaces.end(), [oldSubrace](auto p) { return std::get<1>(p) == oldSubrace; }); it != supportedRaces.end()) {
            subraceCombo->setCurrentIndex(std::distance(supportedRaces.begin(), it));
        }

        genderCombo->clear();
        genderCombo->setCurrentIndex(0);

        const auto supportedGenders = gearView->supportedGenders();
        for (auto gender : supportedGenders) {
            genderCombo->addItem(QLatin1String(magic_enum::enum_name(gender).data(), static_cast<int>(gender)));
        }

        if (auto it = std::find_if(supportedGenders.begin(), supportedGenders.end(), [oldGender](auto p) { return p == oldGender; }); it != supportedGenders.end()) {
            genderCombo->setCurrentIndex(std::distance(supportedGenders.begin(), it));
        }

        lodCombo->clear();
        for (int i = 0; i < gearView->lodCount(); i++) {
            lodCombo->addItem(QStringLiteral("LOD %1").arg(i), i);
        }

        loadingComboData = false;
    }
}

void SingleGearView::setFMVAvailable(const bool available)
{
    if (fmvAvailable != available) {
        fmvAvailable = available;
        addToFMVButton->setEnabled(currentGear.has_value() && available);
    }
}

#include "moc_singlegearview.cpp"