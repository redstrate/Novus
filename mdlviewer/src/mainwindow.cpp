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
        info.slot = Slot::Body;
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

    QComboBox* raceCombo = new QComboBox();
    for(auto [race, raceName] : raceNames) {
        raceCombo->addItem(raceName.data());
    }

    connect(raceCombo, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        currentRace = (Race)index;
        refreshModel();
    });

    viewportLayout->addWidget(raceCombo);

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
        vkWindow->models.push_back(renderer->addModel(parseMDL("top.mdl")));
#else
        standaloneWindow->models.push_back(renderer->addModel(parseMDL("top.mdl")));
#endif
    }
}