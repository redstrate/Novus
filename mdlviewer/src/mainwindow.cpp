#include "mainwindow.h"

#include <QHBoxLayout>
#include <QTableWidget>
#include <fmt/core.h>
#include <QListWidget>
#include <QVulkanWindow>

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
                render();
            }
        }
    }

    bool event(QEvent *e) {
        if (e->type() == QEvent::UpdateRequest)
            render();

        return QWindow::event(e);
    }

    void render() {
        m_renderer->render();
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

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    renderer = new Renderer();

    QVulkanInstance inst;
    inst.setVkInstance(renderer->instance);
    inst.setFlags(QVulkanInstance::Flag::NoDebugOutputRedirect);
    inst.create();

    VulkanWindow* vkWindow = new VulkanWindow(renderer, &inst);
    vkWindow->show();
    vkWindow->setVulkanInstance(&inst);

    auto surface = inst.surfaceForWindow(vkWindow);
    renderer->initSwapchain(surface);

    auto widget = QWidget::createWindowContainer(vkWindow);
    layout->addWidget(widget);

}