// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>
#include <vector>

#include <glslang/Public/ShaderLang.h>
#include <spirv.hpp>
#include <vulkan/vulkan.h>

struct physis_Shader;
class Device;

class ShaderManager
{
public:
    explicit ShaderManager(Device &device);

    VkShaderModule convertShaderModule(const physis_Shader &shader, spv::ExecutionModel executionModel);

private:
    std::vector<uint32_t> compileGLSL(const std::string_view sourceString, const EShLanguage sourceLanguage);

    Device &m_device;
};