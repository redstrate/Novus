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
#include <physis.hpp>
#include <glm/glm.hpp>

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

void MainWindow::calculate_bone_inverse_pose(physis_Skeleton& skeleton, physis_Bone& bone, physis_Bone* parent_bone) {
    const glm::mat4 parentMatrix = parent_bone == nullptr ? glm::mat4(1.0f) : extraBone[parent_bone->index].inversePose;

    glm::mat4 local(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    extraBone[bone.index].inversePose = parentMatrix * local;

    for(int i = 0; i < skeleton.num_bones; i++) {
        if(skeleton.bones[i].parent_bone != nullptr && strcmp(skeleton.bones[i].parent_bone->name, bone.name) == 0) {
            calculate_bone_inverse_pose(skeleton, skeleton.bones[i], &bone);
        }
    }
}

void addItem(physis_Skeleton& skeleton, physis_Bone& bone, QTreeWidget* widget, QTreeWidgetItem* parent_item = nullptr) {
    auto item = new QTreeWidgetItem();
    item->setText(0, bone.name);

    if(parent_item == nullptr) {
        widget->addTopLevelItem(item);
    } else {
        parent_item->addChild(item);
    }

    for(int i = 0; i < skeleton.num_bones; i++) {
        if(skeleton.bones[i].parent_bone != nullptr && strcmp(skeleton.bones[i].parent_bone->name, bone.name) == 0)
            addItem(skeleton, skeleton.bones[i], widget, item);
    }
}

MainWindow::MainWindow(GameData* in_data) : data(*in_data) {
    setWindowTitle("mdlviewer");
    setMinimumSize(QSize(640, 480));

    auto fileMenu = menuBar()->addMenu("File");

    auto openMDLFile = fileMenu->addAction("Open MDL...");
    connect(openMDLFile, &QAction::triggered, [=] {
        auto fileName = QFileDialog::getOpenFileName(nullptr,
                                                     "Open MDL File",
                                                     "~",
                                                     "FFXIV Model File (*.mdl)");

        auto buffer = physis_read_file(fileName.toStdString().c_str());

        loadedGear.model = physis_mdl_parse(buffer.size, buffer.data);

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

    auto exh = physis_gamedata_read_excel_sheet_header(&data, "Item");
    auto exd = physis_gamedata_read_excel_sheet(&data, "Item", exh, Language::English, 1);

    for(int i = 0; i < exd.row_count; i++) {
        const auto row = exd.row_data[i];
        auto primaryModel = row.column_data[47].u_int64._0;
        auto secondaryModel = row.column_data[48].u_int64._0;

        int16_t parts[4];
        memcpy(parts, &primaryModel, sizeof(int16_t) * 4);

        GearInfo info = {};
        info.name = row.column_data[9].string._0;
        info.slot = physis_slot_from_id(row.column_data[17].u_int8._0);
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
        if(index != -1) {
            currentRace = (Race) index;
            reloadGearModel();
        }
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

        physis_MDL model;
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

    skeleton = physis_skeleton_from_skel(physis_read_file("c0101b0001.skel"));
    extraBone.resize(skeleton.num_bones);
    calculate_bone_inverse_pose(skeleton, *skeleton.root_bone, nullptr);

    auto boneListWidget = new QTreeWidget();
    for(auto& bone : extraBone) {
        bone.inversePose = glm::inverse(bone.inversePose);
    }

    addItem(skeleton, *skeleton.root_bone, boneListWidget);

    boneListWidget->setMaximumWidth(200);

    connect(boneListWidget, &QTreeWidget::itemClicked, [this](QTreeWidgetItem* item, int column) {
        for(int i = 0; i < skeleton.num_bones; i++) {
            if(strcmp(skeleton.bones[i].name, item->text(column).toStdString().c_str()) == 0) {
                currentScale = glm::make_vec3(skeleton.bones[i].scale);
                currentEditedBone = &skeleton.bones[i];
            }
        }
    });

    layout->addWidget(boneListWidget);

    Vector3Edit* scaleEdit = new Vector3Edit(currentScale);
    connect(scaleEdit, &Vector3Edit::onValueChanged, [this] {
        memcpy(currentEditedBone->scale, glm::value_ptr(currentScale), sizeof(float) * 3);
        reloadGearAppearance();
    });
    layout->addWidget(scaleEdit);
}

void MainWindow::exportModel(physis_MDL& model, physis_Skeleton& skeleton, QString fileName) {
    Assimp::Exporter exporter;

    aiScene scene;
    scene.mRootNode = new aiNode();

    scene.mRootNode->mNumChildren = model.lods[0].num_parts + 1; // plus one for the skeleton
    scene.mRootNode->mChildren = new aiNode*[scene.mRootNode->mNumChildren];

    scene.mNumMeshes = model.lods[0].num_parts;
    scene.mMeshes = new aiMesh*[scene.mNumMeshes];

    auto skeleton_node = new aiNode();
    skeleton_node->mName = "Skeleton";
    skeleton_node->mNumChildren = 1;
    skeleton_node->mChildren = new aiNode*[skeleton_node->mNumChildren];

    scene.mRootNode->mChildren[scene.mRootNode->mNumChildren - 1] = skeleton_node;

    std::vector<aiNode*> skeletonNodes;

    for(int i = 0; i < model.num_affected_bones; i++) {
        auto& node = skeletonNodes.emplace_back();
        node = new aiNode();
        node->mName = model.affected_bone_names[i];

        int real_bone_id = 0;
        for(int k = 0; k < skeleton.num_bones; k++) {
            if(strcmp(skeleton.bones[k].name, model.affected_bone_names[i]) == 0) {
                real_bone_id  = k;
            }
        }

        node->mChildren = new aiNode*[model.num_affected_bones];

        auto& real_bone = skeleton.bones[real_bone_id];
        memcpy(&node->mTransformation, glm::value_ptr(extraBone[real_bone.index].finalTransform), sizeof(aiMatrix4x4));
    }

    // setup parenting
    for(int i = 0; i < model.num_affected_bones; i++) {
        int real_bone_id = 0;
        for(int k = 0; k < skeleton.num_bones; k++) {
            if(strcmp(skeleton.bones[k].name, model.affected_bone_names[i]) == 0) {
                real_bone_id  = k;
            }
        }

        auto& real_bone = skeleton.bones[real_bone_id];
        if(real_bone.parent_bone != nullptr) {
            for(int k = 0; k < model.num_affected_bones; k++) {
                if(strcmp(model.affected_bone_names[k], real_bone.parent_bone->name) == 0) {
                    skeletonNodes[i]->mParent = skeletonNodes[k];
                    skeletonNodes[k]->mChildren[skeletonNodes[k]->mNumChildren++] = skeletonNodes[i];
                }
            }
        }
    }

    skeleton_node->mChildren[0] = new aiNode();
    skeleton_node->mChildren[0]->mName = "root";
    skeleton_node->mChildren[0]->mChildren = new aiNode*[model.num_affected_bones];

    for(int i = 0; i < skeletonNodes.size(); i++) {
      if(skeletonNodes[i]->mParent == nullptr) {
        skeleton_node->mChildren[0]->mChildren[skeleton_node->mChildren[0]->mNumChildren++] = skeletonNodes[i];
      }
    }

    for(int i = 0; i < model.lods[0].num_parts; i++) {
        scene.mMeshes[i] = new aiMesh();
        scene.mMeshes[i]->mMaterialIndex = 0;

        auto& node = scene.mRootNode->mChildren[i];
        node = new aiNode();
        node->mNumMeshes = 1;
        node->mMeshes = new unsigned int [scene.mRootNode->mNumMeshes];
        node->mMeshes[0] = i;

        auto mesh = scene.mMeshes[i];
        mesh->mNumVertices = model.lods[0].parts[i].num_vertices;
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

        mesh->mNumBones = model.num_affected_bones;
        mesh->mBones = new aiBone*[mesh->mNumBones];
        for(int j = 0; j < mesh->mNumBones; j++) {
            int real_bone_id = j;
            /*for(int k = 0; k < skeleton.bones.size(); k++) {
                if(skeleton.bones[k].name == model.affectedBoneNames[j]) {
                    real_bone_id  = k;
                }
            }*/

            mesh->mBones[j] = new aiBone();
            mesh->mBones[j]->mName = model.affected_bone_names[j];
            mesh->mBones[j]->mNumWeights = mesh->mNumVertices * 4;
            mesh->mBones[j]->mWeights = new aiVertexWeight[mesh->mBones[j]->mNumWeights];
            mesh->mBones[j]->mNode = skeleton_node->mChildren[j];

            for(int k = 0; k < mesh->mNumVertices; k++) {
                for(int z = 0; z < 4; z++) {
                    if (model.lods[0].parts[i].vertices[k].bone_id[z] == real_bone_id) {
                        auto &weight = mesh->mBones[j]->mWeights[k * 4 + z];
                        weight.mVertexId = k;
                        weight.mWeight =  model.lods[0].parts[i].vertices[k].bone_weight[z];
                    }
                }
            }
        }

        mesh->mNumFaces = model.lods[0].parts[i].num_indices / 3;
        mesh->mFaces = new aiFace[mesh->mNumFaces];

        int lastFace = 0;
        for(int j = 0; j < model.lods[0].parts[i].num_indices; j += 3) {
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
        auto equip_path = physis_build_equipment_path(loadedGear.gearInfo->modelInfo.primaryID, race, currentSubrace, currentGender, loadedGear.gearInfo->slot);

        if(physis_gamedata_exists(&data, equip_path))
          raceCombo->addItem(race_name.data());
    }

    currentLod = 0;
    currentRace = Race::Hyur;

    reloadGearModel();
}

void MainWindow::reloadGearModel() {
    auto mdl_data = physis_gamedata_extract_file(&data, physis_build_equipment_path(loadedGear.gearInfo->modelInfo.primaryID, currentRace, currentSubrace, currentGender, loadedGear.gearInfo->slot));

    loadedGear.model = physis_mdl_parse(mdl_data.size, mdl_data.data);

    std::string mtrl_path = loadedGear.gearInfo->getMtrlPath(101);
    qDebug() << "MTRL path: " << mtrl_path.c_str();

    if(physis_gamedata_exists(&data, mtrl_path.c_str())) {
        qDebug() << "loading mtrl...";
        loadedGear.material = physis_material_parse(physis_gamedata_extract_file(&data, mtrl_path.c_str()));
    }

    lodCombo->clear();
    for(int i = 0; i < loadedGear.model.num_lod; i++)
        lodCombo->addItem(QString::number(i));

    reloadGearAppearance();
}

void MainWindow::calculate_bone(physis_Skeleton& skeleton, physis_Bone& bone, const physis_Bone* parent_bone) {
    const glm::mat4 parent_matrix = parent_bone == nullptr ? glm::mat4(1.0f) : extraBone[parent_bone->index].localTransform;

    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, glm::vec3(bone.position[0], bone.position[1], bone.position[2]));
    local *= glm::mat4_cast(glm::quat(bone.rotation[3], bone.rotation[0], bone.rotation[1], bone.rotation[2]));
    local = glm::scale(local, glm::vec3(bone.scale[0], bone.scale[1], bone.scale[2]));

    extraBone[bone.index].localTransform = parent_matrix * local;
    extraBone[bone.index].finalTransform = extraBone[bone.index].localTransform * extraBone[bone.index].inversePose;

    for(int i = 0; i < skeleton.num_bones; i++) {
        if(skeleton.bones[i].parent_bone != nullptr && strcmp(skeleton.bones[i].parent_bone->name, bone.name) == 0) {
            calculate_bone(skeleton, skeleton.bones[i], &bone);
        }
    }
}

void MainWindow::reloadGearAppearance() {
    loadedGear.renderModel = renderer->addModel(loadedGear.model, currentLod);

    if(loadedGear.material.num_textures > 0) {
        auto texture = physis_texture_parse(physis_gamedata_extract_file(&data, loadedGear.material.textures[0]));

        loadedGear.renderTexture = renderer->addTexture(texture.width, texture.height, texture.rgba, texture.rgba_size);

        loadedGear.renderModel.texture = &loadedGear.renderTexture;
    }

    calculate_bone(skeleton, *skeleton.root_bone, nullptr);

    // we want to map the actual affected bones to bone ids
    std::map<int, int> boneMapping;
    for(int i = 0; i < loadedGear.model.num_affected_bones; i++) {
        for(int k = 0; k < skeleton.num_bones; k++) {
            if(strcmp(skeleton.bones[k].name, loadedGear.model.affected_bone_names[i]) == 0)
                boneMapping[i] = k;
        }
    }

    for(int i = 0; i < loadedGear.model.num_affected_bones; i++) {
        loadedGear.renderModel.boneData[i] = extraBone[boneMapping[i]].finalTransform;
    }

#ifndef USE_STANDALONE_WINDOW
    vkWindow->models = {loadedGear.renderModel};
#else
    standaloneWindow->models = {loadedGear.renderModel};
#endif
}