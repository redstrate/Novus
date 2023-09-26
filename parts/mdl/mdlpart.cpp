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
#include <QWindow>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.inl>

#include "filecache.h"
#include "tiny_gltf.h"

#ifndef USE_STANDALONE_WINDOW
class VulkanWindow : public QWindow {
public:
    VulkanWindow(MDLPart* part, Renderer* renderer, QVulkanInstance* instance)
        : part(part), m_renderer(renderer), m_instance(instance) {
        setSurfaceType(VulkanSurface);
        setVulkanInstance(instance);
    }

    void exposeEvent(QExposeEvent*) {
        if (isExposed()) {
            if (!m_initialized) {
                m_initialized = true;

                auto surface = m_instance->surfaceForWindow(this);
                if (!m_renderer->initSwapchain(surface, width(), height()))
                    m_initialized = false;
                else
                    render();
            }
        }
    }

    bool event(QEvent* e) {
        switch (e->type()) {
            case QEvent::UpdateRequest:
                render();
                break;
            case QEvent::Resize: {
                QResizeEvent* resizeEvent = (QResizeEvent*)e;
                auto surface = m_instance->surfaceForWindow(this);
                m_renderer->resize(surface, resizeEvent->size().width(), resizeEvent->size().height());
            } break;
            case QEvent::MouseButtonPress: {
                auto mouseEvent = dynamic_cast<QMouseEvent*>(e);

                if (mouseEvent->button() == Qt::MouseButton::LeftButton) {
                    part->lastX = mouseEvent->x();
                    part->lastY = mouseEvent->y();
                    part->cameraMode = MDLPart::CameraMode::Orbit;

                    setKeyboardGrabEnabled(true);
                    setMouseGrabEnabled(true);
                } else if (mouseEvent->button() == Qt::MouseButton::RightButton) {
                    part->lastX = mouseEvent->x();
                    part->lastY = mouseEvent->y();
                    part->cameraMode = MDLPart::CameraMode::Move;

                    setKeyboardGrabEnabled(true);
                    setMouseGrabEnabled(true);
                }
            } break;
            case QEvent::MouseButtonRelease: {
                part->cameraMode = MDLPart::CameraMode::None;

                setKeyboardGrabEnabled(false);
                setMouseGrabEnabled(false);
            } break;
            case QEvent::MouseMove: {
                auto mouseEvent = dynamic_cast<QMouseEvent*>(e);
                if (part->cameraMode != MDLPart::CameraMode::None) {
                    const int deltaX = mouseEvent->x() - part->lastX;
                    const int deltaY = mouseEvent->y() - part->lastY;

                    if (part->cameraMode == MDLPart::CameraMode::Orbit) {
                        part->yaw += deltaX * 0.01f; // TODO: remove these magic numbers
                        part->pitch += deltaY * 0.01f;
                    } else {
                        glm::vec3 position(
                            part->cameraDistance * sin(part->yaw),
                            part->cameraDistance * part->pitch,
                            part->cameraDistance * cos(part->yaw));

                        glm::quat rot = glm::quatLookAt((part->position + position) - part->position, {0, 1, 0});

                        glm::vec3 up, right;
                        up = rot * glm::vec3{0, 1, 0};
                        right = rot * glm::vec3{1, 0, 0};

                        part->position += up * (float)deltaY * 0.01f;
                        part->position += right * (float)deltaX * 0.01f;
                    }

                    part->lastX = mouseEvent->x();
                    part->lastY = mouseEvent->y();
                }
            } break;
            case QEvent::Wheel: {
                auto scrollEvent = dynamic_cast<QWheelEvent*>(e);

                part->cameraDistance -= scrollEvent->angleDelta().y() / 120.0f; // FIXME: why 120?
            } break;
        }

        return QWindow::event(e);
    }

