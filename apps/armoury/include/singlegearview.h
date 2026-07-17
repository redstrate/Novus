// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"
#include "gearview.h"
#include <QPushButton>
#include <QWidget>

class SingleGearView : public QWidget
{
    Q_OBJECT

public:
    explicit SingleGearView(FileCache &cache, QWidget *parent = nullptr);

    QList<physis_Material> getLoadedMaterials() const;

Q_SIGNALS:
    void gearChanged(const QString &name);
    void gotMDLPath();

    void raceChanged();
    void subraceChanged();
    void genderChanged();
    void levelOfDetailChanged();

    void addToFullModelViewer(GearInfo &info);
    void importedModel();

    void doneLoadingModel();

public Q_SLOTS:
    void clear();
    void setGear(const GearInfo &info);

    void setRace(Race race);
    void setTribe(Tribe subrace);
    void setGender(Gender gender);
    void setLevelOfDetail(int lod);

    void setFMVAvailable(bool available);

private Q_SLOTS:
    void reloadGear() const;

private:
    void importModel(const QString &filename);

    FileCache &m_cache;
    std::optional<GearInfo> m_currentGear;

    Race m_currentRace = Race::Hyur;
    Tribe m_currentTribe = Tribe::Midlander;
    Gender m_currentGender = Gender::Male;
    int m_currentLod = 0;

    GearView *m_gearView = nullptr;
    QComboBox *m_raceCombo, *m_subraceCombo, *m_genderCombo, *m_lodCombo;
    QPushButton *m_addToFMVButton, *m_editButton, *m_importButton, *m_exportButton;

    bool m_fmvAvailable = false;
};
