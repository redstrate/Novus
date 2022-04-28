#pragma once

#include <QMainWindow>
#include <unordered_map>
#include <QComboBox>

#include "renderer.hpp"
#include "types/slot.h"
#include "types/race.h"
#include "havokxmlparser.h"

struct ModelInfo {
    int primaryID;
};

struct GearInfo {
    std::string name;
    Slot slot;
    ModelInfo modelInfo;
};

class GameData;
class VulkanWindow;
class StandaloneWindow;

class MainWindow : public QMainWindow {
public:
    MainWindow(GameData& data);

    void exportModel(Model& model, QString fileName);

private:
    void loadInitialGearInfo(GearInfo& info);
    void reloadGearModel();
    void reloadGearAppearance();

    std::vector<GearInfo> gears;

    struct LoadedGear {
        GearInfo* gearInfo;
        Model model;
        RenderModel renderModel;
    };

    LoadedGear loadedGear;

    QComboBox* raceCombo, *lodCombo;

    Race currentRace = Race::HyurMidlanderMale;
    int currentLod = 0;
    glm::vec3 currentScale = glm::vec3(1);
    Bone* currentEditedBone = nullptr;

    GameData& data;

    Renderer* renderer;
    VulkanWindow* vkWindow;
    StandaloneWindow* standaloneWindow;

    Skeleton skeleton;
};