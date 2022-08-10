#pragma once

#include <QMainWindow>
#include <unordered_map>
#include <QComboBox>
#include <physis.hpp>

#include "renderer.hpp"

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
    MainWindow(GameData* data);

    void exportModel(physis_MDL& model, physis_Skeleton& skeleton, QString fileName);

private:
    void loadInitialGearInfo(GearInfo& info);
    void reloadGearModel();
    void reloadGearAppearance();
    void calculate_bone_inverse_pose(physis_Skeleton& skeleton, physis_Bone& bone, physis_Bone* parent_bone);
    void calculate_bone(physis_Skeleton& skeleton, physis_Bone& bone, const physis_Bone* parent_bone);

    std::vector<GearInfo> gears;

    struct LoadedGear {
        GearInfo* gearInfo;
        physis_MDL model;
        RenderModel renderModel;
    };

    struct BoneExtra {
        glm::mat4 localTransform, finalTransform, inversePose;
    };

    LoadedGear loadedGear;

    QComboBox* raceCombo, *lodCombo;

    Race currentRace = Race::Hyur;
    Subrace currentSubrace = Subrace::Midlander;
    Gender currentGender = Gender::Male;
    int currentLod = 0;
    glm::vec3 currentScale = glm::vec3(1);
    physis_Bone* currentEditedBone = nullptr;

    GameData& data;

    Renderer* renderer;
    VulkanWindow* vkWindow;
    StandaloneWindow* standaloneWindow;

    physis_Skeleton skeleton;
    std::vector<BoneExtra> extraBone;
};