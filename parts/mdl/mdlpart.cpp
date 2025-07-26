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
#include <glm/gtx/matrix_major_storage.hpp>

#include "filecache.h"
#include "vulkanwindow.h"

MDLPart::MDLPart(SqPackResource *data, FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , data(data)
    , cache(cache)
{
    auto viewportLayout = new QVBoxLayout();
    viewportLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(viewportLayout);

    pbd = physis_parse_pbd(physis_gamedata_extract_file(data, "chara/xls/bonedeformer/human.pbd"));

    renderer = new RenderManager(data);

    auto inst = new QVulkanInstance();
    inst->setVkInstance(renderer->device().instance);
    inst->setFlags(QVulkanInstance::Flag::NoDebugOutputRedirect);
    inst->create();

    vkWindow = new VulkanWindow(this, renderer, inst);
    vkWindow->setVulkanInstance(inst);

    auto widget = QWidget::createWindowContainer(vkWindow);
    widget->installEventFilter(vkWindow);

    viewportLayout->addWidget(widget);

    connect(this, &MDLPart::modelChanged, this, &MDLPart::reloadRenderer);
    connect(this, &MDLPart::skeletonChanged, this, &MDLPart::reloadBoneData);
}

void MDLPart::exportModel(const QString &fileName)
{
    auto &model = vkWindow->models[0];
    ::exportModel(model.name, model.sourceObject->model, *skeleton, boneData, fileName);
}

DrawObject &MDLPart::getModel(const int index)
{
    return *vkWindow->models[index].sourceObject;
}

void MDLPart::reloadModel(const int index)
{
    renderer->reloadDrawObject(*vkWindow->models[index].sourceObject, 0);

    Q_EMIT modelChanged();
}

void MDLPart::clear()
{
    vkWindow->models.clear();

    Q_EMIT modelChanged();
}

void MDLPart::addModel(physis_MDL mdl,
                       bool skinned,
                       glm::vec3 position,
                       const QString &name,
                       std::vector<physis_Material> materials,
                       int lod,
                       uint16_t fromBodyId,
                       uint16_t toBodyId)
{
    DrawObject *model = nullptr;
    if (vkWindow->sourceModels.contains(name)) {
        model = vkWindow->sourceModels[name];
    } else {
        model = renderer->addDrawObject(mdl, lod);
        model->from_body_id = fromBodyId;
        model->to_body_id = toBodyId;
        model->skinned = skinned;

        std::transform(materials.begin(), materials.end(), std::back_inserter(model->materials), [this](const physis_Material &mat) {
            return createOrCacheMaterial(mat);
        });

        if (materials.empty()) {
            model->materials.push_back(createOrCacheMaterial(physis_Material{}));
        }

        vkWindow->sourceModels[name] = model;
    }

    Q_ASSERT(model != nullptr);
    vkWindow->models.push_back(DrawObjectInstance{name, model, position});

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
}

void MDLPart::enableFreemode()
{
    vkWindow->freeMode = true;
}

bool MDLPart::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        vkWindow->event(event);
        break;
    default:
        break;
    }
    return QWidget::event(event);
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

        // TODO: this applies to all instances
        for (auto &[_, model] : vkWindow->sourceModels) {
            // we want to map the actual affected bones to bone ids
            std::map<int, int> boneMapping;
            for (uint32_t i = 0; i < model->model.num_affected_bones; i++) {
                for (uint32_t k = 0; k < skeleton->num_bones; k++) {
                    if (std::string_view{skeleton->bones[k].name} == std::string_view{model->model.affected_bone_names[i]}) {
                        boneMapping[i] = k;
                    }
                }
            }

            std::vector<glm::mat4> deformBones(model->model.num_affected_bones);
            for (uint32_t i = 0; i < model->model.num_affected_bones; i++) {
                deformBones[i] = glm::mat4(1.0f);
            }

            // get deform matrices
            if (enableRacialDeform) {
                auto deform = physis_pbd_get_deform_matrix(pbd, model->from_body_id, model->to_body_id);
                if (deform.num_bones != 0) {
                    for (int i = 0; i < deform.num_bones; i++) {
                        auto deformBone = deform.bones[i];

                        for (uint32_t k = 0; k < model->model.num_affected_bones; k++) {
                            if (std::string_view{model->model.affected_bone_names[k]} == std::string_view{deformBone.name}) {
                                deformBones[k] =
                                    glm::rowMajor4(glm::vec4{deformBone.deform[0], deformBone.deform[1], deformBone.deform[2], deformBone.deform[3]},
                                                   glm::vec4{deformBone.deform[4], deformBone.deform[5], deformBone.deform[6], deformBone.deform[7]},
                                                   glm::vec4{deformBone.deform[8], deformBone.deform[9], deformBone.deform[10], deformBone.deform[11]},
                                                   glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
                            }
                        }
                    }
                }
            }

            for (uint32_t i = 0; i < model->model.num_affected_bones; i++) {
                const int originalBoneId = boneMapping[i];
                model->boneData[i] = deformBones[i] * boneData[originalBoneId].localTransform * boneData[originalBoneId].inversePose;
            }
        }
    }
}

