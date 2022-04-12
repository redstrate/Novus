#include "mainwindow.h"

#include <QVBoxLayout>
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
                m_renderer->initSwapchain(surface, width(), height());

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
        m_renderer->render();
        m_instance->presentQueued(this);
        requestUpdate();
    }

private:
    bool m_initialized = false;
    Renderer* m_renderer;
    QVulkanInstance* m_instance;
};

MainWindow::MainWindow(GameData& data) : data(data) {
    setWindowTitle("mdlviewer");

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QVBoxLayout();
    dummyWidget->setLayout(layout);

    QLineEdit* pathEdit = new QLineEdit();
    layout->addWidget(pathEdit);

    renderer = new Renderer();

    QVulkanInstance* inst = new QVulkanInstance();
    inst->setVkInstance(renderer->instance);
    inst->setFlags(QVulkanInstance::Flag::NoDebugOutputRedirect);
    inst->create();

    VulkanWindow* vkWindow = new VulkanWindow(renderer, inst);
    vkWindow->setVulkanInstance(inst);

    auto widget = QWidget::createWindowContainer(vkWindow);
    //widget->resize(640, 480);
    layout->addWidget(widget);
    //vkWindow->show();

    //auto surface = inst.surfaceForWindow(vkWindow);
    //renderer->initSwapchain(surface);



}