    void render() {
        if (part->requestUpdate)
                part->requestUpdate();

        glm::vec3 position(
            part->cameraDistance * sin(part->yaw),
            part->cameraDistance * part->pitch,
            part->cameraDistance * cos(part->yaw));

        m_renderer->view = glm::lookAt(part->position + position, part->position, glm::vec3(0, -1, 0));

        m_renderer->render(models);
        m_instance->presentQueued(this);
        requestUpdate();
    }

    std::vector<RenderModel> models;

private:
    bool m_initialized = false;
    Renderer* m_renderer;
    QVulkanInstance* m_instance;
    MDLPart* part;
};
#else
    #include "equipment.h"
    #include "standalonewindow.h"

#endif

MDLPart::MDLPart(GameData* data, FileCache& cache) : data(data), cache(cache) {
    auto viewportLayout = new QVBoxLayout();
    viewportLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(viewportLayout);

    renderer = new Renderer();

#ifndef USE_STANDALONE_WINDOW
    auto inst = new QVulkanInstance();
    inst->setVkInstance(renderer->instance);
    inst->setFlags(QVulkanInstance::Flag::NoDebugOutputRedirect);
    inst->create();

    vkWindow = new VulkanWindow(this, renderer, inst);
    vkWindow->setVulkanInstance(inst);

    auto widget = QWidget::createWindowContainer(vkWindow);

    viewportLayout->addWidget(widget);
#else
    standaloneWindow = new StandaloneWindow(renderer);
    renderer->initSwapchain(standaloneWindow->getSurface(renderer->instance), 640, 480);

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, [this] {
        standaloneWindow->render();
    });
    timer->start(1000);
#endif

    connect(this, &MDLPart::modelChanged, this, &MDLPart::reloadRenderer);
    connect(this, &MDLPart::skeletonChanged, this, &MDLPart::reloadBoneData);
}

