// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singlegearview.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

#include "filecache.h"
#include "magic_enum.hpp"
#include "mdlimport.h"
#include "pathedit.h"

SingleGearView::SingleGearView(SqPackResource *data, FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , gearView(new GearView(data, cache))
    , data(data)
{
    gearView->setWhatsThis(i18n("A 3D preview of the gear model."));

    // We don't want to see the face in this view
    gearView->setHair(-1);
    gearView->setEar(-1);
    gearView->setFace(-1);

    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto mdlPathEdit = new PathEdit();
    mdlPathEdit->setWhatsThis(i18n("The path to this gear's model file."));
    mdlPathEdit->setReadOnly(true);

    connect(this, &SingleGearView::gotMDLPath, this, [this, mdlPathEdit] {
        mdlPathEdit->setPath(gearView->getLoadedGearPath());
        Q_EMIT doneLoadingModel();
    });

    auto topControlLayout = new QHBoxLayout();
    auto controlLayout = new QHBoxLayout();

    layout->addWidget(mdlPathEdit);
    layout->addLayout(controlLayout);
    layout->addWidget(gearView);
    layout->addLayout(topControlLayout);

    raceCombo = new QComboBox();
    raceCombo->setWhatsThis(i18n("The race used in the gear model preview. Note that this only shows races that have unique models for this gear."));
    connect(raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        setRace(static_cast<Race>(raceCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(raceCombo);

    subraceCombo = new QComboBox();
    subraceCombo->setWhatsThis(i18n("The subrace used in the gear model preview. Note that this only shows subraces that have unique models for this gear."));
    connect(subraceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        setTribe(static_cast<Tribe>(subraceCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(subraceCombo);

    genderCombo = new QComboBox();
    genderCombo->setWhatsThis(i18n("The gender used in the gear model preview. Note that this only shows gender that have unique models for this gear."));
    connect(genderCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        setGender(static_cast<Gender>(genderCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(genderCombo);

    lodCombo = new QComboBox();
    lodCombo->setWhatsThis(i18n("The level of detail to preview. The higher the number, the lower the detail."));
    connect(lodCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        setLevelOfDetail(index);
    });
    controlLayout->addWidget(lodCombo);

    addToFMVButton = new QPushButton(i18nc("@action:button FMV is an abbreviation for Full Model Viewer", "Add to FMV"));
    addToFMVButton->setWhatsThis(i18n("Add this gear to the Full Model Viewer window, to preview on a full character."));
    addToFMVButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add-user")));
    connect(addToFMVButton, &QPushButton::clicked, this, [this](bool) {
        if (currentGear.has_value()) {
            Q_EMIT addToFullModelViewer(*currentGear);
        }
    });

    editButton = new QPushButton(i18nc("@action:button", "Edit"));
    editButton->setWhatsThis(i18n("Edit this model directly in Blender."));
    editButton->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    connect(editButton, &QPushButton::clicked, this, [this](bool) {
        // Export in default location
        // TODO: deduplicate
        const auto sanitizeMdlPath = [](const QString &mdlPath) -> QString {
            return QString(mdlPath).section(QLatin1Char('/'), -1).remove(QStringLiteral(".mdl"));
        };

        KConfig config(QStringLiteral("novusrc"));
        KConfigGroup game = config.group(QStringLiteral("Armoury"));
        QString sourceDirectory = game.readEntry(QStringLiteral("SourcesOutputDirectory"));
        QString newFilename = QStringLiteral("%1.glb").arg(sanitizeMdlPath(gearView->getLoadedGearPath()));

        QString path = QStringLiteral("%1/%2/%3/%4")
                           .arg(sourceDirectory)
                           .arg(QString::fromStdString(magic_enum::enum_name(currentGear->slot).data()))
                           .arg(QString::fromStdString(currentGear->name))
                           .arg(QStringLiteral("3D"));

        if (!QDir().exists(path))
            QDir().mkpath(path);

        const QString fileName = QStringLiteral("%1/%2").arg(path, newFilename);

        gearView->exportModel(fileName);

        QFileInfo info(fileName);

        QProcess *blenderProcess = new QProcess(this);
        blenderProcess->setProgram(game.readEntry(QStringLiteral("BlenderPath")));
        blenderProcess->setArguments(
            {QStringLiteral("--python-expr"),
             QStringLiteral("import bpy\nbpy.ops.import_scene.gltf(filepath=\"%1\", files=[{\"name\":\"%2\", \"name\":\"%3\"}], bone_heuristic='TEMPERANCE')")
                 .arg(info.filePath(), info.fileName(), info.fileName())});
        blenderProcess->start();

        blenderProcess->waitForFinished();

        importButton->click();
    });
    topControlLayout->addWidget(editButton);

    importButton = new QPushButton(i18nc("@action:button", "Import…"));
    importButton->setWhatsThis(i18n("Import a different model for this gear."));
    importButton->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
    connect(importButton, &QPushButton::clicked, this, [this](bool) {
        if (currentGear.has_value()) {
            KConfig config(QStringLiteral("novusrc"));
            KConfigGroup game = config.group(QStringLiteral("Armoury"));
            QString sourceDirectory = game.readEntry(QStringLiteral("SourcesOutputDirectory"));

            // TODO: deduplicate
            QString path = QStringLiteral("%1/%2/%3/%4")
                               .arg(sourceDirectory)
                               .arg(QString::fromStdString(magic_enum::enum_name(currentGear->slot).data()))
                               .arg(QString::fromStdString(currentGear->name))
                               .arg(QStringLiteral("3D"));

            if (!QDir().exists(path))
                QDir().mkpath(path);

            const QString fileName = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Import Model"), path, i18n("glTF Binary File (*.glb)"));
            if (!fileName.isEmpty()) {
                importModel(fileName);
            }
        }
    });
    topControlLayout->addWidget(importButton);

    auto testMenu = new QMenu();
    auto gltfAction = testMenu->addAction(i18nc("@action:inmenu", "glTF"));
    connect(gltfAction, &QAction::triggered, this, [this](bool) {
        if (currentGear.has_value()) {
            // TODO: deduplicate
            const auto sanitizeMdlPath = [](const QString &mdlPath) -> QString {
                return QString(mdlPath).section(QLatin1Char('/'), -1).remove(QStringLiteral(".mdl"));
            };

            KConfig config(QStringLiteral("novusrc"));
            KConfigGroup game = config.group(QStringLiteral("Armoury"));
            QString sourceDirectory = game.readEntry(QStringLiteral("SourcesOutputDirectory"));
            QString newFilename = QStringLiteral("%1.glb").arg(sanitizeMdlPath(gearView->getLoadedGearPath()));

            QString path = QStringLiteral("%1/%2/%3/%4")
                               .arg(sourceDirectory)
                               .arg(QString::fromStdString(magic_enum::enum_name(currentGear->slot).data()))
                               .arg(QString::fromStdString(currentGear->name))
                               .arg(QStringLiteral("3D"));

            if (!QDir().exists(path))
                QDir().mkpath(path);

            const QString fileName =
                QFileDialog::getSaveFileName(this, tr("Export Model"), QStringLiteral("%1/%2").arg(path, newFilename), tr("glTF Binary File (*.glb)"));

            gearView->exportModel(fileName);
        }
    });
    auto mdlAction = testMenu->addAction(i18nc("@action:inmenu", "MDL"));
    connect(mdlAction, &QAction::triggered, this, [this, data](bool) {
        if (currentGear.has_value()) {
            // TODO: deduplicate
            const auto sanitizeMdlPath = [](const QString &mdlPath) -> QString {
                return QString(mdlPath).section(QLatin1Char('/'), -1);
            };

            const QString fileName = QFileDialog::getSaveFileName(this,
                                                                  i18nc("@title:window", "Export Model"),
                                                                  sanitizeMdlPath(gearView->getLoadedGearPath()),
                                                                  i18n("MDL File (*.mdl)"));

            auto buffer = physis_gamedata_extract_file(data, gearView->getLoadedGearPath().toStdString().c_str());

            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reinterpret_cast<char *>(buffer.data), buffer.size);
            } else {
                qFatal() << "Failed to write to" << fileName;
            }
        }
    });

    exportButton = new QPushButton(i18nc("@action:button", "Export"));
    exportButton->setMenu(testMenu);
    exportButton->setWhatsThis(i18n("Export this gear's model."));
    exportButton->setIcon(QIcon::fromTheme(QStringLiteral("document-export")));

    topControlLayout->addWidget(exportButton);
    topControlLayout->addWidget(addToFMVButton);

    connect(gearView, &GearView::loadingChanged, this, [this](const bool loading) {
        if (!loading) {
            reloadGear();
            Q_EMIT gotMDLPath();
        }
    });
    connect(this, &SingleGearView::raceChanged, this, [this] {
        gearView->setRace(currentRace);
    });
    connect(this, &SingleGearView::subraceChanged, this, [this] {
        gearView->setTribe(currentTribe);
    });
    connect(this, &SingleGearView::genderChanged, this, [this] {
        gearView->setGender(currentGender);
    });
    connect(this, &SingleGearView::levelOfDetailChanged, this, [this] {
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

void SingleGearView::setTribe(Tribe subrace)
{
    if (currentTribe == subrace) {
        return;
    }

    currentTribe = subrace;
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
    gearView->setEnabled(currentGear.has_value());
    raceCombo->setEnabled(currentGear.has_value());
    subraceCombo->setEnabled(currentGear.has_value());
    genderCombo->setEnabled(currentGear.has_value());
    lodCombo->setEnabled(currentGear.has_value());
    addToFMVButton->setEnabled(currentGear.has_value() && fmvAvailable);
    exportButton->setEnabled(currentGear.has_value());
    importButton->setEnabled(currentGear.has_value());
    editButton->setEnabled(currentGear.has_value());

    if (currentGear.has_value()) {
        QSignalBlocker raceBlocker(raceCombo);
        QSignalBlocker subraceBlocker(subraceCombo);
        QSignalBlocker genderBlocker(genderCombo);
        QSignalBlocker lodBlocker(lodCombo);

        const auto oldRace = static_cast<Race>(raceCombo->itemData(raceCombo->currentIndex()).toInt());
        const auto oldTribe = static_cast<Tribe>(subraceCombo->itemData(subraceCombo->currentIndex()).toInt());
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
                                   [oldTribe](auto p) {
                                       return std::get<1>(p) == oldTribe;
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
            lodCombo->addItem(i18nc("@action:inmenu LOD stands for Level of Detail", "LOD %1", i), i);
        }
        if (oldLod < gearView->lodCount()) {
            lodCombo->setCurrentIndex(oldLod);
        }
    }
}

void SingleGearView::setFMVAvailable(const bool available)
{
    if (fmvAvailable != available) {
        fmvAvailable = available;
        addToFMVButton->setEnabled(currentGear.has_value() && available);
    }
}

void SingleGearView::importModel(const QString &filename)
{
    auto &mdl = gearView->part().getModel(0);

    ::importModel(mdl.model, filename);

    gearView->part().reloadModel(0);

    const KConfig config(QStringLiteral("novusrc"));
    const KConfigGroup game = config.group(QStringLiteral("Armoury"));
    const QDir outputDirectory = game.readEntry(QStringLiteral("PenumbraOutputDirectory"));

    const QFileInfo info(outputDirectory.absoluteFilePath(gearView->getLoadedGearPath()));

    auto buffer = physis_mdl_write(&mdl.model);
    QFile file(info.absoluteFilePath());

    if (!QDir().exists(info.absolutePath()))
        QDir().mkpath(info.absolutePath());

    if (file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<char *>(buffer.data), buffer.size);
    } else {
        qFatal() << "Failed to write to" << info.absoluteFilePath();
    }

    Q_EMIT importedModel();
}

QList<physis_Material> SingleGearView::getLoadedMaterials() const
{
    QList<physis_Material> materialPaths;

    for (int i = 0; i < gearView->part().numModels(); i++) {
        const auto &model = gearView->part().getModel(i);
        for (const auto &material : model.materials) {
            materialPaths.push_back(material.mat);
        }
    }

    return materialPaths;
}

#include "moc_singlegearview.cpp"