RenderMaterial MDLPart::createMaterial(const physis_Material &material)
{
    RenderMaterial newMaterial;
    newMaterial.mat = material;

    if (material.shpk_name != nullptr) {
        std::string shpkPath = "shader/sm5/shpk/" + std::string(material.shpk_name);

        auto shpkData = physis_gamedata_extract_file(data, shpkPath.c_str());
        if (shpkData.data != nullptr) {
            newMaterial.shaderPackage = physis_parse_shpk(shpkData);
            if (newMaterial.shaderPackage.p_ptr) {
                // create the material parameters for this shader package
                newMaterial.materialBuffer =
                    renderer->device().createBuffer(newMaterial.shaderPackage.material_parameters_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
                renderer->device().nameBuffer(newMaterial.materialBuffer, "g_MaterialParameter"); // TODO: add material name

                // assumed to be floats, maybe not always true?
                std::vector<float> buffer(newMaterial.shaderPackage.material_parameters_size / sizeof(float));

                // copy the material data
                for (uint32_t i = 0; i < newMaterial.shaderPackage.num_material_parameters; i++) {
                    auto param = newMaterial.shaderPackage.material_parameters[i];

                    for (uint32_t j = 0; j < newMaterial.mat.num_constants; j++) {
                        auto constant = newMaterial.mat.constants[j];

                        if (constant.id == param.id) {
                            for (uint32_t z = 0; z < constant.num_values; z++) {
                                buffer[(param.byte_offset / sizeof(float)) + z] = constant.values[z];
                            }
                        }
                    }
                }

                renderer->device().copyToBuffer(newMaterial.materialBuffer, buffer.data(), buffer.size() * sizeof(float));
            }
        }
    }

    for (uint32_t i = 0; i < material.num_textures; i++) {
        std::string t = material.textures[i];

        if (t.find("skin") != std::string::npos) {
            newMaterial.type = MaterialType::Skin;
        }

        if (material.legacy_color_table.num_rows > 0) {
            int width = 4;
            int height = material.legacy_color_table.num_rows;

            qInfo() << "Creating legacy color table" << width << "X" << height;

            std::vector<float> rgbaData(width * height * 4);
            int offset = 0;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    const auto row = material.legacy_color_table.rows[y];

                    glm::vec4 color;
                    if (x == 0) {
                        color = glm::vec4{row.diffuse_color[0], row.diffuse_color[1], row.diffuse_color[2], row.specular_strength};
                    } else if (x == 1) {
                        color = glm::vec4{row.specular_color[0], row.specular_color[1], row.specular_color[2], row.gloss_strength};
                    } else if (x == 2) {
                        color = glm::vec4{row.emissive_color[0], row.emissive_color[1], row.emissive_color[2], row.tile_set};
                    } else if (x == 3) {
                        color = glm::vec4{row.material_repeat[0], row.material_repeat[1], row.material_skew[0], row.material_skew[1]};
                    }

                    rgbaData[offset] = color.x;
                    rgbaData[offset + 1] = color.y;
                    rgbaData[offset + 2] = color.z;
                    rgbaData[offset + 3] = color.a;

                    offset += 4;
                }
            }

            physis_Texture textureConfig;
            textureConfig.texture_type = TextureType::TwoDimensional;
            textureConfig.width = width;
            textureConfig.height = height;
            textureConfig.depth = 1;
            textureConfig.rgba = reinterpret_cast<uint8_t *>(rgbaData.data());
            textureConfig.rgba_size = rgbaData.size() * sizeof(float);

            // TODO: use 16-bit floating points like the game
            newMaterial.tableTexture = renderer->addGameTexture(VK_FORMAT_R32G32B32A32_SFLOAT, textureConfig);
            renderer->device().nameTexture(*newMaterial.tableTexture, "g_SamplerTable"); // TODO: add material name
        } else {
            int width = 4;
            int height = material.dawntrail_color_table.num_rows;
            if (height > 0) {
                qInfo() << "Creating DT color table" << width << "X" << height;

                // NOTE: this is just a copy of the legacy color table gen, it's probably all wrong!
                std::vector<float> rgbaData(width * height * 4);
                int offset = 0;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        const auto row = material.dawntrail_color_table.rows[y];

                        glm::vec4 color;
                        if (x == 0) {
                            color = glm::vec4{row.diffuse_color[0], row.diffuse_color[1], row.diffuse_color[2], 0.0f};
                        } else if (x == 1) {
                            color = glm::vec4{row.specular_color[0], row.specular_color[1], row.specular_color[2], 0.0f};
                        } else if (x == 2) {
                            color = glm::vec4{row.emissive_color[0], row.emissive_color[1], row.emissive_color[2], row.tile_set};
                        } else if (x == 3) {
                            color = glm::vec4{row.material_repeat[0], row.material_repeat[1], row.material_skew[0], row.material_skew[1]};
                        }

                        rgbaData[offset] = color.x;
                        rgbaData[offset + 1] = color.y;
                        rgbaData[offset + 2] = color.z;
                        rgbaData[offset + 3] = color.a;

                        offset += 4;
                    }
                }

                physis_Texture textureConfig;
                textureConfig.texture_type = TextureType::TwoDimensional;
                textureConfig.width = width;
                textureConfig.height = height;
                textureConfig.depth = 1;
                textureConfig.rgba = reinterpret_cast<uint8_t *>(rgbaData.data());
                textureConfig.rgba_size = rgbaData.size() * sizeof(float);

                // TODO: use 16-bit floating points like the game
                newMaterial.tableTexture = renderer->addGameTexture(VK_FORMAT_R32G32B32A32_SFLOAT, textureConfig);
                renderer->device().nameTexture(*newMaterial.tableTexture, "g_SamplerTable"); // TODO: add material name
            }
        }

        qInfo() << "Loading" << t;

        const auto type = t.substr(t.find_last_of('_') + 1, t.find_last_of('.') - t.find_last_of('_') - 1);
        auto texture = physis_texture_parse(cache.lookupFile(QLatin1String(material.textures[i])));
        if (texture.rgba != nullptr) {
            auto gameTexture = renderer->addGameTexture(VK_FORMAT_R8G8B8A8_UNORM, texture);
            renderer->device().nameTexture(gameTexture, material.textures[i]);

            if (type == "m") {
                newMaterial.multiTexture = gameTexture;
            } else if (type == "d") {
                newMaterial.diffuseTexture = gameTexture;
            } else if (type == "n") {
                newMaterial.normalTexture = gameTexture;
            } else if (type == "s") {
                newMaterial.specularTexture = gameTexture;
            } else if (type == "id") {
                newMaterial.indexTexture = gameTexture;
            } else {
                qWarning() << "Unknown texture type" << type;
            }
        } else {
            qInfo() << "Failed to load" << t;
        }
    }

    return newMaterial;
}

