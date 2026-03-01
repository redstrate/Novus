// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDebug>
#include <string_view>

#include <glm/glm.hpp>
#include <physis.hpp>
#include <spirv.hpp>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <vulkan/vulkan.h>

class Renderer;
struct DrawObjectInstance;
struct Camera;
class Texture;
struct Scene;
class Device;

/// Base class for all rendering implementations
class BaseRenderer
{
public:
    virtual ~BaseRenderer() = default;

    /// Perform any operations required on resize, such as recreating images.
    virtual void resize() = 0;

    /// Render a frame into @p commandBuffer. @p currentFrame is the same value as SwapChain::currentFrame for convenience.
    virtual void render(VkCommandBuffer commandBuffer, Camera &camera, Scene &scene, const std::vector<DrawObjectInstance> &models) = 0;

    /// The final composite texture that is drawn into with render()
    virtual Texture &getCompositeTexture() = 0;

    virtual Device &device() = 0;
};
