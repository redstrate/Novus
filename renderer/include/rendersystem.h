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
struct RenderModel;

class RenderSystem
{
public:
    RenderSystem(Renderer &renderer, GameData *data);

    void testInit(::RenderModel *m);

    void render(uint32_t imageIndex, VkCommandBuffer commandBuffer);

    void setSize(uint32_t width, uint32_t height);

private:
    void beginPass(uint32_t imageIndex, VkCommandBuffer commandBuffer, std::string_view passName);
    void endPass(VkCommandBuffer commandBuffer, std::string_view passName);
    void bindPipeline(VkCommandBuffer commandBuffer, std::string_view passName, physis_Shader &vertexShader, physis_Shader &pixelShader);
    VkShaderModule convertShaderModule(const physis_Shader &shader, spv::ExecutionModel executionModel);
    spirv_cross::CompilerGLSL getShaderModuleResources(const physis_Shader &shader);

    physis_SHPK directionalLightningShpk;

    struct RenderModel {
        physis_SHPK shpk;

        ::RenderModel *internal_model = nullptr;
    };
    std::vector<RenderModel> m_renderModels;

    struct RequestedBinding {
        VkDescriptorType type;
        VkShaderStageFlags stageFlags;
        bool used = false;
    };

    struct RequestedSet {
        bool used = true;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        std::vector<RequestedBinding> bindings;
    };

    struct CachedPipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> setLayouts;
        std::map<uint64_t, VkDescriptorSet> cachedDescriptors;
        std::vector<RequestedSet> requestedSets;
        physis_Shader vertexShader, pixelShader;
    };

    // combined vertex + pixel code length
    std::unordered_map<uint32_t, CachedPipeline> m_cachedPipelines;

    Renderer &m_renderer;
    GameData *m_data = nullptr;
    VkExtent2D m_extent = {640, 480};

    VkDescriptorSet createDescriptorFor(const CachedPipeline &cachedPipeline, int i);

    struct UniformBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
        size_t size;
    };

    UniformBuffer createUniformBuffer(size_t size);
    void copyDataToUniform(UniformBuffer &uniformBuffer, void *data, size_t size);

    // Structure definitions from https://github.com/Shaderlayan/Ouroboros
    struct CameraParameter {
        glm::mat3x4 m_ViewMatrix;
        glm::mat3x4 m_InverseViewMatrix;
        glm::mat4 m_InverseViewProjectionMatrix;
        glm::mat4 m_InverseProjectionMatrix;
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewProjectionMatrix;
        /*glm::mat4 m_MainViewToProjectionMatrix;
        glm::vec3 m_EyePosition;
        glm::vec3 m_LookAtVector;*/
    };

    UniformBuffer g_CameraParameter;

    struct JointMatrixArray {
        glm::mat3x4 g_JointMatrixArray[64];
    };

    UniformBuffer g_JointMatrixArray;

    struct CameraLight {
        glm::vec4 m_DiffuseSpecular;
        glm::vec4 m_Rim;
    };

    struct InstanceParameterStruct {
        glm::vec4 m_MulColor;
        glm::vec4 m_EnvParameter;
        CameraLight m_CameraLight;
        glm::vec4 m_Wetness;
    };

    struct InstanceParameter {
        InstanceParameterStruct g_InstanceParameter;
    };

    UniformBuffer g_InstanceParameter;

    struct ModelParameterStruct {
        glm::vec4 m_Params;
    };

    struct ModelParameter {
        ModelParameterStruct g_ModelParameter;
    };

    UniformBuffer g_ModelParameter;

    struct MaterialParameter {
        glm::vec3 g_DiffuseColor;
        float g_AlphaThreshold;
    };

    UniformBuffer g_MaterialParameter;

    VkBuffer m_planeVertexBuffer;
    VkDeviceMemory m_planeVertexMemory;

    struct VulkanImage {
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;
    };

    VulkanImage createImage(int width, int height, VkFormat format, VkImageUsageFlags usage);

    VulkanImage normalGBuffer;
};