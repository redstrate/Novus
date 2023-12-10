// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mdlpart.h"
#include "glm/gtx/transform.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QVulkanInstance>
#include <QVulkanWindow>
#include <cmath>
#include <glm/gtc/quaternion.hpp>

#include "filecache.h"
#include "vulkanwindow.h"

MDLPart::MDLPart(GameData *data, FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , data(data)
    , cache(cache)
{
    auto viewportLayout = new QVBoxLayout();
    viewportLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(viewportLayout);

    pbd = physis_parse_pbd(physis_gamedata_extract_file(data, "chara/xls/bonedeformer/human.pbd"));

    renderer = new Renderer();

    auto inst = new QVulkanInstance();
    inst->setVkInstance(renderer->instance);
    inst->setFlags(QVulkanInstance::Flag::NoDebugOutputRedirect);
    inst->create();

    vkWindow = new VulkanWindow(this, renderer, inst);
    vkWindow->setVulkanInstance(inst);

    auto widget = QWidget::createWindowContainer(vkWindow);

    viewportLayout->addWidget(widget);

    connect(this, &MDLPart::modelChanged, this, &MDLPart::reloadRenderer);
    connect(this, &MDLPart::skeletonChanged, this, &MDLPart::reloadBoneData);
}

void MDLPart::exportModel(const QString &fileName)
{
    auto &model = models[0];
    ::exportModel(model.name, model.model, *skeleton, boneData, fileName);
}

RenderModel &MDLPart::getModel(const int index)
{
    return models[index];
}

void MDLPart::reloadModel(const int index)
{
    renderer->reloadModel(models[index], 0);

    Q_EMIT modelChanged();
}

void MDLPart::clear()
{
    models.clear();

    Q_EMIT modelChanged();
}

void MDLPart::addModel(physis_MDL mdl, const QString &name, std::vector<physis_Material> materials, int lod, uint16_t fromBodyId, uint16_t toBodyId)
{
    qDebug() << "Adding model to MDLPart";

    auto model = renderer->addModel(mdl, lod);
    model.name = name;
    model.from_body_id = fromBodyId;
    model.to_body_id = toBodyId;

    std::transform(materials.begin(), materials.end(), std::back_inserter(model.materials), [this](const physis_Material &mat) {
        return createMaterial(mat);
    });

    if (materials.empty()) {
        model.materials.push_back(createMaterial(physis_Material{}));
    }

    models.push_back(model);

    Q_EMIT modelChanged();
}

void MDLPart::setSkeleton(physis_Skeleton newSkeleton)
{
    skeleton = std::make_unique<physis_Skeleton>(newSkeleton);

    firstTimeSkeletonDataCalculated = false;

    Q_EMIT skeletonChanged();
}

void MDLPart::clearSkeleton()
{
    skeleton.reset();

    firstTimeSkeletonDataCalculated = false;

    Q_EMIT skeletonChanged();
}

void MDLPart::reloadRenderer()
{
    reloadBoneData();

    vkWindow->models = models;
}

void MDLPart::reloadBoneData()
{
    if (skeleton) {
        if (!firstTimeSkeletonDataCalculated) {
            if (boneData.empty()) {
                boneData.resize(skeleton->num_bones);
            }

            calculateBoneInversePose(*skeleton, *skeleton->root_bone, nullptr);

            for (auto &bone : boneData) {
                bone.inversePose = glm::inverse(bone.inversePose);
            }
            firstTimeSkeletonDataCalculated = true;
        }

        // update data
        calculateBone(*skeleton, *skeleton->root_bone, nullptr);

        for (auto &model : models) {
            // we want to map the actual affected bones to bone ids
            std::map<int, int> boneMapping;
            for (int i = 0; i < model.model.num_affected_bones; i++) {
                for (int k = 0; k < skeleton->num_bones; k++) {
                    if (std::string_view{skeleton->bones[k].name} == std::string_view{model.model.affected_bone_names[i]}) {
                        boneMapping[i] = k;
                    }
                }
            }

            std::vector<glm::mat4> deformBones(model.model.num_affected_bones);
            for (int i = 0; i < model.model.num_affected_bones; i++) {
                deformBones[i] = glm::mat4(1.0f);
            }

            // get deform matrices
            auto deform = physis_pbd_get_deform_matrix(pbd, model.from_body_id, model.to_body_id);
            if (deform.num_bones != 0) {
                for (int i = 0; i < deform.num_bones; i++) {
                    auto deformBone = deform.bones[i];

                    for (int k = 0; k < model.model.num_affected_bones; k++) {
                        if (std::string_view{model.model.affected_bone_names[k]} == std::string_view{deformBone.name}) {
                            deformBones[k] = glm::mat4{deformBone.deform[0],
                                                       deformBone.deform[1],
                                                       deformBone.deform[2],
                                                       deformBone.deform[3],
                                                       deformBone.deform[4],
                                                       deformBone.deform[5],
                                                       deformBone.deform[6],
                                                       deformBone.deform[7],
                                                       deformBone.deform[8],
                                                       deformBone.deform[9],
                                                       deformBone.deform[10],
                                                       deformBone.deform[11],
                                                       0.0f,
                                                       0.0f,
                                                       0.0f,
                                                       1.0f};
                        }
                    }
                }
            }

            for (int i = 0; i < model.model.num_affected_bones; i++) {
                const int originalBoneId = boneMapping[i];
                model.boneData[i] = boneData[originalBoneId].localTransform * deformBones[i] * boneData[originalBoneId].inversePose;
            }
        }
    }
}

RenderMaterial MDLPart::createMaterial(const physis_Material &material)
{
    RenderMaterial newMaterial;

    for (int i = 0; i < material.num_textures; i++) {
        std::string t = material.textures[i];

        if (t.find("skin") != std::string::npos) {
            newMaterial.type = MaterialType::Skin;
        }

        char type = t[t.length() - 5];

        switch (type) {
        case 'm': {
            auto texture = physis_texture_parse(cache.lookupFile(QLatin1String(material.textures[i])));
            auto tex = renderer->addTexture(texture.width, texture.height, texture.rgba, texture.rgba_size);

            newMaterial.multiTexture = new RenderTexture(tex);
        }
        case 'd': {
            auto texture = physis_texture_parse(cache.lookupFile(QLatin1String(material.textures[i])));
            auto tex = renderer->addTexture(texture.width, texture.height, texture.rgba, texture.rgba_size);

            newMaterial.diffuseTexture = new RenderTexture(tex);
        } break;
        case 'n': {
            auto texture = physis_texture_parse(cache.lookupFile(QLatin1String(material.textures[i])));
            auto tex = renderer->addTexture(texture.width, texture.height, texture.rgba, texture.rgba_size);

            newMaterial.normalTexture = new RenderTexture(tex);
        } break;
        case 's': {
            auto texture = physis_texture_parse(cache.lookupFile(QLatin1String(material.textures[i])));
            auto tex = renderer->addTexture(texture.width, texture.height, texture.rgba, texture.rgba_size);

            newMaterial.specularTexture = new RenderTexture(tex);
        } break;
        default:
            qDebug() << "unhandled type" << type;
            break;
        }
    }

    return newMaterial;
}

void MDLPart::calculateBoneInversePose(physis_Skeleton &skeleton, physis_Bone &bone, physis_Bone *parent_bone)
{
    const glm::mat4 parentMatrix = parent_bone == nullptr ? glm::mat4(1.0f) : boneData[parent_bone->index].inversePose;

    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    boneData[bone.index].inversePose = parentMatrix * local;

    for (int i = 0; i < skeleton.num_bones; i++) {
        if (skeleton.bones[i].parent_bone != nullptr && std::string_view{skeleton.bones[i].parent_bone->name} == std::string_view{bone.name}) {
            calculateBoneInversePose(skeleton, skeleton.bones[i], &bone);
        }
    }
}

void MDLPart::calculateBone(physis_Skeleton &skeleton, physis_Bone &bone, const physis_Bone *parent_bone)
{
    const glm::mat4 parent_matrix = parent_bone == nullptr ? glm::mat4(1.0f) : (boneData[parent_bone->index].localTransform);

    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    boneData[bone.index].localTransform = parent_matrix * local;
    boneData[bone.index].finalTransform = parent_matrix;

    for (int i = 0; i < skeleton.num_bones; i++) {
        if (skeleton.bones[i].parent_bone != nullptr && std::string_view{skeleton.bones[i].parent_bone->name} == std::string_view{bone.name}) {
            calculateBone(skeleton, skeleton.bones[i], &bone);
        }
    }
}

void MDLPart::removeModel(const physis_MDL &mdl)
{
    models.erase(std::remove_if(models.begin(),
                                models.end(),
                                [mdl](const RenderModel &other) {
                                    return mdl.p_ptr == other.model.p_ptr;
                                }),
                 models.end());
    Q_EMIT modelChanged();
}

#include "moc_mdlpart.cpp"