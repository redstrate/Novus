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

SingleGearView::SingleGearView(FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , m_cache(cache)
    , m_gearView(new GearView(m_cache))
{
    m_gearView->setWhatsThis(i18n("A 3D preview of the gear model."));

    // We don't want to see the face in this view
    m_gearView->setHair(-1);
    m_gearView->setEar(-1);
    m_gearView->setFace(-1);

    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto mdlPathEdit = new PathEdit();
    mdlPathEdit->setWhatsThis(i18n("The path to this gear's model file."));
    mdlPathEdit->setReadOnly(true);

    connect(this, &SingleGearView::gotMDLPath, this, [this, mdlPathEdit] {
        mdlPathEdit->setPath(m_gearView->getLoadedGearPath());
        Q_EMIT doneLoadingModel();
    });

    auto topControlLayout = new QHBoxLayout();
    auto controlLayout = new QHBoxLayout();

    layout->addWidget(mdlPathEdit);
    layout->addLayout(controlLayout);
    layout->addWidget(m_gearView);
    layout->addLayout(topControlLayout);

    m_raceCombo = new QComboBox();
    m_raceCombo->setWhatsThis(i18n("The race used in the gear model preview. Note that this only shows races that have unique models for this gear."));
    connect(m_raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        setRace(static_cast<Race>(m_raceCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(m_raceCombo);

    m_subraceCombo = new QComboBox();
    m_subraceCombo->setWhatsThis(i18n("The subrace used in the gear model preview. Note that this only shows subraces that have unique models for this gear."));
    connect(m_subraceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        setTribe(static_cast<Tribe>(m_subraceCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(m_subraceCombo);

    m_genderCombo = new QComboBox();
    m_genderCombo->setWhatsThis(i18n("The gender used in the gear model preview. Note that this only shows gender that have unique models for this gear."));
    connect(m_genderCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        setGender(static_cast<Gender>(m_genderCombo->itemData(index).toInt()));
    });
    controlLayout->addWidget(m_genderCombo);

    m_lodCombo = new QComboBox();
    m_lodCombo->setWhatsThis(i18n("The level of detail to preview. The higher the number, the lower the detail."));
    connect(m_lodCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        setLevelOfDetail(index);
    });
    controlLayout->addWidget(m_lodCombo);

    m_addToFMVButton = new QPushButton(i18nc("@action:button FMV is an abbreviation for Full Model Viewer", "Add to FMV"));
    m_addToFMVButton->setWhatsThis(i18n("Add this gear to the Full Model Viewer window, to preview on a full character."));
    m_addToFMVButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add-user")));
    connect(m_addToFMVButton, &QPushButton::clicked, this, [this](bool) {
        if (m_currentGear.has_value()) {
            Q_EMIT addToFullModelViewer(*m_currentGear);
        }
    });

    m_editButton = new QPushButton(i18nc("@action:button", "Edit"));
    m_editButton->setWhatsThis(i18n("Edit this model directly in Blender."));
    m_editButton->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    connect(m_editButton, &QPushButton::clicked, this, [this](bool) {
        // Export in default location
        // TODO: deduplicate
        const auto sanitizeMdlPath = [](const QString &mdlPath) -> QString {
            return QString(mdlPath).section(QLatin1Char('/'), -1).remove(QStringLiteral(".mdl"));
        };

        KConfig config(QStringLiteral("novusrc"));
        KConfigGroup game = config.group(QStringLiteral("Armoury"));
        QString sourceDirectory = game.readEntry(QStringLiteral("SourcesOutputDirectory"));
        QString newFilename = QStringLiteral("%1.glb").arg(sanitizeMdlPath(m_gearView->getLoadedGearPath()));

        QString path = QStringLiteral("%1/%2/%3/%4")
                           .arg(sourceDirectory)
                           .arg(QString::fromStdString(magic_enum::enum_name(m_currentGear->slot).data()))
                           .arg(m_currentGear->name)
                           .arg(QStringLiteral("3D"));

        if (!QDir().exists(path))
            QDir().mkpath(path);

        const QString fileName = QStringLiteral("%1/%2").arg(path, newFilename);

        m_gearView->exportModel(fileName);

        QFileInfo info(fileName);

        QProcess *blenderProcess = new QProcess(this);
        blenderProcess->setProgram(game.readEntry(QStringLiteral("BlenderPath")));
        blenderProcess->setArguments(
            {QStringLiteral("--python-expr"),
             QStringLiteral("import bpy\nbpy.ops.import_scene.gltf(filepath=\"%1\", files=[{\"name\":\"%2\", \"name\":\"%3\"}], bone_heuristic='TEMPERANCE')")
                 .arg(info.filePath(), info.fileName(), info.fileName())});
        blenderProcess->start();

        blenderProcess->waitForFinished();

        m_importButton->click();
    });
    topControlLayout->addWidget(m_editButton);

    m_importButton = new QPushButton(i18nc("@action:button", "Import…"));
    m_importButton->setWhatsThis(i18n("Import a different model for this gear."));
    m_importButton->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
    connect(m_importButton, &QPushButton::clicked, this, [this](bool) {
        if (m_currentGear.has_value()) {
            KConfig config(QStringLiteral("novusrc"));
            KConfigGroup game = config.group(QStringLiteral("Armoury"));
            QString sourceDirectory = game.readEntry(QStringLiteral("SourcesOutputDirectory"));

            // TODO: deduplicate
            QString path = QStringLiteral("%1/%2/%3/%4")
                               .arg(sourceDirectory)
                               .arg(QString::fromStdString(magic_enum::enum_name(m_currentGear->slot).data()))
                               .arg(m_currentGear->name)
                               .arg(QStringLiteral("3D"));

            if (!QDir().exists(path))
                QDir().mkpath(path);

            const QString fileName = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Import Model"), path, i18n("glTF Binary File (*.glb)"));
            if (!fileName.isEmpty()) {
                importModel(fileName);
            }
        }
    });
    topControlLayout->addWidget(m_importButton);

    auto testMenu = new QMenu();
    auto gltfAction = testMenu->addAction(i18nc("@action:inmenu", "glTF"));
    connect(gltfAction, &QAction::triggered, this, [this](bool) {
        if (m_currentGear.has_value()) {
            // TODO: deduplicate
            const auto sanitizeMdlPath = [](const QString &mdlPath) -> QString {
                return QString(mdlPath).section(QLatin1Char('/'), -1).remove(QStringLiteral(".mdl"));
            };

            KConfig config(QStringLiteral("novusrc"));
            KConfigGroup game = config.group(QStringLiteral("Armoury"));
            QString sourceDirectory = game.readEntry(QStringLiteral("SourcesOutputDirectory"));
            QString newFilename = QStringLiteral("%1.glb").arg(sanitizeMdlPath(m_gearView->getLoadedGearPath()));

            QString path = QStringLiteral("%1/%2/%3/%4")
                               .arg(sourceDirectory)
                               .arg(QString::fromStdString(magic_enum::enum_name(m_currentGear->slot).data()))
                               .arg(m_currentGear->name)
                               .arg(QStringLiteral("3D"));

            if (!QDir().exists(path))
                QDir().mkpath(path);

            const QString fileName =
                QFileDialog::getSaveFileName(this, tr("Export Model"), QStringLiteral("%1/%2").arg(path, newFilename), tr("glTF Binary File (*.glb)"));

            m_gearView->exportModel(fileName);
        }
    });
    auto mdlAction = testMenu->addAction(i18nc("@action:inmenu", "MDL"));
    connect(mdlAction, &QAction::triggered, this, [this, &cache](bool) {
        if (m_currentGear.has_value()) {
            // TODO: deduplicate
            const auto sanitizeMdlPath = [](const QString &mdlPath) -> QString {
                return QString(mdlPath).section(QLatin1Char('/'), -1);
            };

            const QString fileName = QFileDialog::getSaveFileName(this,
                                                                  i18nc("@title:window", "Export Model"),
                                                                  sanitizeMdlPath(m_gearView->getLoadedGearPath()),
                                                                  i18n("MDL File (*.mdl)"));

            auto buffer = m_cache.read(m_gearView->getLoadedGearPath());

            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reinterpret_cast<char *>(buffer.data), buffer.size);
            } else {
                qFatal() << "Failed to write to" << fileName;
            }
        }
    });

    m_exportButton = new QPushButton(i18nc("@action:button", "Export"));
    m_exportButton->setMenu(testMenu);
    m_exportButton->setWhatsThis(i18n("Export this gear's model."));
    m_exportButton->setIcon(QIcon::fromTheme(QStringLiteral("document-export")));

    topControlLayout->addWidget(m_exportButton);
    topControlLayout->addWidget(m_addToFMVButton);

    connect(m_gearView, &GearView::loadingChanged, this, [this](const bool loading) {
        if (!loading) {
            reloadGear();
            Q_EMIT gotMDLPath();
        }
    });
    connect(this, &SingleGearView::raceChanged, this, [this] {
        m_gearView->setRace(m_currentRace);
    });
    connect(this, &SingleGearView::subraceChanged, this, [this] {
        m_gearView->setTribe(m_currentTribe);
    });
    connect(this, &SingleGearView::genderChanged, this, [this] {
        m_gearView->setGender(m_currentGender);
    });
    connect(this, &SingleGearView::levelOfDetailChanged, this, [this] {
        m_gearView->setLevelOfDetail(m_currentLod);
    });

    reloadGear();
}

void SingleGearView::clear()
{
    if (m_currentGear) {
        m_gearView->removeGear(*m_currentGear);
    }
    m_currentGear.reset();

    Q_EMIT gearChanged(QString());
}

void SingleGearView::setGear(const GearInfo &info)
{
    if (info != m_currentGear) {
        if (m_currentGear) {
            m_gearView->removeGear(*m_currentGear);
        }

        m_currentGear = info;
        m_gearView->addGear(*m_currentGear);

        Q_EMIT gearChanged(info.name);
    }
}

void SingleGearView::setRace(Race race)
{
    if (m_currentRace == race) {
        return;
    }

    m_currentRace = race;
    Q_EMIT raceChanged();
}

void SingleGearView::setTribe(Tribe subrace)
{
    if (m_currentTribe == subrace) {
        return;
    }

    m_currentTribe = subrace;
    Q_EMIT subraceChanged();
}

void SingleGearView::setGender(Gender gender)
{
    if (m_currentGender == gender) {
        return;
    }

    m_currentGender = gender;
    Q_EMIT genderChanged();
}

void SingleGearView::setLevelOfDetail(int lod)
{
    if (m_currentLod == lod) {
        return;
    }

    m_currentLod = lod;
    Q_EMIT levelOfDetailChanged();
}

void SingleGearView::reloadGear()
{
    m_gearView->setEnabled(m_currentGear.has_value());
    m_raceCombo->setEnabled(m_currentGear.has_value());
    m_subraceCombo->setEnabled(m_currentGear.has_value());
    m_genderCombo->setEnabled(m_currentGear.has_value());
    m_lodCombo->setEnabled(m_currentGear.has_value());
    m_addToFMVButton->setEnabled(m_currentGear.has_value() && m_fmvAvailable);
    m_exportButton->setEnabled(m_currentGear.has_value());
    m_importButton->setEnabled(m_currentGear.has_value());
    m_editButton->setEnabled(m_currentGear.has_value());

    if (m_currentGear.has_value()) {
        QSignalBlocker raceBlocker(m_raceCombo);
        QSignalBlocker subraceBlocker(m_subraceCombo);
        QSignalBlocker genderBlocker(m_genderCombo);
        QSignalBlocker lodBlocker(m_lodCombo);

        const auto oldRace = static_cast<Race>(m_raceCombo->itemData(m_raceCombo->currentIndex()).toInt());
        const auto oldTribe = static_cast<Tribe>(m_subraceCombo->itemData(m_subraceCombo->currentIndex()).toInt());
        const auto oldGender = static_cast<Gender>(m_genderCombo->itemData(m_genderCombo->currentIndex()).toInt());
        const auto oldLod = m_lodCombo->itemData(m_lodCombo->currentIndex()).toInt();

        m_raceCombo->clear();
        m_subraceCombo->clear();
        m_raceCombo->setCurrentIndex(0);
        m_subraceCombo->setCurrentIndex(0);

        const auto supportedRaces = m_gearView->supportedRaces();
        QList<Race> addedRaces;
        for (auto [race, subrace] : supportedRaces) {
            // TODO: supportedRaces should be designed better
            if (!addedRaces.contains(race)) {
                m_raceCombo->addItem(QLatin1String(magic_enum::enum_name(race).data()), static_cast<int>(race));
                addedRaces.push_back(race);
            }
        }

        if (auto it = std::find_if(supportedRaces.begin(),
                                   supportedRaces.end(),
                                   [oldRace](auto p) {
                                       return std::get<0>(p) == oldRace;
                                   });
            it != supportedRaces.end()) {
            m_raceCombo->setCurrentIndex(std::distance(supportedRaces.begin(), it));
        }

        const Race selectedRace = static_cast<Race>(m_raceCombo->currentData().toInt());
        for (auto [race, subrace] : supportedRaces) {
            if (race == selectedRace) {
                m_subraceCombo->addItem(QLatin1String(magic_enum::enum_name(subrace).data()), static_cast<int>(subrace));
            }
        }

        if (auto it = std::find_if(supportedRaces.begin(),
                                   supportedRaces.end(),
                                   [oldTribe](auto p) {
                                       return std::get<1>(p) == oldTribe;
                                   });
            it != supportedRaces.end()) {
            m_subraceCombo->setCurrentIndex(std::distance(supportedRaces.begin(), it));
        }

        m_genderCombo->clear();
        m_genderCombo->setCurrentIndex(0);

        const auto supportedGenders = m_gearView->supportedGenders();
        for (auto gender : supportedGenders) {
            m_genderCombo->addItem(QLatin1String(magic_enum::enum_name(gender).data()), static_cast<int>(gender));
        }

        if (auto it = std::find_if(supportedGenders.begin(),
                                   supportedGenders.end(),
                                   [oldGender](auto p) {
                                       return p == oldGender;
                                   });
            it != supportedGenders.end()) {
            m_genderCombo->setCurrentIndex(std::distance(supportedGenders.begin(), it));
        }

        m_lodCombo->clear();
        for (int i = 0; i < m_gearView->lodCount(); i++) {
            m_lodCombo->addItem(i18nc("@action:inmenu LOD stands for Level of Detail", "LOD %1", i), i);
        }
        if (oldLod < m_gearView->lodCount()) {
            m_lodCombo->setCurrentIndex(oldLod);
        }
    }
}

void SingleGearView::setFMVAvailable(const bool available)
{
    if (m_fmvAvailable != available) {
        m_fmvAvailable = available;
        m_addToFMVButton->setEnabled(m_currentGear.has_value() && available);
    }
}

void SingleGearView::importModel(const QString &filename)
{
    auto &mdl = m_gearView->part().getModel(0);

    ::importModel(mdl.model, filename);

    m_gearView->part().reloadModel(0);

    const KConfig config(QStringLiteral("novusrc"));
    const KConfigGroup game = config.group(QStringLiteral("Armoury"));
    const QDir outputDirectory = game.readEntry(QStringLiteral("PenumbraOutputDirectory"));

    const QFileInfo info(outputDirectory.absoluteFilePath(m_gearView->getLoadedGearPath()));

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

    for (int i = 0; i < m_gearView->part().numModels(); i++) {
        const auto &model = m_gearView->part().getModel(i);
        for (const auto &material : model.materials) {
            materialPaths.push_back(material.mat);
        }
    }

    return materialPaths;
}

#include "moc_singlegearview.cpp"
