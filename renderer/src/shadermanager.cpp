// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shadermanager.h"

#include <dxbc_module.h>
#include <dxbc_reader.h>
#include <physis.hpp>
#include <spirv_glsl.hpp>

#ifdef HAVE_GLSLANG
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/Logger.h>
#endif

#include "device.h"

#include <ranges>

ShaderManager::ShaderManager(Device &device)
    : m_device(device)
{
}

spirv_cross::CompilerGLSL ShaderManager::getShaderModuleTest(const physis_Shader &shader)
{
    dxvk::DxbcReader reader(reinterpret_cast<const char *>(shader.bytecode), shader.len);

    dxvk::DxbcModule module(reader);

    dxvk::DxbcModuleInfo info;
    auto result = module.compile(info, "test");

    return {result.code.data(), result.code.dwords()};
}

std::string ShaderManager::getShaderModuleResources(const physis_Shader &shader, int i)
{
    dxvk::DxbcReader reader(reinterpret_cast<const char *>(shader.bytecode), shader.len);

    dxvk::DxbcModule module(reader);

    dxvk::DxbcModuleInfo info;
    auto result = module.compile(info, "test");

    return module.isgn()->findByRegister(i)->semanticName;
}

VkShaderModule ShaderManager::convertShaderModule(const physis_Shader &shader, spv::ExecutionModel executionModel)
{
    dxvk::DxbcReader reader(reinterpret_cast<const char *>(shader.bytecode), shader.len);

    dxvk::DxbcModule module(reader);

    dxvk::DxbcModuleInfo info;
    auto result = module.compile(info, "test");

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

#ifdef HAVE_GLSLANG
    // TODO: for debug only
    spirv_cross::CompilerGLSL glsl(result.code.data(), result.code.dwords());

    auto resources = glsl.get_shader_resources();

    uint32_t i = 0;
    for (const auto &texture : resources.stage_inputs) {
        unsigned binding = glsl.get_decoration(texture.id, spv::DecorationLocation);
        glsl.set_name(texture.id, module.isgn()->findByRegister(binding)->semanticName);
        i++;
    }

    // Here you can also set up decorations if you want (binding = #N).
    i = 0;
    for (const auto &texture : resources.separate_images) {
        if (i < shader.num_resource_parameters) {
            glsl.set_name(texture.id, shader.resource_parameters[i].name);
        } else {
            glsl.set_name(texture.id, texture.name);
        }
        i++;
    }

    i = 0;
    for (const auto &buffer : resources.uniform_buffers) {
        glsl.set_name(buffer.id, shader.scalar_parameters[i].name);
        i++;
    }

    spirv_cross::CompilerGLSL::Options options;
    options.vulkan_semantics = true;
    options.enable_420pack_extension = false;
    glsl.set_common_options(options);
    glsl.set_entry_point("main", executionModel);

    auto newModule = compileGLSL(glsl.compile(), executionModel == spv::ExecutionModelVertex ? ShaderStage::Vertex : ShaderStage::Pixel);
    createInfo.codeSize = newModule.size() * sizeof(uint32_t);
    createInfo.pCode = reinterpret_cast<const uint32_t *>(newModule.data());
#else
    createInfo.codeSize = result.code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(result.code.data());
#endif

    VkShaderModule shaderModule;
    vkCreateShaderModule(m_device.device, &createInfo, nullptr, &shaderModule);

    return shaderModule;
}

std::vector<uint32_t> ShaderManager::compileGLSL(const std::string_view sourceString, const ShaderStage stage)
{
#ifdef HAVE_GLSLANG
    static bool ProcessInitialized = false;

    if (!ProcessInitialized) {
        glslang::InitializeProcess();
        ProcessInitialized = true;
    }

    const char *InputCString = sourceString.data();

    EShLanguage sourceLanguage = EShLanguage::EShLangVertex;
    switch (stage) {
    case ShaderStage::Vertex:
        sourceLanguage = EShLanguage::EShLangVertex;
        break;
    case ShaderStage::Pixel:
        sourceLanguage = EShLanguage::EShLangFragment;
        break;
    }

    glslang::TShader shader(sourceLanguage);
    shader.setStrings(&InputCString, 1);

    int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100

    shader.setEnvInput(glslang::EShSourceGlsl, sourceLanguage, glslang::EShClientVulkan, ClientInputSemanticsVersion);

    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);

    if (!shader.parse(GetDefaultResources(), 100, false, EShMsgDebugInfo)) {
        return {};
    }

    glslang::TProgram Program;
    Program.addShader(&shader);

    if (!Program.link(EShMsgDebugInfo)) {
        return {};
    }

    std::vector<unsigned int> SpirV;
    spv::SpvBuildLogger logger;

    glslang::SpvOptions spvOptions;
    spvOptions.generateDebugInfo = true;
    spvOptions.stripDebugInfo = false;
    spvOptions.disableOptimizer = true;
    spvOptions.emitNonSemanticShaderDebugSource = true;
    spvOptions.emitNonSemanticShaderDebugInfo = true;

    glslang::GlslangToSpv(*Program.getIntermediate(sourceLanguage), SpirV, &logger, &spvOptions);

    return SpirV;
#else
    return {};
#endif
}
