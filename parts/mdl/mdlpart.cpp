#include "mdlpart.h"
#include "glm/gtx/transform.hpp"

#include <QWindow>
#include <QVulkanInstance>
#include <QVulkanWindow>
#include <QResizeEvent>
#include <fmt/core.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <QVBoxLayout>
#include <glm/gtc/type_ptr.inl>

#ifndef USE_STANDALONE_WINDOW
class VulkanWindow : public QWindow
{
public:
    VulkanWindow(MDLPart* part, Renderer* renderer, QVulkanInstance* instance) : part(part), m_renderer(renderer), m_instance(instance) {
        setSurfaceType(VulkanSurface);
        setVulkanInstance(instance);
    }

    void exposeEvent(QExposeEvent *) {
        if (isExposed()) {
            if (!m_initialized) {
                m_initialized = true;

                auto surface = m_instance->surfaceForWindow(this);
                if(!m_renderer->initSwapchain(surface, width(), height()))
                    m_initialized = false;
                else
                    render();
            }
        }
    }

    bool event(QEvent *e) {
        switch(e->type()) {
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
        case QEvent::Wheel:
        {
            auto scrollEvent = dynamic_cast<QWheelEvent*>(e);

            part->cameraDistance -= scrollEvent->angleDelta().y() / 120.0f; // FIXME: why 120?
        }
            break;
        }

        return QWindow::event(e);
    }

