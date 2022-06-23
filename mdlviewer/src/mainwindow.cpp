#include "mainwindow.h"

#include <QWindow>
#include <QVulkanInstance>
#include <QHBoxLayout>
#include <QTableWidget>
#include <fmt/core.h>
#include <QListWidget>
#include <QVulkanWindow>
#include <QLineEdit>
#include <QResizeEvent>
#include <QComboBox>
#include <QTimer>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <QPushButton>
#include <QFileDialog>
#include <magic_enum.hpp>
#include <glm/gtc/quaternion.hpp>
#include <QMenuBar>
#include <QAction>
#include <glm/gtc/type_ptr.hpp>
#include <QTreeWidget>

#include "gamedata.h"
#include "exhparser.h"
#include "exdparser.h"
#include "mdlparser.h"
#include "equipment.h"
#include "glm/glm.hpp"
#include "vec3edit.h"

#ifndef USE_STANDALONE_WINDOW
class VulkanWindow : public QWindow
{
public:
    VulkanWindow(Renderer* renderer, QVulkanInstance* instance) : m_renderer(renderer), m_instance(instance) {
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
        if (e->type() == QEvent::UpdateRequest)
            render();

        if (e->type() == QEvent::Resize) {
            QResizeEvent* resizeEvent = (QResizeEvent*)e;
            auto surface = m_instance->surfaceForWindow(this);
            m_renderer->resize(surface, resizeEvent->size().width(), resizeEvent->size().height());
        }

        return QWindow::event(e);
    }

    void render() {
        m_renderer->render(models);
        m_instance->presentQueued(this);
        requestUpdate();
    }

    std::vector<RenderModel> models;

private:
    bool m_initialized = false;
    Renderer* m_renderer;
    QVulkanInstance* m_instance;
};
#else
#include "standalonewindow.h"
#include "equipment.h"

#endif

