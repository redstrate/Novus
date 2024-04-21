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
struct DrawObject;
struct Camera;
struct Texture;

/// Base class for all rendering implementations
class BaseRenderer
{
public:
    virtual ~BaseRenderer() = default;

    /// Perform any operations required on resize, such as recreating images.
    virtual void resize() = 0;

    /// Render a frame into @p commandBuffer. @p currentFrame is the same value as SwapChain::currentFrame for convenience.
    virtual void render(VkCommandBuffer commandBuffer, uint32_t currentFrame, Camera &camera, const std::vector<DrawObject> &models) = 0;

    /// Do whatever is needed in the backend to prepare it for drawing
    virtual void addDrawObject(const DrawObject &drawObject) = 0;

    /// The final composite texture that is drawn into with render()
    virtual Texture &getCompositeTexture() = 0;
};