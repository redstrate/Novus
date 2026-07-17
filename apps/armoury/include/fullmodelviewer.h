// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMainWindow>

#include "gearview.h"

struct SqPackResource;
class FileCache;
class QGroupBox;
class BoneEditor;

class FullModelViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit FullModelViewer(FileCache &cache, QWidget *parent = nullptr);

protected:
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
    void updateSupportedTribes() const;
    void updateRaceData() const;

    QGroupBox *addFaceGroup();
    QGroupBox *addHairGroup();
    QGroupBox *addEarGroup();
    QGroupBox *addTailGroup();

    std::optional<GearInfo> m_topSlot;
    std::optional<GearInfo> m_bottomSlot;

    GearView *m_gearView = nullptr;
    QComboBox *m_raceCombo = nullptr, *m_subraceCombo = nullptr, *m_genderCombo = nullptr;

    FileCache &m_cache;
    physis_CMP m_cmp{};

    BoneEditor *m_boneEditor;

    float m_heightScale = 0.5f;
    float m_bustScale = 1.0f;
};
