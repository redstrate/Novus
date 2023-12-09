// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vulkanwindow.h"

#include <QResizeEvent>
#include <QScreen>
#include <QVulkanInstance>

#include <glm/gtc/quaternion.hpp>

VulkanWindow::VulkanWindow(MDLPart *part, Renderer *renderer, QVulkanInstance *instance)
    : m_renderer(renderer)
    , m_instance(instance)
    , part(part)
{
    setSurfaceType(VulkanSurface);
    setVulkanInstance(instance);
}

void VulkanWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed()) {
        if (!m_initialized) {
            m_initialized = true;

            auto surface = m_instance->surfaceForWindow(this);
            if (!m_renderer->initSwapchain(surface, width() * screen()->devicePixelRatio(), height() * screen()->devicePixelRatio()))
                m_initialized = false;
            else
                render();
        }
    }
}

bool VulkanWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::UpdateRequest:
        render();
        break;
    case QEvent::Resize: {
        QResizeEvent *resizeEvent = (QResizeEvent *)e;
        auto surface = m_instance->surfaceForWindow(this);
        m_renderer->resize(surface, resizeEvent->size().width() * screen()->devicePixelRatio(), resizeEvent->size().height() * screen()->devicePixelRatio());
    } break;
    case QEvent::MouseButtonPress: {
        auto mouseEvent = dynamic_cast<QMouseEvent *>(e);

        if (mouseEvent->button() == Qt::MouseButton::LeftButton || mouseEvent->button() == Qt::MouseButton::RightButton) {
            part->lastX = mouseEvent->position().x();
            part->lastY = mouseEvent->position().y();
            part->cameraMode = mouseEvent->button() == Qt::MouseButton::LeftButton ? MDLPart::CameraMode::Orbit : MDLPart::CameraMode::Move;

            setKeyboardGrabEnabled(true);
            setCursor(Qt::BlankCursor);
        }
    } break;
    case QEvent::MouseButtonRelease: {
        part->cameraMode = MDLPart::CameraMode::None;

        setKeyboardGrabEnabled(false);
        setCursor({});
    } break;
    case QEvent::MouseMove: {
        auto mouseEvent = dynamic_cast<QMouseEvent *>(e);
        if (part->cameraMode != MDLPart::CameraMode::None) {
            const int deltaX = mouseEvent->position().x() - part->lastX;
            const int deltaY = mouseEvent->position().y() - part->lastY;

            if (part->cameraMode == MDLPart::CameraMode::Orbit) {
                part->yaw += deltaX * 0.01f; // TODO: remove these magic numbers
                part->pitch += deltaY * 0.01f;
            } else {
                const glm::vec3 position(part->cameraDistance * std::sin(part->yaw),
                                         part->cameraDistance * part->pitch,
                                         part->cameraDistance * std::cos(part->yaw));

                const glm::quat rot = glm::quatLookAt((part->position + position) - part->position, {0, 1, 0});

                part->position += glm::vec3{0, 1, 0} * (float)deltaY * 0.01f;
                part->position.y = std::clamp(part->position.y, 0.0f, 10.0f);
            }

            part->lastX = mouseEvent->position().x();
            part->lastY = mouseEvent->position().y();
        }
    } break;
    case QEvent::Wheel: {
        auto scrollEvent = dynamic_cast<QWheelEvent *>(e);

        part->cameraDistance -= (scrollEvent->angleDelta().y() / 120.0f) * 0.1f; // FIXME: why 120?
        part->cameraDistance = std::clamp(part->cameraDistance, 1.0f, 4.0f);
    } break;
    }

    return QWindow::event(e);
}

void VulkanWindow::render()
{
    ImGui::SetCurrentContext(m_renderer->ctx);

    auto &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width(), height());

    ImGui::NewFrame();

    if (part->requestUpdate)
        part->requestUpdate();

    ImGui::Render();

    glm::vec3 position(part->cameraDistance * sin(part->yaw), part->cameraDistance * part->pitch, part->cameraDistance * cos(part->yaw));

    m_renderer->view = glm::lookAt(part->position + position, part->position, glm::vec3(0, -1, 0));

    m_renderer->render(models);
    m_instance->presentQueued(this);
    requestUpdate();
}
