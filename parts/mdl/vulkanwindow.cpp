// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vulkanwindow.h"

#include <QResizeEvent>
#include <QScreen>
#include <QVulkanInstance>

#include <glm/gtc/quaternion.hpp>

VulkanWindow::VulkanWindow(MDLPart *part, RenderManager *renderer, QVulkanInstance *instance)
    : m_renderer(renderer)
    , m_instance(instance)
    , part(part)
{
    setSurfaceType(VulkanSurface);
    setVulkanInstance(instance);
}

void VulkanWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed() && !m_initialized) {
        m_initialized = true;

        auto surface = m_instance->surfaceForWindow(this);
        if (!m_renderer->initSwapchain(surface, width() * screen()->devicePixelRatio(), height() * screen()->devicePixelRatio())) {
            m_initialized = false;
        } else {
            Q_EMIT part->initializeRender();
            render();
        }
    }

    if (!isExposed() && m_initialized) {
        m_initialized = false;
        m_renderer->destroySwapchain();
    }
}

bool VulkanWindow::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Hide:
        // QWindow is reset when hiding a widget container without "SurfaceAboutToBeDestroyed" notification (Qt bug, tested on 6.5.1)
        m_renderer->destroySwapchain();
        m_initialized = false;
        break;
    default:
        break;
    }
    // dispatchEvent(event, watched);
    return QWindow::eventFilter(watched, event);
}

bool VulkanWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::UpdateRequest:
        render();
        break;
    case QEvent::Resize: {
        auto resizeEvent = (QResizeEvent *)e;
        auto surface = m_instance->surfaceForWindow(this);
        if (surface != nullptr) {
            m_renderer->resize(surface,
                               resizeEvent->size().width() * screen()->devicePixelRatio(),
                               resizeEvent->size().height() * screen()->devicePixelRatio());
        }
    } break;
    case QEvent::Hide: {
        m_renderer->destroySwapchain();
    } break;
    case QEvent::PlatformSurface: {
        auto surfaceEvent = dynamic_cast<QPlatformSurfaceEvent *>(e);
        auto surfaceEventType = surfaceEvent->surfaceEventType();
        if (surfaceEventType == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed && m_initialized) {
            m_renderer->destroySwapchain();
        }
    } break;
    case QEvent::MouseButtonPress: {
        auto mouseEvent = dynamic_cast<QMouseEvent *>(e);

        part->setFocus(Qt::FocusReason::MouseFocusReason);

        if (part->isEnabled() && (mouseEvent->button() == Qt::MouseButton::LeftButton || mouseEvent->button() == Qt::MouseButton::RightButton)) {
            part->lastX = mouseEvent->position().x();
            part->lastY = mouseEvent->position().y();
            part->cameraMode = mouseEvent->button() == Qt::MouseButton::LeftButton ? MDLPart::CameraMode::Orbit : MDLPart::CameraMode::Move;

            setKeyboardGrabEnabled(true);
            setCursor(Qt::BlankCursor);
        }
    } break;
    case QEvent::MouseButtonRelease: {
        if (part->isEnabled()) {
            part->cameraMode = MDLPart::CameraMode::None;

            setKeyboardGrabEnabled(false);
            setCursor({});
        }
    } break;
    case QEvent::MouseMove: {
        auto mouseEvent = dynamic_cast<QMouseEvent *>(e);
        if (part->isEnabled() && part->cameraMode != MDLPart::CameraMode::None) {
            const int deltaX = mouseEvent->position().x() - part->lastX;
            const int deltaY = mouseEvent->position().y() - part->lastY;

            if (part->cameraMode == MDLPart::CameraMode::Orbit) {
                part->yaw += deltaX * 0.01f; // TODO: remove these magic numbers
                part->pitch += deltaY * 0.01f;
            } else {
                const glm::vec3 position(part->cameraDistance * std::sin(part->yaw),
                                         part->cameraDistance * part->pitch,
                                         part->cameraDistance * std::cos(part->yaw));

                // const glm::quat rot = glm::quatLookAt((part->position + position) - part->position, {0, 1, 0});

                part->position += glm::vec3{0, 1, 0} * (float)deltaY * 0.01f;
                part->position.y = std::clamp(part->position.y, 0.0f, 10.0f);
            }

            part->lastX = mouseEvent->position().x();
            part->lastY = mouseEvent->position().y();
        }
    } break;
    case QEvent::Wheel: {
        auto scrollEvent = dynamic_cast<QWheelEvent *>(e);

        if (part->isEnabled()) {
            part->cameraDistance -= (scrollEvent->angleDelta().y() / 120.0f) * 0.1f; // FIXME: why 120?
            part->cameraDistance = std::clamp(part->cameraDistance, part->minimumCameraDistance, 4.0f);
        }
    } break;
    case QEvent::KeyPress: {
        auto keyEvent = dynamic_cast<QKeyEvent *>(e);

        if (part->isEnabled()) {
            switch (keyEvent->key()) {
            case Qt::Key_W:
                pressed_keys[0] = true;
                break;
            case Qt::Key_A:
                pressed_keys[1] = true;
                break;
            case Qt::Key_S:
                pressed_keys[2] = true;
                break;
            case Qt::Key_D:
                pressed_keys[3] = true;
                break;
            }
        }
    } break;
    case QEvent::KeyRelease: {
        auto keyEvent = dynamic_cast<QKeyEvent *>(e);

        if (part->isEnabled()) {
            switch (keyEvent->key()) {
            case Qt::Key_W:
                pressed_keys[0] = false;
                break;
            case Qt::Key_A:
                pressed_keys[1] = false;
                break;
            case Qt::Key_S:
                pressed_keys[2] = false;
                break;
            case Qt::Key_D:
                pressed_keys[3] = false;
                break;
            }
        }
    } break;
    default:
        break;
    }

    return QWindow::event(e);
}

