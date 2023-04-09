#pragma once

#include <QWidget>

#include "gearview.h"

struct GameData;

class FullModelViewer : public QWidget {
    Q_OBJECT
public:
    explicit FullModelViewer(GameData* data);

Q_SIGNALS:
    void gearChanged();

public Q_SLOTS:
    void clear();
    void addGear(GearInfo& info);

private Q_SLOTS:
    void reloadGear();

private:
    std::optional<GearInfo> topSlot;
    std::optional<GearInfo> bottomSlot;\

    GearView* gearView = nullptr;
    QComboBox* raceCombo, *genderCombo;

    GameData* data = nullptr;
};