void MDLPart::exportModel(const QString& fileName) {
    const int selectedModel = 0;
    const int selectedLod = 0;

    const physis_MDL& model = models[selectedModel].model;
    const physis_LOD& lod = model.lods[selectedLod];

    tinygltf::Model gltfModel;
    gltfModel.asset.generator = "Novus";

    auto& gltfSkeletonNode = gltfModel.nodes.emplace_back();
    gltfSkeletonNode.name = skeleton->root_bone->name;

    for (int i = 0; i < model.num_affected_bones; i++) {
        auto& node = gltfModel.nodes.emplace_back();
        node.name = model.affected_bone_names[i];

        int real_bone_id = 0;
        for (int k = 0; k < skeleton->num_bones; k++) {
            if (strcmp(skeleton->bones[k].name, model.affected_bone_names[i]) == 0) {
                real_bone_id = k;
            }
        }

        auto& real_bone = skeleton->bones[real_bone_id];
        node.translation = {real_bone.position[0], real_bone.position[1], real_bone.position[2]};
        node.rotation = {real_bone.rotation[0], real_bone.rotation[1], real_bone.rotation[2], real_bone.rotation[3]};
        node.scale = {real_bone.scale[0], real_bone.scale[1], real_bone.scale[2]};
    }

    // setup parenting
    for (int i = 0; i < model.num_affected_bones; i++) {
        int real_bone_id = 0;
        for (int k = 0; k < skeleton->num_bones; k++) {
            if (strcmp(skeleton->bones[k].name, model.affected_bone_names[i]) == 0) {
                real_bone_id = k;
            }
        }

        auto& real_bone = skeleton->bones[real_bone_id];
        if (real_bone.parent_bone != nullptr) {
            for (int k = 0; k < model.num_affected_bones; k++) {
                if (strcmp(model.affected_bone_names[k], real_bone.parent_bone->name) == 0) {
                    gltfModel.nodes[k + 1].children.push_back(i + 1); // +1 for the skeleton node taking up the first index
                }
            }
        } else {
            gltfSkeletonNode.children.push_back(i + 1);
        }
    }

    auto& gltfSkin = gltfModel.skins.emplace_back();
    gltfSkin.name = gltfSkeletonNode.name;
    for (int i = 1; i < gltfModel.nodes.size(); i++) {
        gltfSkin.joints.push_back(i);
    }

    // Inverse bind matrices
    {
        gltfSkin.inverseBindMatrices = gltfModel.accessors.size();

        auto& inverseAccessor = gltfModel.accessors.emplace_back();
        inverseAccessor.bufferView = gltfModel.bufferViews.size();
        inverseAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        inverseAccessor.count = model.num_affected_bones;
        inverseAccessor.type = TINYGLTF_TYPE_MAT4;

        auto& inverseBufferView = gltfModel.bufferViews.emplace_back();
        inverseBufferView.buffer = gltfModel.buffers.size();

        auto& inverseBuffer = gltfModel.buffers.emplace_back();
        for (int i = 0; i < model.num_affected_bones; i++) {
            int real_bone_id = 0;
            for (int k = 0; k < skeleton->num_bones; k++) {
                if (strcmp(skeleton->bones[k].name, model.affected_bone_names[i]) == 0) {
                    real_bone_id = k;
                }
            }

            auto& real_bone = skeleton->bones[real_bone_id];
            auto inverseMatrix = boneData[real_bone.index].inversePose;
            auto inverseMatrixCPtr = reinterpret_cast<uint8_t*>(glm::value_ptr(inverseMatrix));

            inverseBuffer.data.insert(inverseBuffer.data.end(), inverseMatrixCPtr, inverseMatrixCPtr + sizeof(float) * 16);
        }

        inverseBufferView.byteLength = inverseBuffer.data.size();
    }

    for (int i = 0; i < lod.num_parts; i++) {
        auto& gltfNode = gltfModel.nodes.emplace_back();;
        gltfNode.name = "placeholder Part 0.1";
        gltfNode.skin = 0;

        gltfNode.mesh = gltfModel.meshes.size();
        auto& gltfMesh = gltfModel.meshes.emplace_back();

        gltfMesh.name = "what?";

        auto& gltfPrimitive = gltfMesh.primitives.emplace_back();
        gltfPrimitive.attributes["POSITION"] = gltfModel.accessors.size();
        gltfPrimitive.attributes["TEXCOORD_0"] = gltfModel.accessors.size() + 1;
        gltfPrimitive.attributes["NORMAL"] = gltfModel.accessors.size() + 2;
        gltfPrimitive.attributes["WEIGHTS_0"] = gltfModel.accessors.size() + 3;
        gltfPrimitive.attributes["JOINTS_0"] = gltfModel.accessors.size() + 4;
        gltfPrimitive.mode = TINYGLTF_MODE_TRIANGLES;

        // Vertices
        {
            auto& positionAccessor = gltfModel.accessors.emplace_back();
            positionAccessor.bufferView = gltfModel.bufferViews.size();
            positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            positionAccessor.count = lod.parts[i].num_vertices;
            positionAccessor.type = TINYGLTF_TYPE_VEC3;

            auto& uvAccessor = gltfModel.accessors.emplace_back();
            uvAccessor.bufferView = gltfModel.bufferViews.size();
            uvAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            uvAccessor.count = lod.parts[i].num_vertices;
            uvAccessor.type = TINYGLTF_TYPE_VEC2;
            uvAccessor.byteOffset = offsetof(Vertex, uv);

            auto& normalAccessor = gltfModel.accessors.emplace_back();
            normalAccessor.bufferView = gltfModel.bufferViews.size();
            normalAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            normalAccessor.count = lod.parts[i].num_vertices;
            normalAccessor.type = TINYGLTF_TYPE_VEC3;
            normalAccessor.byteOffset = offsetof(Vertex, normal);

            auto& boneWeightAccessor = gltfModel.accessors.emplace_back();
            boneWeightAccessor.bufferView = gltfModel.bufferViews.size();
            boneWeightAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            boneWeightAccessor.count = lod.parts[i].num_vertices;
            boneWeightAccessor.type = TINYGLTF_TYPE_VEC4;
            boneWeightAccessor.byteOffset = offsetof(Vertex, bone_weight);

            auto& boneIdAccessor = gltfModel.accessors.emplace_back();
            boneIdAccessor.bufferView = gltfModel.bufferViews.size();
            boneIdAccessor.componentType = TINYGLTF_COMPONENT_TYPE_BYTE;
            boneIdAccessor.count = lod.parts[i].num_vertices;
            boneIdAccessor.type = TINYGLTF_TYPE_VEC4;
            boneIdAccessor.byteOffset = offsetof(Vertex, bone_id);

            auto& vertexBufferView = gltfModel.bufferViews.emplace_back();
            vertexBufferView.buffer = gltfModel.buffers.size();
            vertexBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            auto& vertexBuffer = gltfModel.buffers.emplace_back();
            vertexBuffer.data.resize(lod.parts[i].num_vertices * sizeof(Vertex));
            memcpy(vertexBuffer.data.data(), lod.parts[i].vertices, vertexBuffer.data.size());

            vertexBufferView.byteLength = vertexBuffer.data.size();
            vertexBufferView.byteStride = sizeof(Vertex);
        }

        // Indices
        {
            gltfPrimitive.indices = gltfModel.accessors.size();
            auto& indexAccessor = gltfModel.accessors.emplace_back();
            indexAccessor.bufferView = gltfModel.bufferViews.size();
            indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
            indexAccessor.count = lod.parts[i].num_indices;
            indexAccessor.type = TINYGLTF_TYPE_SCALAR;

            auto& indexBufferView = gltfModel.bufferViews.emplace_back();
            indexBufferView.buffer = gltfModel.buffers.size();
            indexBufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

            auto& indexBuffer = gltfModel.buffers.emplace_back();
            indexBuffer.data.resize(lod.parts[i].num_indices * sizeof(uint16_t));
            memcpy(indexBuffer.data.data(), lod.parts[i].indices, indexBuffer.data.size());

            indexBufferView.byteLength = indexBuffer.data.size();
            indexBufferView.byteStride = sizeof(uint16_t);
        }
    }

    tinygltf::TinyGLTF loader;
    loader.WriteGltfSceneToFile(&gltfModel, fileName.toStdString(), true, true, false, true);
}