void VulkanWindow::render()
{
    if (!m_initialized) {
        return;
    }

    const float deltaTime = timer.nsecsElapsed() / 1000000000.0f;
    timer.restart();

    ImGui::SetCurrentContext(m_renderer->ctx);

    auto &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width() * screen()->devicePixelRatio(), height() * screen()->devicePixelRatio());

    ImGui::NewFrame();

    if (part->requestUpdate)
        part->requestUpdate();

    ImGui::Render();

    if (freeMode) {
        float movX = 0.0f;
        float movY = 0.0f;

        if (pressed_keys[0]) {
            movY = -0.05f;
        }

        if (pressed_keys[1]) {
            movX = -0.05f;
        }

        if (pressed_keys[2]) {
            movY = 0.05f;
        }

        if (pressed_keys[3]) {
            movX = 0.05f;
        }

        glm::vec3 forward, right;
        forward = normalize(glm::angleAxis(part->yaw, glm::vec3(0, 1, 0)) * glm::angleAxis(part->pitch, glm::vec3(1, 0, 0)) * glm::vec3(0, 0, 1));
        right = normalize(glm::angleAxis(part->yaw, glm::vec3(0, 1, 0)) * glm::vec3(1, 0, 0));

        part->position += right * movX * 200.0f * deltaTime;
        part->position += forward * movY * 200.0f * deltaTime;

        m_renderer->camera.view = glm::mat4(1.0f);
        m_renderer->camera.view = glm::translate(m_renderer->camera.view, part->position);
        m_renderer->camera.view *= glm::mat4_cast(glm::angleAxis(part->yaw, glm::vec3(0, 1, 0)) * glm::angleAxis(part->pitch, glm::vec3(1, 0, 0)));
        m_renderer->camera.view = glm::inverse(m_renderer->camera.view);
        m_renderer->camera.position = part->position;
    } else {
        glm::vec3 position(part->cameraDistance * sin(part->yaw), part->cameraDistance * part->pitch, part->cameraDistance * cos(part->yaw));

        m_renderer->camera.view = glm::lookAt(part->position + position, part->position, glm::vec3(0, -1, 0));
        m_renderer->camera.position = part->position + position;
    }

    m_renderer->render(models);
    m_instance->presentQueued(this);
    requestUpdate();
}
