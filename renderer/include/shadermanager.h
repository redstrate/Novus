// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>
#include <vector>

#include <physis.hpp>
#include <spirv.hpp>
#include <spirv_glsl.hpp>
#include <vulkan/vulkan.h>

class Device;

class ShaderManager
{
public:
    explicit ShaderManager(Device &device);

    spirv_cross::CompilerGLSL getShaderModuleTest(const physis_Shader &shader);
    std::string getShaderModuleResources(const physis_Shader &shader, int i);
    VkShaderModule convertShaderModule(const physis_Shader &shader, spv::ExecutionModel executionModel);

private:
    std::vector<uint32_t> compileGLSL(std::string_view sourceString, ShaderStage stage);

    Device &m_device;
};
