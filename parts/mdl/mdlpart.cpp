// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mdlpart.h"
#include "glm/gtx/transform.hpp"

#include <KLocalizedString>
#include <QJsonObject>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QVulkanWindow>
#include <cmath>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_major_storage.hpp>

#include "filecache.h"
#include "knownvalues.h"
#include "vulkanwindow.h"

MDLPart::MDLPart(FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , m_cache(cache)
{
    const auto viewportLayout = new QVBoxLayout();
    viewportLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(viewportLayout);

    const auto pbdFile = m_cache.read(QStringLiteral("chara/xls/bonedeformer/human.pbd"));
    if (pbdFile.size == 0) {
        qWarning() << "Failed to read chara/xls/bonedeformer/human.pbd";
    } else {
        pbd = physis_pbd_parse(cache.platform(), pbdFile);
        if (!pbd.p_ptr) {
            qWarning() << "Failed to parse chara/xls/bonedeformer/human.pbd";
        }
    }

    m_renderer = std::make_unique<RenderManager>(m_cache);

    m_instance = std::make_unique<QVulkanInstance>();
    m_instance->setVkInstance(m_renderer->device().instance);
    m_instance->setFlags(QVulkanInstance::Flag::NoDebugOutputRedirect);
    if (!m_instance->create()) {
        qWarning() << "Failed to create QVulkanInstance!";
    }

    m_vkWindow = new VulkanWindow(this, m_renderer.get(), m_instance.get());
    m_vkWindow->setVulkanInstance(m_instance.get());

    const auto widget = createWindowContainer(m_vkWindow);
    widget->installEventFilter(m_vkWindow);

    viewportLayout->addWidget(widget);

    connect(this, &MDLPart::modelChanged, this, &MDLPart::reloadRenderer);
    connect(this, &MDLPart::skeletonChanged, this, &MDLPart::reloadBoneData);

    m_wireframeAction = new QAction(i18n("Wireframe"));
    m_wireframeAction->setCheckable(true);
    m_wireframeAction->setChecked(m_renderer->scene.wireframe);
    connect(m_wireframeAction, &QAction::toggled, this, [this](const bool toggled) {
        m_renderer->scene.wireframe = toggled;
    });

    m_frustumCullingAction = new QAction(i18n("Frustum Culling"));
    m_frustumCullingAction->setCheckable(true);
    m_frustumCullingAction->setChecked(m_renderer->scene.frustumCulling);
    connect(m_frustumCullingAction, &QAction::toggled, this, [this](const bool toggled) {
        m_renderer->scene.frustumCulling = toggled;
    });

    m_debugFrustumCullingAction = new QAction(i18n("Draw Culling AABBs"));
    m_debugFrustumCullingAction->setCheckable(true);
    m_debugFrustumCullingAction->setChecked(m_renderer->scene.debugFrustumCulling);
    connect(m_debugFrustumCullingAction, &QAction::toggled, this, [this](const bool toggled) {
        m_renderer->scene.debugFrustumCulling = toggled;
    });
}

MDLPart::~MDLPart()
{
    physis_pbd_free(&pbd);
    destroyObjects();
}

void MDLPart::exportModel(const QString &fileName) const
{
    const auto &model = m_vkWindow->models[0];
    ::exportModel(model.name, model.sourceObject->model, skeleton.get(), boneData, fileName);
}

DrawObject &MDLPart::getModel(const int index) const
{
    return *m_vkWindow->models[index].sourceObject;
}

void MDLPart::reloadModel(const int index)
{
    m_renderer->reloadDrawObject(*m_vkWindow->models[index].sourceObject);

    Q_EMIT modelChanged();
}

void MDLPart::clear()
{
    destroyObjects();
    Q_EMIT modelChanged();
}

void MDLPart::addModel(const physis_MDL &mdl,
                       const bool skinned,
                       const Transformation &transformation,
                       const QString &name,
                       std::vector<std::pair<std::string, physis_Material>> materials,
                       const uint16_t fromBodyId,
                       const uint16_t toBodyId)
{
    DrawObject *model = nullptr;
    if (m_vkWindow->sourceModels.contains(name)) {
        model = m_vkWindow->sourceModels[name];
    } else {
        model = m_renderer->addDrawObject(mdl, name.toStdString());
        model->from_body_id = fromBodyId;
        model->to_body_id = toBodyId;
        model->skinned = skinned;

        std::ranges::transform(materials, std::back_inserter(model->materials), [this](const std::pair<std::string, physis_Material> &mat) {
            return createOrCacheMaterial(mat.first, mat.second);
        });

        if (materials.empty()) {
            model->materials.push_back(createOrCacheMaterial("invalid", physis_Material{}));
        }

        m_vkWindow->sourceModels[name] = model;
    }

    Q_ASSERT(model != nullptr);
    m_vkWindow->models.push_back(DrawObjectInstance{name, model, transformation});

    Q_EMIT modelChanged();
}

void MDLPart::setSkeleton(physis_Skeleton newSkeleton)
{
    skeleton = std::make_unique<physis_Skeleton>(newSkeleton);

    m_firstTimeSkeletonDataCalculated = false;

    Q_EMIT skeletonChanged();
}

void MDLPart::clearSkeleton()
{
    skeleton.reset();

    m_firstTimeSkeletonDataCalculated = false;

    Q_EMIT skeletonChanged();
}

void MDLPart::reloadRenderer()
{
    reloadBoneData();
}

void MDLPart::enableFreemode()
{
    pitch = glm::radians(180.0f); // rotate the camera to face forward by default
    m_vkWindow->freeMode = true;
}

bool MDLPart::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        m_vkWindow->event(event);
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

void MDLPart::destroyObjects()
{
    // If we're destroying all objects, more than likely we're loading a new set of materials and have no use for the old ones.
    m_renderer->freeResources();

    m_renderer->scene.resetLights();

    m_vkWindow->models.clear();
    for (const auto &model : m_vkWindow->sourceModels | std::views::values) {
        m_renderer->destroyDrawObject(*model);
        delete model;
    }
    m_vkWindow->sourceModels.clear();
    for (auto &material : m_renderMaterialCache | std::views::values) {
        destroyMaterial(material);
    }
    m_renderMaterialCache.clear();
    for (auto &shpk : m_shaderPackageCache | std::views::values) {
        physis_shpk_free(&shpk);
    }
    m_shaderPackageCache.clear();
    for (auto &texture : m_textureCache | std::views::values) {
        destroyTexture(texture);
    }
    m_textureCache.clear();
}

void MDLPart::reloadBoneData()
{
    if (skeleton) {
        if (!m_firstTimeSkeletonDataCalculated) {
            if (boneData.empty()) {
                boneData.resize(skeleton->num_bones);
            }

            calculateBoneInversePose(*skeleton, *skeleton->root_bone, nullptr);

            for (auto &bone : boneData) {
                bone.inversePose = glm::inverse(bone.inversePose);
            }
            m_firstTimeSkeletonDataCalculated = true;
        }

        // update data
        calculateBone(*skeleton, *skeleton->root_bone, nullptr);

        // TODO: this applies to all instances
        for (const auto &model : m_vkWindow->sourceModels | std::views::values) {
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
                const auto deform = physis_pbd_get_deform_matrix(pbd, model->from_body_id, model->to_body_id);
                if (deform.num_bones != 0) {
                    for (int i = 0; i < deform.num_bones; i++) {
                        const auto deformBone = deform.bones[i];

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

            for (uint32_t i = 0; i < model->boneData.size(); i++) {
                model->boneData[i] = glm::mat3x4(1.0f);
            }

            for (uint32_t i = 0; i < model->model.num_affected_bones; i++) {
                const int originalBoneId = boneMapping[i];
                model->boneData[i] = deformBones[i] * boneData[originalBoneId].localTransform * boneData[originalBoneId].inversePose;
            }
        }
    }
}

RenderMaterial MDLPart::createMaterial(const std::string &path, const physis_Material &material)
{
    RenderMaterial newMaterial;
    newMaterial.path = path;
    newMaterial.mat = material;

    if (material.shpk_name != nullptr) {
        QString shpkPath = QStringLiteral("shader/sm5/shpk/%1").arg(QString::fromStdString(material.shpk_name));
        auto h = std::hash<std::string>{}(shpkPath.toStdString());
        if (!m_shaderPackageCache.contains(h)) {
            auto shpkData = m_cache.read(shpkPath);
            if (shpkData.data != nullptr) {
                m_shaderPackageCache[h] = physis_shpk_parse(m_cache.platform(), shpkData);
            }
        }

        newMaterial.shaderPackage = m_shaderPackageCache[h];
        if (newMaterial.shaderPackage.p_ptr) {
            // create the material parameters for this shader package
            newMaterial.materialBuffer =
                m_renderer->device().createBuffer(newMaterial.shaderPackage.material_parameters_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
            m_renderer->device().nameBuffer(newMaterial.materialBuffer, "g_MaterialParameter"); // TODO: add material name

            // assumed to be floats, maybe not always true?
            std::vector<float> buffer(newMaterial.shaderPackage.material_parameters_size / sizeof(float));

            // we need to fill it with the defaults from the shader package. These are then either filled in/overwritten by the material
            for (uint32_t i = 0; i < newMaterial.shaderPackage.material_default_parameters_size; i++) {
                buffer[i] = newMaterial.shaderPackage.material_default_parameters[i];
            }

            // copy the material data
            for (uint32_t i = 0; i < newMaterial.shaderPackage.num_material_parameters; i++) {
                auto param = newMaterial.shaderPackage.material_parameters[i];

                for (uint32_t j = 0; j < newMaterial.mat.num_constants; j++) {
                    auto constant = newMaterial.mat.constants[j];

                    if (constant.id == param.id) {
                        for (uint32_t z = 0; z < constant.num_values; z++) {
                            buffer[param.byte_offset / sizeof(float) + z] = constant.values[z];
                        }
                    }
                }
            }

            m_renderer->device().copyToBuffer(newMaterial.materialBuffer, buffer.data(), buffer.size() * sizeof(float));
        }
    }

    for (uint32_t i = 0; i < material.num_samplers; i++) {
        const auto sampler = material.samplers[i];
        const auto usage = nameFromCrc(sampler.texture_usage);

        if (sampler.texture_index > material.num_textures) {
            qWarning() << path << "has a sampler that is indexing a texture that shouldn't exist, please report this as a bug!";
            continue;
        }

        std::string t = material.textures[sampler.texture_index];
        if (t.find("skin") != std::string::npos) {
            newMaterial.type = MaterialType::Skin;
        }

        // TODO: pass through the textures directly and allow the renderer to figure this out i guess
        if (usage == "g_SamplerDiffuse" || usage == "g_SamplerColorMap0") { // ColorMap0 is used by bg.shpk, g_SamplerDiffuse is character.shpk
            newMaterial.diffuseTexture = createOrCacheTexture(t);
            newMaterial.diffuseTexturePath = t;
        } else if (usage == "g_SamplerMask") {
            newMaterial.maskTexture = createOrCacheTexture(t);
        } else if (usage == "g_SamplerNormal" || usage == "g_SamplerNormalMap0") {
            newMaterial.normalTexture = createOrCacheTexture(t);
        } else if (usage == "g_SamplerSpecular" || usage == "g_SamplerSpecularMap0") {
            newMaterial.specularTexture = createOrCacheTexture(t);
        } else if (usage == "g_SamplerIndex") {
            newMaterial.indexTexture = createOrCacheTexture(t);
        } else if (usage.starts_with("g_")) {
            // Intentionally ignored as we don't support these yet
        } else {
            qWarning() << "Unknown texture usage:" << usage << sampler.texture_usage << "from" << path << "Please report this is a bug!";
        }
    }

    // Create dye table
    if (material.legacy_color_table.num_rows > 0) {
        int width = 4;
        int height = material.legacy_color_table.num_rows;

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
                    color = glm::vec4{row.material_repeat_x, row.material_repeat_y, row.material_skew[0], row.material_skew[1]};
                }

                rgbaData[offset] = color.x;
                rgbaData[offset + 1] = color.y;
                rgbaData[offset + 2] = color.z;
                rgbaData[offset + 3] = color.a;

                offset += 4;
            }
        }

        physis_Texture textureConfig{};
        textureConfig.attribute = TextureAttribute_TEXTURE_TYPE2_D;
        textureConfig.format = TextureFormat::R32G32B32A32_FLOAT;
        textureConfig.width = width;
        textureConfig.height = height;
        textureConfig.depth = 1;
        textureConfig.data = reinterpret_cast<uint8_t *>(rgbaData.data());
        textureConfig.data_size = rgbaData.size() * sizeof(float);
        textureConfig.mip_levels = 1;

        // TODO: use 16-bit floating points like the game
        newMaterial.tableTexture = m_renderer->addGameTexture(textureConfig);
        m_renderer->device().nameTexture(*newMaterial.tableTexture, "g_SamplerTable"); // TODO: add material name
    } else {
        constexpr int width = 8;
        const uint32_t height = material.dawntrail_color_table.num_rows;
        if (height > 0) {
            // NOTE: this is just a copy of the legacy color table gen, it's probably all wrong!
            std::vector<float> rgbaData(width * height * 4);
            int offset = 0;
            for (uint32_t y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    const auto row = material.dawntrail_color_table.rows[y];

                    glm::vec4 color{};
                    if (x == 0) {
                        color = glm::vec4{row.diffuse_color[0], row.diffuse_color[1], row.diffuse_color[2], 0.0f};
                    } else if (x == 1) {
                        color = glm::vec4{row.specular_color[0], row.specular_color[1], row.specular_color[2], 0.0f};
                    } else if (x == 2) {
                        color = glm::vec4{row.emissive_color[0], row.emissive_color[1], row.emissive_color[2], row.tile_set};
                    } else if (x == 3) {
                        color = glm::vec4{row.material_repeat[0], row.material_repeat[1], row.material_skew[0], row.material_skew[1]};
                    }
                    // TOOD: fill out with other DT values

                    rgbaData[offset] = color.x;
                    rgbaData[offset + 1] = color.y;
                    rgbaData[offset + 2] = color.z;
                    rgbaData[offset + 3] = color.a;

                    offset += 4;
                }
            }

            physis_Texture textureConfig{};
            textureConfig.attribute = TextureAttribute_TEXTURE_TYPE2_D;
            textureConfig.format = TextureFormat::R32G32B32A32_FLOAT;
            textureConfig.width = width;
            textureConfig.height = height;
            textureConfig.depth = 1;
            textureConfig.data = reinterpret_cast<uint8_t *>(rgbaData.data());
            textureConfig.data_size = rgbaData.size() * sizeof(float);
            textureConfig.mip_levels = 1;

            // TODO: use 16-bit floating points like the game
            newMaterial.tableTexture = m_renderer->addGameTexture(textureConfig);
            m_renderer->device().nameTexture(*newMaterial.tableTexture, "g_SamplerTable"); // TODO: add material name
        }
    }

    return newMaterial;
}

