// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vulkanwindow.h"

#include "imgui.h"

#include <QResizeEvent>
#include <QScreen>
#include <QVulkanInstance>

#include <glm/gtc/quaternion.hpp>

VulkanWindow::VulkanWindow(MDLPart *part, RenderManager *renderer, QVulkanInstance *instance)
    : m_renderer(renderer)
    , m_instance(instance)
    , m_part(part)
{
    setSurfaceType(VulkanSurface);
    setVulkanInstance(instance);
}

void VulkanWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed() && !m_initialized) {
        m_initialized = true;

        const auto surface = m_instance->surfaceForWindow(this);
        if (!m_renderer->initSwapchain(surface, width() * screen()->devicePixelRatio(), height() * screen()->devicePixelRatio())) {
            m_initialized = false;
        } else {
            Q_EMIT m_part->initializeRender();
            render();
        }
    }

    if (!isExposed() && m_initialized) {
        m_initialized = false;
        m_renderer->destroySwapchain(false);
    }
}

bool VulkanWindow::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Hide:
        // QWindow is reset when hiding a widget container without "SurfaceAboutToBeDestroyed" notification (Qt bug, tested on 6.5.1)
        m_renderer->destroySwapchain(false);
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
        const auto resizeEvent = static_cast<QResizeEvent *>(e);
        const auto surface = m_instance->surfaceForWindow(this);
        if (surface != nullptr && m_initialized) {
            m_renderer->resize(surface,
                               resizeEvent->size().width() * screen()->devicePixelRatio(),
                               resizeEvent->size().height() * screen()->devicePixelRatio());
        }
    } break;
    case QEvent::Hide: {
        m_renderer->destroySwapchain(false);
    } break;
    case QEvent::PlatformSurface: {
        const auto surfaceEvent = dynamic_cast<QPlatformSurfaceEvent *>(e);
        const auto surfaceEventType = surfaceEvent->surfaceEventType();
        if (surfaceEventType == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed && m_initialized) {
            m_renderer->destroySwapchain(false);
        }
    } break;
    case QEvent::MouseButtonPress: {
        const auto mouseEvent = dynamic_cast<QMouseEvent *>(e);

        m_part->setFocus(Qt::FocusReason::MouseFocusReason);

        if (m_part->isEnabled() && (mouseEvent->button() == Qt::MouseButton::LeftButton || mouseEvent->button() == Qt::MouseButton::RightButton)) {
            m_part->lastX = mouseEvent->position().x();
            m_part->lastY = mouseEvent->position().y();
            m_part->cameraMode = mouseEvent->button() == Qt::MouseButton::LeftButton ? MDLPart::CameraMode::Orbit : MDLPart::CameraMode::Move;

            setKeyboardGrabEnabled(true);
            setCursor(Qt::BlankCursor);
        }
    } break;
    case QEvent::MouseButtonRelease: {
        if (m_part->isEnabled()) {
            m_part->cameraMode = MDLPart::CameraMode::None;

            setKeyboardGrabEnabled(false);
            setCursor({});
        }
    } break;
    case QEvent::MouseMove: {
        const auto mouseEvent = dynamic_cast<QMouseEvent *>(e);
        if (m_part->isEnabled() && m_part->cameraMode != MDLPart::CameraMode::None) {
            const int deltaX = mouseEvent->position().x() - m_part->lastX;
            const int deltaY = mouseEvent->position().y() - m_part->lastY;

            if (m_part->cameraMode == MDLPart::CameraMode::Orbit) {
                m_part->yaw -= deltaX * 0.01f; // TODO: remove these magic numbers
                m_part->pitch += deltaY * 0.01f;
            } else {
                m_part->position += glm::vec3{0, 1, 0} * static_cast<float>(deltaY) * 0.01f;
                m_part->position.y = std::clamp(m_part->position.y, 0.0f, 10.0f);
                Q_EMIT m_part->cameraMoved();
            }

            m_part->lastX = mouseEvent->position().x();
            m_part->lastY = mouseEvent->position().y();
        }
    } break;
    case QEvent::Wheel: {
        const auto scrollEvent = dynamic_cast<QWheelEvent *>(e);

        if (m_part->isEnabled()) {
            m_part->cameraDistance -= scrollEvent->angleDelta().y() / 120.0f * 0.1f; // FIXME: why 120?
            m_part->cameraDistance = std::clamp(m_part->cameraDistance, m_part->minimumCameraDistance, 4.0f);
        }
    } break;
    case QEvent::KeyPress: {
        const auto keyEvent = dynamic_cast<QKeyEvent *>(e);

        if (m_part->isEnabled()) {
            switch (keyEvent->key()) {
            case Qt::Key_W:
                m_pressedKeys[0] = true;
                break;
            case Qt::Key_A:
                m_pressedKeys[1] = true;
                break;
            case Qt::Key_S:
                m_pressedKeys[2] = true;
                break;
            case Qt::Key_D:
                m_pressedKeys[3] = true;
                break;
            case Qt::Key_Shift:
                m_pressedKeys[4] = true;
                break;
            case Qt::Key_Q:
                m_pressedKeys[5] = true;
                break;
            case Qt::Key_E:
                m_pressedKeys[6] = true;
                break;
            default:
                break;
            }
        }
    } break;
    case QEvent::KeyRelease: {
        const auto keyEvent = dynamic_cast<QKeyEvent *>(e);

        if (m_part->isEnabled()) {
            switch (keyEvent->key()) {
            case Qt::Key_W:
                m_pressedKeys[0] = false;
                break;
            case Qt::Key_A:
                m_pressedKeys[1] = false;
                break;
            case Qt::Key_S:
                m_pressedKeys[2] = false;
                break;
            case Qt::Key_D:
                m_pressedKeys[3] = false;
                break;
            case Qt::Key_Shift:
                m_pressedKeys[4] = false;
                break;
            case Qt::Key_Q:
                m_pressedKeys[5] = false;
                break;
            case Qt::Key_E:
                m_pressedKeys[6] = false;
                break;
            default:
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

    const float deltaTime = m_timer.nsecsElapsed() / 1000000000.0f;
    m_timer.restart();

    ImGui::SetCurrentContext(m_renderer->ctx);

    auto &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width(), height());

    ImGui::NewFrame();

    if (m_part->requestUpdate)
        m_part->requestUpdate();

    ImGui::Render();

    if (freeMode) {
        float movX = 0.0f;
        float movY = 0.0f;

        if (m_pressedKeys[0]) {
            movY = -0.05f;
        }

        if (m_pressedKeys[1]) {
            movX = 0.05f;
        }

        if (m_pressedKeys[2]) {
            movY = 0.05f;
        }

        if (m_pressedKeys[3]) {
            movX = -0.05f;
        }

        const glm::vec3 forward =
            normalize(glm::angleAxis(m_part->yaw, glm::vec3(0, 1, 0)) * glm::angleAxis(m_part->pitch, glm::vec3(1, 0, 0)) * glm::vec3(0, 0, 1));
        const glm::vec3 right = normalize(glm::angleAxis(m_part->yaw, glm::vec3(0, 1, 0)) * glm::vec3(1, 0, 0));

        float speed = 200.0f;
        if (m_pressedKeys[4]) {
            speed = 1000.0f;
        }

        m_part->position += right * movX * speed * deltaTime;
        m_part->position += forward * movY * speed * deltaTime;

        // TODO: should up/down be considered from the camera's rotation?
        if (m_pressedKeys[5]) {
            m_part->position.y -= 0.05f * speed * deltaTime;
        }

        if (m_pressedKeys[6]) {
            m_part->position.y += 0.05f * speed * deltaTime;
        }

        Q_EMIT m_part->cameraMoved();

        m_renderer->camera.view = glm::mat4(1.0f);
        m_renderer->camera.view = glm::translate(m_renderer->camera.view, m_part->position);
        m_renderer->camera.view *= glm::mat4_cast(glm::angleAxis(m_part->yaw, glm::vec3(0, 1, 0)) * glm::angleAxis(m_part->pitch, glm::vec3(1, 0, 0)));
        m_renderer->camera.view = glm::inverse(m_renderer->camera.view);
        m_renderer->camera.position = m_part->position;
    } else {
        const glm::vec3 position(m_part->cameraDistance * sinf(m_part->yaw),
                                 m_part->cameraDistance * m_part->pitch,
                                 m_part->cameraDistance * cosf(m_part->yaw));

        m_renderer->camera.view = glm::lookAt(m_part->position + position, m_part->position, glm::vec3(0, -1, 0));
        m_renderer->camera.position = m_part->position + position;
    }

    m_renderer->render(models, vfx);
    m_instance->presentQueued(this);
    requestUpdate();
}