void calculate_bone_inverse_pose(Skeleton& skeleton, Bone& bone, Bone* parent_bone) {
    const glm::mat4 parentMatrix = parent_bone == nullptr ? glm::mat4(1.0f) : parent_bone->inversePose;

    glm::mat4 local(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    bone.inversePose = parentMatrix * local;

    for(auto& b : skeleton.bones) {
        if(b.parent != nullptr && b.parent->name == bone.name)
            calculate_bone_inverse_pose(skeleton, b, &bone);
    }
}

void addItem(Skeleton& skeleton, Bone& bone, QTreeWidget* widget, QTreeWidgetItem* parent_item = nullptr) {
    auto item = new QTreeWidgetItem();
    item->setText(0, bone.name.c_str());

    if(parent_item == nullptr) {
        widget->addTopLevelItem(item);
    } else {
        parent_item->addChild(item);
    }

    for(auto& b : skeleton.bones) {
        if(b.parent != nullptr && b.parent->name == bone.name)
            addItem(skeleton, b, widget, item);
    }
}

MainWindow::MainWindow(GameData& data) : data(data) {
    setWindowTitle("mdlviewer");
    setMinimumSize(QSize(640, 480));

    auto fileMenu = menuBar()->addMenu("File");

    auto openMDLFile = fileMenu->addAction("Open MDL...");
    connect(openMDLFile, &QAction::triggered, [=] {
        auto fileName = QFileDialog::getOpenFileName(nullptr,
                                                     "Open MDL File",
                                                     "~",
                                                     "FFXIV Model File (*.mdl)");

        loadedGear.model = parseMDL(read_file_to_buffer(fileName.toStdString()));

        reloadGearAppearance();
    });

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    // smallclothes body
    {
        GearInfo info = {};
        info.name = "Smallclothes Body";
        info.slot = Slot::Body;

        gears.push_back(info);
    }

    // smallclothes legs
    {
        GearInfo info = {};
        info.name = "Smallclothes Legs";
        info.slot = Slot::Legs;

        gears.push_back(info);
    }

    auto exh = *data.readExcelSheet("Item");

    auto path = getEXDFilename(exh, "item", getLanguageCode(Language::English), exh.pages[1]);
    auto exd = readEXD(exh, *data.extractFile("exd/" + path), exh.pages[1]);
    for(auto row : exd.rows) {
        auto primaryModel = row.data[47].uint64Data;
        auto secondaryModel = row.data[48].uint64Data;

        int16_t parts[4];
        memcpy(parts, &primaryModel, sizeof(int16_t) * 4);

        GearInfo info = {};
        info.name = row.data[9].data;
        info.slot = *get_slot_from_id(row.data[17].uint64Data);
        info.modelInfo.primaryID = parts[0];

        gears.push_back(info);
    }

    auto listWidget = new QListWidget();
    for(auto gear : gears)
        listWidget->addItem(gear.name.c_str());

    listWidget->setMaximumWidth(200);

    layout->addWidget(listWidget);

    renderer = new Renderer();

    auto viewportLayout = new QVBoxLayout();
    layout->addLayout(viewportLayout);

#ifndef USE_STANDALONE_WINDOW
    auto inst = new QVulkanInstance();
    inst->setVkInstance(renderer->instance);
    inst->setFlags(QVulkanInstance::Flag::NoDebugOutputRedirect);
    inst->create();

    vkWindow = new VulkanWindow(renderer, inst);
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

    QHBoxLayout* controlLayout = new QHBoxLayout();
    viewportLayout->addLayout(controlLayout);

    raceCombo = new QComboBox();

    connect(raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        currentRace = (Race)index;
        reloadGearModel();
    });

    controlLayout->addWidget(raceCombo);

    lodCombo = new QComboBox();

    connect(lodCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        currentLod = index;
        reloadGearAppearance();
    });

    controlLayout->addWidget(lodCombo);

    QPushButton* exportButton = new QPushButton("Export");

    connect(exportButton, &QPushButton::clicked, [this] {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Model"),
                                                        "model.fbx",
                                                        tr("FBX Files (*.fbx)"));

        Model model;
#ifdef USE_STANDALONE_WINDOW
        model = standaloneWindow->models[0].model;
#else
        model = vkWindow->models[0].model;
#endif
        exportModel(model, skeleton, fileName);
    });

    controlLayout->addWidget(exportButton);

    connect(listWidget, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        for(auto& gear : gears) {
            if(gear.name == item->text().toStdString()) {
                loadInitialGearInfo(gear);
                return;
            }
        }
    });

    skeleton = parseHavokXML("test.xml");
    calculate_bone_inverse_pose(skeleton, *skeleton.root_bone, nullptr);

    auto boneListWidget = new QTreeWidget();
    for(auto& bone : skeleton.bones) {
        bone.inversePose = glm::inverse(bone.inversePose);
    }

    addItem(skeleton, *skeleton.root_bone, boneListWidget);

    boneListWidget->setMaximumWidth(200);

    connect(boneListWidget, &QTreeWidget::itemClicked, [this](QTreeWidgetItem* item, int column) {
        for(auto& bone : skeleton.bones) {
            if(bone.name == item->text(column).toStdString()) {
                currentScale = glm::make_vec3(bone.scale.data());
                currentEditedBone = &bone;
            }
        }
    });

    layout->addWidget(boneListWidget);

    Vector3Edit* scaleEdit = new Vector3Edit(currentScale);
    connect(scaleEdit, &Vector3Edit::onValueChanged, [this] {
        memcpy(currentEditedBone->scale.data(), glm::value_ptr(currentScale), sizeof(float) * 3);
        reloadGearAppearance();
    });
    layout->addWidget(scaleEdit);
}