RenderMaterial MDLPart::createOrCacheMaterial(const std::string &path, const physis_Material &mat)
{
    const auto hash = std::hash<std::string>{}(path);
    if (!m_renderMaterialCache.contains(hash)) {
        m_renderMaterialCache[hash] = createMaterial(path, mat);
    }

    return m_renderMaterialCache[hash];
}

void MDLPart::destroyMaterial(RenderMaterial &material) const
{
    // NOTE: the other textures are cached so we don't want to destroy them here! (maybe in the future if we remove them from the cache...)
    if (auto &texture = material.tableTexture) {
        m_renderer->device().destroyTexture(texture.value());
    }

    m_renderer->device().destroyBuffer(material.materialBuffer);
}

Texture MDLPart::createTexture(const std::string &path) const
{
    const auto texture = physis_texture_parse(m_cache.platform(), m_cache.read(QLatin1String(path)));
    if (texture.p_ptr != nullptr) {
        auto gameTexture = m_renderer->addGameTexture(texture);
        m_renderer->device().nameTexture(gameTexture, "Game Texture " + path);

        physis_tex_free(&texture);

        return gameTexture;
    }

    qWarning() << "Failed to load" << path;
    return {};
}

Texture MDLPart::createOrCacheTexture(const std::string &path)
{
    const auto hash = std::hash<std::string>{}(path);
    if (!m_textureCache.contains(hash)) {
        m_textureCache[hash] = createTexture(path);
    }

    return m_textureCache[hash];
}