void MDLPart::clear() {
    models.clear();

    Q_EMIT modelChanged();
}

void MDLPart::addModel(physis_MDL mdl, std::vector<physis_Material> materials, int lod) {
    qDebug() << "Adding model to MDLPart";

    auto model = renderer->addModel(mdl, lod);

    std::transform(
        materials.begin(), materials.end(), std::back_inserter(model.materials), [this](const physis_Material& mat) {
            return createMaterial(mat);
        });

    if (materials.empty()) {
        model.materials.push_back(createMaterial(physis_Material{}));
    }

    models.push_back(model);

    Q_EMIT modelChanged();
}

void MDLPart::setSkeleton(physis_Skeleton newSkeleton) {
    skeleton = std::make_unique<physis_Skeleton>(newSkeleton);

    firstTimeSkeletonDataCalculated = false;

    Q_EMIT skeletonChanged();
}

void MDLPart::loadRaceDeformMatrices(physis_Buffer buffer) {
    QJsonDocument document = QJsonDocument::fromJson(QByteArray((const char*)buffer.data, buffer.size));
    for (auto boneObj : document.object()[QLatin1String("Data")].toArray()) {
        QJsonArray matrix = boneObj.toObject()[QLatin1String("Matrix")].toArray();
        QString boneName = boneObj.toObject()[QLatin1String("Name")].toString();

        glm::mat4 actualMatrix;
        int i = 0;
        for (auto val : matrix) {
            glm::value_ptr(actualMatrix)[i++] = val.toDouble();
        }

        for (int i = 0; i < skeleton->num_bones; i++) {
            if (std::string_view{skeleton->bones[i].name} == boneName.toStdString()) {
                auto& data = boneData[i];

                data.deformRaceMatrix = actualMatrix;
            }
        }

        firstTimeSkeletonDataCalculated = false;
    }
}