void MainWindow::exportModel(Model& model, Skeleton& skeleton, QString fileName) {
    Assimp::Exporter exporter;

    aiScene scene;
    scene.mRootNode = new aiNode();

    scene.mRootNode->mNumChildren = model.lods[0].parts.size() + 1; // plus one for the skeleton
    scene.mRootNode->mChildren = new aiNode*[scene.mRootNode->mNumChildren];

    scene.mNumMeshes = model.lods[0].parts.size();
    scene.mMeshes = new aiMesh*[scene.mNumMeshes];

    auto skeleton_node = new aiNode();
    skeleton_node->mName = "Skeleton";
    skeleton_node->mNumChildren = 1;
    skeleton_node->mChildren = new aiNode*[skeleton_node->mNumChildren];

    scene.mRootNode->mChildren[scene.mRootNode->mNumChildren - 1] = skeleton_node;

    std::vector<aiNode*> skeletonNodes;

    for(int i = 0; i < model.affectedBoneNames.size(); i++) {
        auto& node = skeletonNodes.emplace_back();
        node = new aiNode();
        node->mName = model.affectedBoneNames[i];

        int real_bone_id = 0;
        for(int k = 0; k < skeleton.bones.size(); k++) {
            if(skeleton.bones[k].name == model.affectedBoneNames[i]) {
                real_bone_id  = k;
            }
        }

        node->mChildren = new aiNode*[model.affectedBoneNames.size()];

        auto& real_bone = skeleton.bones[real_bone_id];
        memcpy(&node->mTransformation, glm::value_ptr(real_bone.finalTransform), sizeof(aiMatrix4x4));
    }

    // setup parenting
    for(int i = 0; i < model.affectedBoneNames.size(); i++) {
        int real_bone_id = 0;
        for(int k = 0; k < skeleton.bones.size(); k++) {
            if(skeleton.bones[k].name == model.affectedBoneNames[i]) {
                real_bone_id  = k;
            }
        }

        auto& real_bone = skeleton.bones[real_bone_id];
        if(real_bone.parent != nullptr) {
            for(int k = 0; k < model.affectedBoneNames.size(); k++) {
                if(model.affectedBoneNames[k] == real_bone.parent->name) {
                    skeletonNodes[i]->mParent = skeletonNodes[k];
                    skeletonNodes[k]->mChildren[skeletonNodes[k]->mNumChildren++] = skeletonNodes[i];
                }
            }
        }
    }

    skeleton_node->mChildren[0] = new aiNode();
    skeleton_node->mChildren[0]->mName = "root";
    skeleton_node->mChildren[0]->mChildren = new aiNode*[model.affectedBoneNames.size()];

    for(int i = 0; i < skeletonNodes.size(); i++) {
      if(skeletonNodes[i]->mParent == nullptr) {
        skeleton_node->mChildren[0]->mChildren[skeleton_node->mChildren[0]->mNumChildren++] = skeletonNodes[i];
      }
    }

    for(int i = 0; i < model.lods[0].parts.size(); i++) {
        scene.mMeshes[i] = new aiMesh();
        scene.mMeshes[i]->mMaterialIndex = 0;

        auto& node = scene.mRootNode->mChildren[i];
        node = new aiNode();
        node->mNumMeshes = 1;
        node->mMeshes = new unsigned int [scene.mRootNode->mNumMeshes];
        node->mMeshes[0] = i;

        auto mesh = scene.mMeshes[i];
        mesh->mNumVertices = model.lods[0].parts[i].vertices.size();
        mesh->mVertices = new aiVector3D [mesh->mNumVertices];
        mesh->mNormals = new aiVector3D [mesh->mNumVertices];
        mesh->mTextureCoords[0] = new aiVector3D [mesh->mNumVertices];
        mesh->mNumUVComponents[0] = 2;
        for(int j = 0; j < mesh->mNumVertices; j++) {
            auto vertex = model.lods[0].parts[i].vertices[j];
            mesh->mVertices[j] = aiVector3D(vertex.position[0], vertex.position[1], vertex.position[2]);
            mesh->mNormals[j] = aiVector3D (vertex.normal[0], vertex.normal[1], vertex.normal[2]);
            mesh->mTextureCoords[0][j] = aiVector3D(vertex.uv[0], vertex.uv[1], 0.0f);
        }

        mesh->mNumBones = model.affectedBoneNames.size();
        mesh->mBones = new aiBone*[mesh->mNumBones];
        for(int j = 0; j < mesh->mNumBones; j++) {
            int real_bone_id = j;
            /*for(int k = 0; k < skeleton.bones.size(); k++) {
                if(skeleton.bones[k].name == model.affectedBoneNames[j]) {
                    real_bone_id  = k;
                }
            }*/

            mesh->mBones[j] = new aiBone();
            mesh->mBones[j]->mName = model.affectedBoneNames[j];
            mesh->mBones[j]->mNumWeights = mesh->mNumVertices * 4;
            mesh->mBones[j]->mWeights = new aiVertexWeight[mesh->mBones[j]->mNumWeights];
            mesh->mBones[j]->mNode = skeleton_node->mChildren[j];

            for(int k = 0; k < mesh->mNumVertices; k++) {
                for(int z = 0; z < 4; z++) {
                    if (model.lods[0].parts[i].vertices[k].boneIds[z] == real_bone_id) {
                        auto &weight = mesh->mBones[j]->mWeights[k * 4 + z];
                        weight.mVertexId = k;
                        weight.mWeight =  model.lods[0].parts[i].vertices[k].boneWeights[z];
                    }
                }
            }
        }

        mesh->mNumFaces = model.lods[0].parts[i].indices.size() / 3;
        mesh->mFaces = new aiFace[mesh->mNumFaces];

        int lastFace = 0;
        for(int j = 0; j < model.lods[0].parts[i].indices.size(); j += 3) {
            aiFace& face = mesh->mFaces[lastFace++];

            face.mNumIndices = 3;
            face.mIndices = new unsigned int[face.mNumIndices];

            face.mIndices[0] = model.lods[0].parts[i].indices[j];
            face.mIndices[1] = model.lods[0].parts[i].indices[j + 1];
            face.mIndices[2] = model.lods[0].parts[i].indices[j + 2];
        }
    }

    scene.mNumMaterials = 1;
    scene.mMaterials = new aiMaterial*[1];
    scene.mMaterials[0] = new aiMaterial();

    exporter.Export(&scene, "fbx", fileName.toStdString());
}

