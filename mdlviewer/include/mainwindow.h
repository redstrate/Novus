#pragma once

#include <QMainWindow>
#include <unordered_map>

#include "renderer.hpp"
#include "types/slot.h"
#include "types/race.h"

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

    void refreshModel();

    void exportModel(Model& model, QString fileName);

private:
    std::vector<GearInfo> gears;
    std::vector<GearInfo*> loadedGears;

    Race currentRace = Race::HyurMidlanderMale;
    int currentLod = 0;

    GameData& data;

    Renderer* renderer;
    VulkanWindow* vkWindow;
    StandaloneWindow* standaloneWindow;
};