    void render() {
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
#include "standalonewindow.h"
#include "equipment.h"

#endif

MDLPart::MDLPart(GameData *data) : data(data) {
    auto viewportLayout = new QVBoxLayout();
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

void MDLPart::exportModel(const QString &fileName) {
    Assimp::Exporter exporter;

    aiScene scene;
    scene.mRootNode = new aiNode();

    // TODO: hardcoded to the first model for now
    scene.mRootNode->mNumChildren = models[0].model.lods[0].num_parts + 1; // plus one for the skeleton
    scene.mRootNode->mChildren = new aiNode*[scene.mRootNode->mNumChildren];

    scene.mNumMeshes = models[0].model.lods[0].num_parts;
    scene.mMeshes = new aiMesh*[scene.mNumMeshes];

    auto skeleton_node = new aiNode();
    skeleton_node->mName = "Skeleton";
    skeleton_node->mNumChildren = 1;
    skeleton_node->mChildren = new aiNode*[skeleton_node->mNumChildren];

    scene.mRootNode->mChildren[scene.mRootNode->mNumChildren - 1] = skeleton_node;

    std::vector<aiNode*> skeletonNodes;

    for(int i = 0; i < models[0].model.num_affected_bones; i++) {
        auto& node = skeletonNodes.emplace_back();
        node = new aiNode();
        node->mName = models[0].model.affected_bone_names[i];

        int real_bone_id = 0;
        for(int k = 0; k < skeleton->num_bones; k++) {
            if(strcmp(skeleton->bones[k].name, models[0].model.affected_bone_names[i]) == 0) {
                real_bone_id  = k;
            }
        }

        node->mChildren = new aiNode*[models[0].model.num_affected_bones];

        auto& real_bone = skeleton->bones[real_bone_id];
        memcpy(&node->mTransformation, glm::value_ptr(boneData[real_bone.index].finalTransform), sizeof(aiMatrix4x4));
    }

    // setup parenting
    for(int i = 0; i < models[0].model.num_affected_bones; i++) {
        int real_bone_id = 0;
        for(int k = 0; k < skeleton->num_bones; k++) {
            if(strcmp(skeleton->bones[k].name, models[0].model.affected_bone_names[i]) == 0) {
                real_bone_id  = k;
            }
        }

        auto& real_bone = skeleton->bones[real_bone_id];
        if(real_bone.parent_bone != nullptr) {
            for(int k = 0; k < models[0].model.num_affected_bones; k++) {
                if(strcmp(models[0].model.affected_bone_names[k], real_bone.parent_bone->name) == 0) {
                    skeletonNodes[i]->mParent = skeletonNodes[k];
                    skeletonNodes[k]->mChildren[skeletonNodes[k]->mNumChildren++] = skeletonNodes[i];
                }
            }
        }
    }

    skeleton_node->mChildren[0] = new aiNode();
    skeleton_node->mChildren[0]->mName = "root";
    skeleton_node->mChildren[0]->mChildren = new aiNode*[models[0].model.num_affected_bones];

    for(int i = 0; i < skeletonNodes.size(); i++) {
        if(skeletonNodes[i]->mParent == nullptr) {
            skeleton_node->mChildren[0]->mChildren[skeleton_node->mChildren[0]->mNumChildren++] = skeletonNodes[i];
        }
    }

    for(int i = 0; i < models[0].model.lods[0].num_parts; i++) {
        scene.mMeshes[i] = new aiMesh();
        scene.mMeshes[i]->mMaterialIndex = 0;

        auto& node = scene.mRootNode->mChildren[i];
        node = new aiNode();
        node->mNumMeshes = 1;
        node->mMeshes = new unsigned int [scene.mRootNode->mNumMeshes];
        node->mMeshes[0] = i;

        auto mesh = scene.mMeshes[i];
        mesh->mNumVertices = models[0].model.lods[0].parts[i].num_vertices;
        mesh->mVertices = new aiVector3D [mesh->mNumVertices];
        mesh->mNormals = new aiVector3D [mesh->mNumVertices];
        mesh->mTextureCoords[0] = new aiVector3D [mesh->mNumVertices];
        mesh->mNumUVComponents[0] = 2;

        for(int j = 0; j < mesh->mNumVertices; j++) {
            auto vertex = models[0].model.lods[0].parts[i].vertices[j];
            mesh->mVertices[j] = aiVector3D(vertex.position[0], vertex.position[1], vertex.position[2]);
            mesh->mNormals[j] = aiVector3D (vertex.normal[0], vertex.normal[1], vertex.normal[2]);
            mesh->mTextureCoords[0][j] = aiVector3D(vertex.uv[0], vertex.uv[1], 0.0f);
        }

        mesh->mNumBones = models[0].model.num_affected_bones;
        mesh->mBones = new aiBone*[mesh->mNumBones];
        for(int j = 0; j < mesh->mNumBones; j++) {
            int real_bone_id = j;
            // TODO: is this still relevant?5
            /*for(int k = 0; k < skeleton.bones.size(); k++) {
                if(skeleton.bones[k].name == model.affectedBoneNames[j]) {
                    real_bone_id  = k;
                }
            }*/

            mesh->mBones[j] = new aiBone();
            mesh->mBones[j]->mName = models[0].model.affected_bone_names[j];
            mesh->mBones[j]->mNumWeights = mesh->mNumVertices * 4;
            mesh->mBones[j]->mWeights = new aiVertexWeight[mesh->mBones[j]->mNumWeights];
            // mesh->mBones[j]->mNode = skeleton_node->mChildren[j];

            for(int k = 0; k < mesh->mNumVertices; k++) {
                for(int z = 0; z < 4; z++) {
                    if (models[0].model.lods[0].parts[i].vertices[k].bone_id[z] == real_bone_id) {
                        auto &weight = mesh->mBones[j]->mWeights[k * 4 + z];
                        weight.mVertexId = k;
                        weight.mWeight = models[0].model.lods[0].parts[i].vertices[k].bone_weight[z];
                    }
                }
            }
        }

        mesh->mNumFaces = models[0].model.lods[0].parts[i].num_indices / 3;
        mesh->mFaces = new aiFace[mesh->mNumFaces];

        int lastFace = 0;
        for(int j = 0; j < models[0].model.lods[0].parts[i].num_indices; j += 3) {
            aiFace& face = mesh->mFaces[lastFace++];

            face.mNumIndices = 3;
            face.mIndices = new unsigned int[face.mNumIndices];

            face.mIndices[0] = models[0].model.lods[0].parts[i].indices[j];
            face.mIndices[1] = models[0].model.lods[0].parts[i].indices[j + 1];
            face.mIndices[2] = models[0].model.lods[0].parts[i].indices[j + 2];
        }
    }

    scene.mNumMaterials = 1;
    scene.mMaterials = new aiMaterial*[1];
    scene.mMaterials[0] = new aiMaterial();

    exporter.Export(&scene, "fbx", fileName.toStdString());
}

void MDLPart::clear() {
    models.clear();

    Q_EMIT modelChanged();
}

void MDLPart::addModel(physis_MDL mdl, std::vector<physis_Material> materials, int lod) {
    qDebug() << "Adding model to MDLPart";

    auto model = renderer->addModel(mdl, lod);

    std::transform(materials.begin(), materials.end(), std::back_inserter(model.materials), [this](const physis_Material& mat) {
        return createMaterial(mat);
    });

    models.push_back(model);

    Q_EMIT modelChanged();
}

void MDLPart::setSkeleton(physis_Skeleton newSkeleton) {
    skeleton = std::make_unique<physis_Skeleton>(newSkeleton);

    firstTimeSkeletonDataCalculated = false;

    Q_EMIT skeletonChanged();
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
    if(skeleton) {
        // first-time data, TODO split out
        if (!firstTimeSkeletonDataCalculated) {
            boneData.resize(skeleton->num_bones);
            calculateBoneInversePose(*skeleton, *skeleton->root_bone, nullptr);

            for (auto &bone: boneData) {
                bone.inversePose = glm::inverse(bone.inversePose);
            }
            firstTimeSkeletonDataCalculated = true;
        }

        // update data
        calculateBone(*skeleton, *skeleton->root_bone, nullptr);

        for(auto& model : models) {
            // we want to map the actual affected bones to bone ids
            std::map<int, int> boneMapping;
            for (int i = 0; i < model.model.num_affected_bones; i++) {
                for (int k = 0; k < skeleton->num_bones; k++) {
                    if (std::string_view{skeleton->bones[k].name} == std::string_view{model.model.affected_bone_names[i]}) {
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

RenderMaterial MDLPart::createMaterial(const physis_Material &material) {
    RenderMaterial newMaterial;

    for (int i = 0; i < material.num_textures; i++) {
        std::string t = material.textures[i];

        if (t.find("skin") != std::string::npos) {
            newMaterial.type = MaterialType::Skin;
        }

        char type = t[t.length() - 5];

        switch(type) {
            case 'm': {
                auto texture = physis_texture_parse(physis_gamedata_extract_file(data, material.textures[i]));
                auto tex = renderer->addTexture(texture.width, texture.height, texture.rgba, texture.rgba_size);

                newMaterial.multiTexture = new RenderTexture(tex);
            }
            case 'd': {
                auto texture = physis_texture_parse(physis_gamedata_extract_file(data, material.textures[i]));
                auto tex = renderer->addTexture(texture.width, texture.height, texture.rgba, texture.rgba_size);

                newMaterial.diffuseTexture = new RenderTexture(tex);
            }
                break;
            case 'n': {
                auto texture = physis_texture_parse(physis_gamedata_extract_file(data, material.textures[i]));
                auto tex = renderer->addTexture(texture.width, texture.height, texture.rgba, texture.rgba_size);

                newMaterial.normalTexture = new RenderTexture(tex);
            }
                break;
            case 's': {
                auto texture = physis_texture_parse(physis_gamedata_extract_file(data, material.textures[i]));
                auto tex = renderer->addTexture(texture.width, texture.height, texture.rgba, texture.rgba_size);

                newMaterial.specularTexture = new RenderTexture(tex);
            }
                break;
            default:
                qDebug() << "unhandled type" << type;
                break;
        }
    }

    return newMaterial;
}

void MDLPart::calculateBoneInversePose(physis_Skeleton& skeleton, physis_Bone& bone, physis_Bone* parent_bone) {
    const glm::mat4 parentMatrix = parent_bone == nullptr ? glm::mat4(1.0f) : boneData[parent_bone->index].inversePose;

    glm::mat4 local(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    boneData[bone.index].inversePose = parentMatrix * local;

    for(int i = 0; i < skeleton.num_bones; i++) {
        if(skeleton.bones[i].parent_bone != nullptr && std::string_view{skeleton.bones[i].parent_bone->name} == std::string_view{bone.name}) {
            calculateBoneInversePose(skeleton, skeleton.bones[i], &bone);
        }
    }
}

void MDLPart::calculateBone(physis_Skeleton& skeleton, physis_Bone& bone, const physis_Bone* parent_bone) {
    const glm::mat4 parent_matrix = parent_bone == nullptr ? glm::mat4(1.0f) : boneData[parent_bone->index].localTransform;

    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    boneData[bone.index].localTransform = parent_matrix * local;
    boneData[bone.index].finalTransform = boneData[bone.index].localTransform * boneData[bone.index].inversePose;

    for(int i = 0; i < skeleton.num_bones; i++) {
        if(skeleton.bones[i].parent_bone != nullptr && std::string_view{skeleton.bones[i].parent_bone->name} == std::string_view{bone.name}) {
            calculateBone(skeleton, skeleton.bones[i], &bone);
        }
    }
}

#include "moc_mdlpart.cpp"