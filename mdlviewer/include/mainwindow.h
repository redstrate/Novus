#pragma once

#include <QMainWindow>
#include <unordered_map>

#include "renderer.hpp"

enum class Slot {
    Head = 3,
    Hands = 5,
    Legs = 7,
    Feet = 8,
    Body = 4,
    Earring = 9,
    Neck = 10,
    Rings = 12,
    Wrists = 11
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
    int primaryID;
};

struct GearInfo {
    std::string name;
    Slot slot;
    ModelInfo modelInfo;
};

inline std::unordered_map<Slot, std::string_view> slotToName = {
        {Slot::Head, "met"},
        {Slot::Hands, "glv"},
        {Slot::Legs, "dwn"},
        {Slot::Feet, "sho"},
        {Slot::Body, "top"},
        {Slot::Earring, "ear"},
        {Slot::Neck, "nek"},
        {Slot::Rings, "rir"},
        {Slot::Wrists, "wrs"}
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

    GameData& data;

    Renderer* renderer;
    VulkanWindow* vkWindow;
    StandaloneWindow* standaloneWindow;
};