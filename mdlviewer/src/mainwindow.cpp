#include "mainwindow.h"

#include <QHBoxLayout>
#include <QTableWidget>
#include <fmt/core.h>
#include <QListWidget>
#include <QVulkanWindow>
#include <QLineEdit>
#include <QResizeEvent>

#include "gamedata.h"
#include "exhparser.h"
#include "exdparser.h"
#include "mdlparser.h"

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

struct ModelInfo {

};

enum class Slot {
    Body,
    Legs
};

struct GearInfo {
    std::string name;
    Slot slot;
    ModelInfo modelInfo;
};

std::unordered_map<Slot, std::string_view> slotToName = {
        {Slot::Body, "top"},
        {Slot::Legs, "dwn"}
};

MainWindow::MainWindow(GameData& data) : data(data) {
    setWindowTitle("mdlviewer");
    setMinimumSize(QSize(640, 480));

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    std::vector<GearInfo> gears;

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

    auto listWidget = new QListWidget();
    for(auto gear : gears)
        listWidget->addItem(gear.name.c_str());

    listWidget->setMaximumWidth(200);

    layout->addWidget(listWidget);

    renderer = new Renderer();

    QVulkanInstance* inst = new QVulkanInstance();
    inst->setVkInstance(renderer->instance);
    inst->setFlags(QVulkanInstance::Flag::NoDebugOutputRedirect);
    inst->create();

    VulkanWindow* vkWindow = new VulkanWindow(renderer, inst);
    vkWindow->setVulkanInstance(inst);

    auto widget = QWidget::createWindowContainer(vkWindow);
    layout->addWidget(widget);

    connect(listWidget, &QListWidget::itemClicked, [this, &data, vkWindow, gears](QListWidgetItem* item) {
        for(auto gear : gears) {
            if(gear.name == item->text().toStdString()) {
                QString resolvedModelPath = QString("chara/equipment/e0000/model/c0201e0000_%1.mdl");
                resolvedModelPath = resolvedModelPath.arg(slotToName[gear.slot].data());

                data.extractFile(resolvedModelPath.toStdString(), "top.mdl");
                vkWindow->models.push_back(renderer->addModel(parseMDL("top.mdl")));
            }
        }
    });
}