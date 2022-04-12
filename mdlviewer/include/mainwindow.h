#pragma once

#include <QMainWindow>

#include "renderer.hpp"

enum class Slot {
    Body,
    Legs
};

enum class Race {
    HyurMidlanderMale,
    HyurMidlanderFemale
};

inline std::map<Race, std::string_view> raceNames = {
        {Race::HyurMidlanderMale, "Hyur Midlander Male"},
        {Race::HyurMidlanderFemale, "Hyur Midlander Female"}
};

inline std::unordered_map<Race, std::string_view> raceIDs = {
        {Race::HyurMidlanderMale, "0101"},
        {Race::HyurMidlanderFemale, "0201"}
};

struct ModelInfo {

};

struct GearInfo {
    std::string name;
    Slot slot;
    ModelInfo modelInfo;
};

inline std::unordered_map<Slot, std::string_view> slotToName = {
        {Slot::Body, "top"},
        {Slot::Legs, "dwn"}
};

class GameData;
class VulkanWindow;

class MainWindow : public QMainWindow {
public:
    MainWindow(GameData& data);

    void refreshModel();

private:
    std::vector<GearInfo> gears;
    std::vector<GearInfo*> loadedGears;

    Race currentRace;

    GameData& data;

    Renderer* renderer;
    VulkanWindow* vkWindow;
};