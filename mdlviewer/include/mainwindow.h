#pragma once

#include <QComboBox>
#include <QMainWindow>
#include <fmt/format.h>
#include <physis.hpp>
#include <unordered_map>

#include "renderer.hpp"

struct ModelInfo {
    int primaryID;
    int gearVersion = 1;
};

struct GearInfo {
    std::string name;
    Slot slot;
    ModelInfo modelInfo;

    std::string getMtrlPath(int raceID) {
        return fmt::format("chara/equipment/e{gearId:04d}/material/v{gearVersion:04d}/mt_c{raceId:04d}e{gearId:04d}_{slot}_a.mtrl",
                  fmt::arg("gearId", modelInfo.primaryID),
                  fmt::arg("gearVersion", modelInfo.gearVersion),
                  fmt::arg("raceId", raceID),
                  fmt::arg("slot", physis_get_slot_name(slot)));
    }
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
        physis_Material material;
        RenderModel renderModel;
        RenderTexture renderTexture;
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