void MDLPart::clearSkeleton() {
    skeleton.reset();

    firstTimeSkeletonDataCalculated = false;

    Q_EMIT skeletonChanged();
}

void MDLPart::reloadRenderer() {
    reloadBoneData();

#ifndef USE_STANDALONE_WINDOW
    vkWindow->models = models;
#else
    standaloneWindow->models = models;
#endif
}

void MDLPart::reloadBoneData() {
    if (skeleton) {
        if (!firstTimeSkeletonDataCalculated) {
            if (boneData.empty()) {
                boneData.resize(skeleton->num_bones);
            }

            calculateBoneInversePose(*skeleton, *skeleton->root_bone, nullptr);

            for (auto& bone : boneData) {
                bone.inversePose = glm::inverse(bone.inversePose);
            }
            firstTimeSkeletonDataCalculated = true;
        }

        // update data
        calculateBone(*skeleton, *skeleton->root_bone, nullptr);

        for (auto& model : models) {
            // we want to map the actual affected bones to bone ids
            std::map<int, int> boneMapping;
            for (int i = 0; i < model.model.num_affected_bones; i++) {
                for (int k = 0; k < skeleton->num_bones; k++) {
                    if (std::string_view{skeleton->bones[k].name} ==
                        std::string_view{model.model.affected_bone_names[i]}) {
                        boneMapping[i] = k;
                    }
                }
            }

            for (int i = 0; i < model.model.num_affected_bones; i++) {
                model.boneData[i] = boneData[boneMapping[i]].finalTransform;
            }
        }
    }
}

RenderMaterial MDLPart::createMaterial(const physis_Material& material) {
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

void MDLPart::calculateBoneInversePose(physis_Skeleton& skeleton, physis_Bone& bone, physis_Bone* parent_bone) {
    const glm::mat4 parentMatrix = parent_bone == nullptr ? glm::mat4(1.0f) : boneData[parent_bone->index].inversePose;

    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    boneData[bone.index].inversePose = parentMatrix * local;

    for (int i = 0; i < skeleton.num_bones; i++) {
        if (skeleton.bones[i].parent_bone != nullptr &&
            std::string_view{skeleton.bones[i].parent_bone->name} == std::string_view{bone.name}) {
            calculateBoneInversePose(skeleton, skeleton.bones[i], &bone);
        }
    }
}

void MDLPart::calculateBone(physis_Skeleton& skeleton, physis_Bone& bone, const physis_Bone* parent_bone) {
    const glm::mat4 parent_matrix =
        parent_bone == nullptr ? glm::mat4(1.0f) : boneData[parent_bone->index].localTransform;

    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    boneData[bone.index].localTransform = parent_matrix * local;
    boneData[bone.index].finalTransform =
        boneData[bone.index].localTransform * boneData[bone.index].deformRaceMatrix * boneData[bone.index].inversePose;

    for (int i = 0; i < skeleton.num_bones; i++) {
        if (skeleton.bones[i].parent_bone != nullptr &&
            std::string_view{skeleton.bones[i].parent_bone->name} == std::string_view{bone.name}) {
            calculateBone(skeleton, skeleton.bones[i], &bone);
        }
    }
}

void MDLPart::removeModel(const physis_MDL &mdl)
{
    models.erase(std::remove_if(models.begin(),
                                models.end(),
                                [mdl](const RenderModel other) {
                                    return mdl.lods == other.model.lods;
                                }),
                 models.end());
}

#include "moc_mdlpart.cpp"