void MainWindow::loadInitialGearInfo(GearInfo& info) {
    loadedGear.gearInfo = &info;

    raceCombo->clear();
    for(auto [race, race_name] : magic_enum::enum_entries<Race>()) {
        if(data.exists(build_equipment_path(loadedGear.gearInfo->modelInfo.primaryID, race, loadedGear.gearInfo->slot)))
            raceCombo->addItem(race_name.data());
    }

    currentLod = 0;
    currentRace = Race::HyurMidlanderMale;

    reloadGearModel();
}

void MainWindow::reloadGearModel() {
    auto mdl_data = data.extractFile(build_equipment_path(loadedGear.gearInfo->modelInfo.primaryID, currentRace, loadedGear.gearInfo->slot));
    if(mdl_data == std::nullopt)
        return;

    loadedGear.model = parseMDL(*mdl_data);

    lodCombo->clear();
    for(int i = 0; i < loadedGear.model.lods.size(); i++)
        lodCombo->addItem(QString::number(i));

    reloadGearAppearance();
}

void calculate_bone(Skeleton& skeleton, Bone& bone, const Bone* parent_bone) {
    const glm::mat4 parent_matrix = parent_bone == nullptr ? glm::mat4(1.0f) : parent_bone->localTransform;

    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    bone.localTransform = parent_matrix * local;
    bone.finalTransform = bone.localTransform * bone.inversePose;

    for(auto& b : skeleton.bones) {
        if(b.parent != nullptr && b.parent->name == bone.name)
            calculate_bone(skeleton, b, &bone);
    }
}

void MainWindow::reloadGearAppearance() {
    loadedGear.renderModel = renderer->addModel(loadedGear.model, currentLod);

    calculate_bone(skeleton, *skeleton.root_bone, nullptr);

    // we want to map the actual affected bones to bone ids
    std::map<int, int> boneMapping;
    for(int i = 0; i < loadedGear.model.affectedBoneNames.size(); i++) {
        for(int k = 0; k < skeleton.bones.size(); k++) {
            if(skeleton.bones[k].name == loadedGear.model.affectedBoneNames[i])
                boneMapping[i] = k;
        }
    }

    for(int i = 0; i < loadedGear.model.affectedBoneNames.size(); i++) {
        loadedGear.renderModel.boneData[i] = skeleton.bones[boneMapping[i]].finalTransform;
    }

#ifndef USE_STANDALONE_WINDOW
    vkWindow->models = {loadedGear.renderModel};
#else
    standaloneWindow->models = {loadedGear.renderModel};
#endif
}