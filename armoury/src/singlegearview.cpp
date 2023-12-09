// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singlegearview.h"

#include <QDebug>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "filecache.h"
#include "magic_enum.hpp"
#include "mdlimport.h"

SingleGearView::SingleGearView(GameData *data, FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    gearView = new GearView(data, cache);

    // We don't want to see the face in this view
    gearView->setHair(-1);
    gearView->setEar(-1);
    gearView->setFace(-1);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    auto mdlPathEdit = new QLineEdit();
    mdlPathEdit->setReadOnly(true);

    connect(this, &SingleGearView::gotMDLPath, this, [this, mdlPathEdit] {
        mdlPathEdit->setText(gearView->getLoadedGearPath());
    });

    auto topControlLayout = new QHBoxLayout();
    auto controlLayout = new QHBoxLayout();

    layout->addWidget(mdlPathEdit);
    layout->addLayout(controlLayout);
    layout->addWidget(gearView);
    layout->addLayout(topControlLayout);

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

        setSubrace(static_cast<Subrace>(subraceCombo->itemData(index).toInt()));
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
    addToFMVButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add-user")));
    connect(addToFMVButton, &QPushButton::clicked, this, [this](bool) {
        if (currentGear.has_value()) {
            Q_EMIT addToFullModelViewer(*currentGear);
        }
    });

    importButton = new QPushButton(QStringLiteral("Import..."));
    importButton->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
    connect(importButton, &QPushButton::clicked, this, [this](bool) {
        if (currentGear.has_value()) {
            // TODO: deduplicate
            const auto sanitizeMdlPath = [](const QString &mdlPath) -> QString {
                return QString(mdlPath).section(QLatin1Char('/'), -1).remove(QStringLiteral(".mdl"));
            };

            const QString fileName = QFileDialog::getOpenFileName(this,
                                                                  tr("Import Model"),
                                                                  QStringLiteral("%1.glb").arg(sanitizeMdlPath(gearView->getLoadedGearPath())),
                                                                  tr("glTF Binary File (*.glb)"));

            importModel(fileName);
        }
    });
    topControlLayout->addWidget(importButton);

    exportButton = new QPushButton(QStringLiteral("Export..."));
    exportButton->setIcon(QIcon::fromTheme(QStringLiteral("document-export")));
    connect(exportButton, &QPushButton::clicked, this, [this](bool) {
        if (currentGear.has_value()) {
            // TODO: deduplicate
            const auto sanitizeMdlPath = [](const QString &mdlPath) -> QString {
                return QString(mdlPath).section(QLatin1Char('/'), -1).remove(QStringLiteral(".mdl"));
            };

            const QString fileName = QFileDialog::getSaveFileName(this,
                                                                  tr("Save Model"),
                                                                  QStringLiteral("%1.glb").arg(sanitizeMdlPath(gearView->getLoadedGearPath())),
                                                                  tr("glTF Binary File (*.glb)"));

            gearView->exportModel(fileName);
        }
    });
    topControlLayout->addWidget(exportButton);
    topControlLayout->addWidget(addToFMVButton);

    connect(gearView, &GearView::loadingChanged, this, [this](const bool loading) {
        if (!loading) {
            reloadGear();
            Q_EMIT gotMDLPath();
        }
    });
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

void SingleGearView::clear()
{
    if (currentGear) {
        gearView->removeGear(*currentGear);
    }
    currentGear.reset();

    Q_EMIT gearChanged();
}

void SingleGearView::setGear(const GearInfo &info)
{
    if (info != currentGear) {
        if (currentGear) {
            gearView->removeGear(*currentGear);
        }

        currentGear = info;
        gearView->addGear(*currentGear);

        Q_EMIT gearChanged();
    }
}

void SingleGearView::setRace(Race race)
{
    if (currentRace == race) {
        return;
    }

    currentRace = race;
    Q_EMIT raceChanged();
}

void SingleGearView::setSubrace(Subrace subrace)
{
    if (currentSubrace == subrace) {
        return;
    }

    qInfo() << "Setting subrace to" << magic_enum::enum_name(subrace);

    currentSubrace = subrace;
    Q_EMIT subraceChanged();
}

void SingleGearView::setGender(Gender gender)
{
    if (currentGender == gender) {
        return;
    }

    currentGender = gender;
    Q_EMIT genderChanged();
}

void SingleGearView::setLevelOfDetail(int lod)
{
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
                raceCombo->addItem(QLatin1String(magic_enum::enum_name(race).data()), static_cast<int>(race));
                addedRaces.push_back(race);
            }
        }

        if (auto it = std::find_if(supportedRaces.begin(),
                                   supportedRaces.end(),
                                   [oldRace](auto p) {
                                       return std::get<0>(p) == oldRace;
                                   });
            it != supportedRaces.end()) {
            raceCombo->setCurrentIndex(std::distance(supportedRaces.begin(), it));
        }

        const Race selectedRace = static_cast<Race>(raceCombo->currentData().toInt());
        for (auto [race, subrace] : supportedRaces) {
            if (race == selectedRace) {
                subraceCombo->addItem(QLatin1String(magic_enum::enum_name(subrace).data()), static_cast<int>(subrace));
            }
        }

        if (auto it = std::find_if(supportedRaces.begin(),
                                   supportedRaces.end(),
                                   [oldSubrace](auto p) {
                                       return std::get<1>(p) == oldSubrace;
                                   });
            it != supportedRaces.end()) {
            subraceCombo->setCurrentIndex(std::distance(supportedRaces.begin(), it));
        }

        genderCombo->clear();
        genderCombo->setCurrentIndex(0);

        const auto supportedGenders = gearView->supportedGenders();
        for (auto gender : supportedGenders) {
            genderCombo->addItem(QLatin1String(magic_enum::enum_name(gender).data()), static_cast<int>(gender));
        }

        if (auto it = std::find_if(supportedGenders.begin(),
                                   supportedGenders.end(),
                                   [oldGender](auto p) {
                                       return p == oldGender;
                                   });
            it != supportedGenders.end()) {
            genderCombo->setCurrentIndex(std::distance(supportedGenders.begin(), it));
        }

        lodCombo->clear();
        for (int i = 0; i < gearView->lodCount(); i++) {
            lodCombo->addItem(QStringLiteral("LOD %1").arg(i), i);
        }
        if (oldLod < gearView->lodCount()) {
            lodCombo->setCurrentIndex(oldLod);
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

QString SingleGearView::getLoadedGearPath() const
{
    return gearView->getLoadedGearPath();
}

void SingleGearView::importModel(const QString &filename)
{
    auto &mdl = gearView->part().getModel(0);

    ::importModel(mdl.model, filename);

    gearView->part().reloadModel(0);

    qInfo() << "Successfully imported model!";
}

#include "moc_singlegearview.cpp"