void MDLPart::destroyTexture(Texture &texture) const
{
    m_renderer->device().destroyTexture(texture);
}

void MDLPart::calculateBoneInversePose(physis_Skeleton &skeleton, const physis_Bone &bone, const physis_Bone *parent_bone)
{
    const glm::mat4 parentMatrix = parent_bone == nullptr ? glm::mat4(1.0f) : boneData[parent_bone->index].inversePose;

    auto local = glm::mat4(1.0f);
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

void MDLPart::calculateBone(physis_Skeleton &skeleton, const physis_Bone &bone, const physis_Bone *parent_bone)
{
    const glm::mat4 parent_matrix = parent_bone == nullptr ? glm::mat4(1.0f) : boneData[parent_bone->index].localTransform;

    auto local = glm::mat4(1.0f);
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
    m_vkWindow->models.erase(std::ranges::remove_if(m_vkWindow->models,
                                                    [mdl](const DrawObjectInstance &other) {
                                                        return mdl.p_ptr == other.sourceObject->model.p_ptr;
                                                    })
                                 .begin(),
                             m_vkWindow->models.end());
    Q_EMIT modelChanged();
}

void MDLPart::addLight(const SceneLight &light) const
{
    m_renderer->scene.lights.push_back(light);
}

void MDLPart::addVfx(const physis_Avfx &vfx, const Transformation &transformation, const QString &name)
{
    VfxObject *vfxObj = nullptr;
    if (m_vkWindow->sourceVfx.contains(name)) {
        vfxObj = m_vkWindow->sourceVfx[name];
    } else {
        std::vector<physis_Texture> textures;
        for (uint32_t i = 0; i < vfx.texture_count; i++) {
            const auto buffer = m_cache.read(QString::fromStdString(vfx.textures[i]));
            if (buffer.size > 0) {
                textures.push_back(physis_texture_parse(m_cache.platform(), buffer));
            }
        }
        vfxObj = m_renderer->addVFXObject(vfx, textures, name.toStdString());
        m_vkWindow->sourceVfx[name] = vfxObj;
    }

    Q_ASSERT(vfxObj != nullptr);
    m_vkWindow->vfx.push_back(VfxObjectInstance{name, vfxObj, transformation});

    Q_EMIT modelChanged();
}

void MDLPart::addExistingVfx(const QString &name, const Transformation &transformation) const
{
    const auto vfx = m_vkWindow->sourceVfx[name];
    m_vkWindow->vfx.push_back(VfxObjectInstance{name, vfx, transformation});
}

int MDLPart::numModels() const
{
    return m_vkWindow->models.size();
}

RenderManager *MDLPart::manager() const
{
    return m_renderer.get();
}

QImage MDLPart::grab() const
{
    return m_renderer->grab(m_vkWindow->models, m_vkWindow->vfx);
}

QAction *MDLPart::wireframeAction() const
{
    return m_wireframeAction;
}

QAction *MDLPart::frustumCullingAction() const
{
    return m_frustumCullingAction;
}

QAction *MDLPart::debugFrustumCullingAction() const
{
    return m_debugFrustumCullingAction;
}

bool MDLPart::modelExists(const QString &name) const
{
    return m_vkWindow->sourceModels.contains(name);
}

bool MDLPart::vfxExists(const QString &name) const
{
    return m_vkWindow->sourceVfx.contains(name);
}

void MDLPart::addExistingModel(const QString &name, const Transformation &transformation) const
{
    const auto model = m_vkWindow->sourceModels[name];
    m_vkWindow->models.push_back(DrawObjectInstance{name, model, transformation});
}

#include "moc_mdlpart.cpp"
