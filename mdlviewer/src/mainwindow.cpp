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

#include "gamedata.h"
#include "exhparser.h"
#include "exdparser.h"
#include "mdlparser.h"

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
#endif

MainWindow::MainWindow(GameData& data) : data(data) {
    setWindowTitle("mdlviewer");
    setMinimumSize(QSize(640, 480));

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
    data.extractFile("exd/" + path, path);
    auto exd = readEXD(exh, path, exh.pages[1]);
    for(auto row : exd.rows) {
        auto primaryModel = row.data[47].uint64Data;
        auto secondaryModel = row.data[48].uint64Data;

        int16_t parts[4];
        memcpy(parts, &primaryModel, sizeof(int16_t) * 4);

        GearInfo info = {};
        info.name = row.data[9].data;
        info.slot = (Slot)row.data[17].uint64Data;
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

    QComboBox* raceCombo = new QComboBox();
    for(auto [race, raceName] : raceNames) {
        raceCombo->addItem(raceName.data());
    }

    connect(raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        currentRace = (Race)index;
        refreshModel();
    });

    controlLayout->addWidget(raceCombo);

    auto lodCombo = new QComboBox();
    lodCombo->addItem("0");
    lodCombo->addItem("1");
    lodCombo->addItem("2");

    connect(lodCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        currentLod = index;
        refreshModel();
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
        exportModel(model, fileName);
    });

    controlLayout->addWidget(exportButton);

    connect(listWidget, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        for(auto& gear : gears) {
            if(gear.name == item->text().toStdString())
                loadedGears = {&gear};
        }

        refreshModel();
    });
}

void MainWindow::refreshModel() {
#ifdef USE_STANDALONE_WINDOW
    standaloneWindow->models.clear();
#else
    vkWindow->models.clear();
#endif

    for(auto gear : loadedGears) {
        QString modelID = QString("%1").arg(gear->modelInfo.primaryID, 4, 10, QLatin1Char('0'));

        QString resolvedModelPath = QString("chara/equipment/e%1/model/c%2e%3_%4.mdl");
        resolvedModelPath = resolvedModelPath.arg(modelID, raceIDs[currentRace].data(), modelID, slotToName[gear->slot].data());

        data.extractFile(resolvedModelPath.toStdString(), "top.mdl");

#ifndef USE_STANDALONE_WINDOW
        vkWindow->models.push_back(renderer->addModel(parseMDL("top.mdl"), currentLod));
#else
        standaloneWindow->models.push_back(renderer->addModel(parseMDL("top.mdl"), currentLod));
#endif
    }
}

void MainWindow::exportModel(Model& model, QString fileName) {
    Assimp::Exporter exporter;

    aiScene scene;
    scene.mRootNode = new aiNode();

    scene.mRootNode->mNumChildren = model.lods[0].parts.size();
    scene.mRootNode->mChildren = new aiNode*[scene.mRootNode->mNumChildren];

    scene.mNumMeshes = model.lods[0].parts.size();
    scene.mMeshes = new aiMesh*[scene.mNumMeshes];

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
        for(int j = 0; j < mesh->mNumVertices; j++) {
            auto vertex = model.lods[0].parts[i].vertices[j];
            mesh->mVertices[j] = aiVector3D(vertex.position[0], vertex.position[1], vertex.position[2]);
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
