// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QGroupBox>
#include <QMainWindow>

#include "boneeditor.h"
#include "gearview.h"

struct GameData;
class FileCache;

class FullModelViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit FullModelViewer(GameData *data, FileCache &cache, QWidget *parent = nullptr);

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

Q_SIGNALS:
    void gearChanged();
    void loadingChanged(bool loading);
    void visibleChanged();

public Q_SLOTS:
    void clear();
    void addGear(GearInfo &info);

private Q_SLOTS:
    void reloadGear();

private:
    void updateHeightScaling(float scale);
    void updateBustScaling(float scale);
    void updateCharacterParameters();
    void updateSupportedSubraces();
    void updateRaceData();

    QGroupBox *addFaceGroup();
    QGroupBox *addHairGroup();
    QGroupBox *addEarGroup();
    QGroupBox *addTailGroup();

    std::optional<GearInfo> topSlot;
    std::optional<GearInfo> bottomSlot;

    GearView *gearView = nullptr;
    QComboBox *raceCombo = nullptr, *subraceCombo = nullptr, *genderCombo = nullptr;

    GameData *data = nullptr;
    physis_CMP cmp{};

    BoneEditor *m_boneEditor;

    float heightScale = 0.5f;
    float bustScale = 1.0f;
};