RenderMaterial MDLPart::createOrCacheMaterial(const physis_Material &mat)
{
    auto hash = getMaterialHash(mat);
    if (!renderMaterialCache.contains(hash)) {
        renderMaterialCache[hash] = createMaterial(mat);
    }

    return renderMaterialCache[hash];
}

uint64_t MDLPart::getMaterialHash(const physis_Material &mat)
{
    // TODO: this hash is terrible
    uint64_t hash = mat.shpk_name ? strlen(mat.shpk_name) : 0;
    hash += mat.num_constants;
    hash += mat.num_samplers;
    hash += mat.num_shader_keys;
    hash += mat.num_textures;

    return hash;
}

void MDLPart::calculateBoneInversePose(physis_Skeleton &skeleton, physis_Bone &bone, physis_Bone *parent_bone)
{
    const glm::mat4 parentMatrix = parent_bone == nullptr ? glm::mat4(1.0f) : boneData[parent_bone->index].inversePose;

    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    boneData[bone.index].inversePose = parentMatrix * local;

    for (uint32_t i = 0; i < skeleton.num_bones; i++) {
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

    for (uint32_t i = 0; i < skeleton.num_bones; i++) {
        if (skeleton.bones[i].parent_bone != nullptr && std::string_view{skeleton.bones[i].parent_bone->name} == std::string_view{bone.name}) {
            calculateBone(skeleton, skeleton.bones[i], &bone);
        }
    }
}

void MDLPart::removeModel(const physis_MDL &mdl)
{
    // TODO: restore this functionality
    qWarning() << "MDLPart::removeModel needs to be reimplemented!";
    /*vkWindow->models.erase(std::remove_if(vkWindow->models.begin(),
                                vkWindow->models.end(),
                                [mdl](const DrawObject &other) {
                                    return mdl.p_ptr == other.model.p_ptr;
                                }),
                 vkWindow->models.end());*/
    Q_EMIT modelChanged();
}

void MDLPart::setWireframe(bool wireframe)
{
    Q_UNUSED(wireframe)
    // renderer->wireframe = wireframe;
}

bool MDLPart::wireframe() const
{
    // return renderer->wireframe;
    return false;
}

int MDLPart::numModels() const
{
    return vkWindow->models.size();
}

RenderManager *MDLPart::manager() const
{
    return renderer;
}

bool MDLPart::modelExists(const QString &name)
{
    return vkWindow->sourceModels.contains(name);
}

void MDLPart::addExistingModel(const QString &name, glm::vec3 position)
{
    auto model = vkWindow->sourceModels[name];
    vkWindow->models.push_back(DrawObjectInstance{name, model, position});
}

#include "moc_mdlpart.cpp"
