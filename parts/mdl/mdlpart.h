// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <optional>
#include <physis.hpp>

#include "mdlexport.h"
#include "rendermanager.h"

struct GameData;

class VulkanWindow;
class FileCache;

class MDLPart : public QWidget
{
    Q_OBJECT

public:
    explicit MDLPart(GameData *data, FileCache &cache, QWidget *parent = nullptr);

    void exportModel(const QString &fileName);
    DrawObject &getModel(int index);
    void reloadModel(int index);

    int lastX = -1;
    int lastY = -1;

    enum class CameraMode { None, Orbit, Move };

    CameraMode cameraMode = CameraMode::None;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float cameraDistance = 2.0f;
    float minimumCameraDistance = 1.0f;
    glm::vec3 position{0, 0, 0};

    std::unique_ptr<physis_Skeleton> skeleton;

    std::vector<BoneData> boneData;

    std::function<void()> requestUpdate;

    void setWireframe(bool wireframe);
    bool wireframe() const;

    int numModels() const;

    RenderManager *manager() const;

    physis_PBD pbd{};

    bool enableRacialDeform = true;

Q_SIGNALS:
    void modelChanged();
    void skeletonChanged();
    // Called when a Vulkan context is available, and you can safely access RenderManager
    void initializeRender();

public Q_SLOTS:
    /// Clears all stored MDLs.
    void clear();

    /// Adds a new MDL with a list of materials used.
    void addModel(physis_MDL mdl,
                  bool skinned,
                  glm::vec3 position,
                  const QString &name,
                  std::vector<physis_Material> materials,
                  int lod,
                  uint16_t fromBodyId = 101,
                  uint16_t toBodyId = 101);

    void removeModel(const physis_MDL &mdl);

    /// Sets the skeleton any skinned MDLs should bind to.
    void setSkeleton(physis_Skeleton skeleton);

    /// Clears the current skeleton.
    void clearSkeleton();

    void reloadBoneData();
    void reloadRenderer();

    void enableFreemode();

protected:
    bool event(QEvent *event) override;

private:
    RenderMaterial createMaterial(const physis_Material &mat);
    RenderMaterial createOrCacheMaterial(const physis_Material &mat);
    uint64_t getMaterialHash(const physis_Material &mat);

    void calculateBoneInversePose(physis_Skeleton &skeleton, physis_Bone &bone, physis_Bone *parent_bone);
    void calculateBone(physis_Skeleton &skeleton, physis_Bone &bone, const physis_Bone *parent_bone);

    GameData *data = nullptr;
    FileCache &cache;

    std::unordered_map<uint64_t, RenderMaterial> renderMaterialCache;

    RenderManager *renderer = nullptr;
    VulkanWindow *vkWindow = nullptr;
    bool firstTimeSkeletonDataCalculated = false;
};
