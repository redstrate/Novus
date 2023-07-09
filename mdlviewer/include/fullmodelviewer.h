#pragma once

#include <QWidget>
#include <QGroupBox>

#include "gearview.h"

struct GameData;
class FileCache;

class FullModelViewer : public QWidget {
    Q_OBJECT
public:
    explicit FullModelViewer(GameData* data, FileCache& cache);

Q_SIGNALS:
    void gearChanged();

public Q_SLOTS:
    void clear();
    void addGear(GearInfo& info);

private Q_SLOTS:
    void reloadGear();

private:
    void updateHeightScaling(float scale);
    void updateBustScaling(float scale);
    void updateCharacterParameters();
    void updateSupportedSubraces();

    QGroupBox* addFaceGroup();
    QGroupBox* addHairGroup();
    QGroupBox* addEarGroup();
    QGroupBox* addTailGroup();

    std::optional<GearInfo> topSlot;
    std::optional<GearInfo> bottomSlot;

    GearView* gearView = nullptr;
    QComboBox *raceCombo, *subraceCombo, *genderCombo;

    GameData* data = nullptr;
    physis_CMP cmp;

    float heightScale = 0.5f;
    float bustScale = 1.0f;
};