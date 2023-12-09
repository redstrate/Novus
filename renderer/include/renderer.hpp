// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <array>
#include <glm/ext/matrix_float4x4.hpp>
#include <map>
#include <vector>
#include <vulkan/vulkan.h>

#include <physis.hpp>

struct RenderPart {
    size_t numIndices;

    VkBuffer vertexBuffer, indexBuffer;
    VkDeviceMemory vertexMemory, indexMemory;

    int materialIndex = 0;
};

struct RenderTexture {
    VkImage handle = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
};

enum class MaterialType { Object, Skin };

struct RenderMaterial {
    MaterialType type = MaterialType::Object;

    RenderTexture *diffuseTexture = nullptr;
    RenderTexture *normalTexture = nullptr;
    RenderTexture *specularTexture = nullptr;
    RenderTexture *multiTexture = nullptr;
};

struct RenderModel {
    QString name;

    physis_MDL model;
    std::vector<RenderPart> parts;
    std::array<glm::mat4, 128> boneData;
    std::vector<RenderMaterial> materials;

    uint16_t from_body_id = 101;
    uint16_t to_body_id = 101;

    VkBuffer boneInfoBuffer = VK_NULL_HANDLE;
    VkDeviceMemory boneInfoMemory = VK_NULL_HANDLE;
};

class ImGuiPass;
struct ImGuiContext;

class Renderer
{
public:
    Renderer();

    void initPipeline();
    void initDescriptors();
    void initDepth(int width, int height);
    bool initSwapchain(VkSurfaceKHR surface, int width, int height);
    void resize(VkSurfaceKHR surface, int width, int height);

    RenderModel addModel(const physis_MDL &model, int lod);
    void reloadModel(RenderModel &model, int lod);
    RenderTexture addTexture(uint32_t width, uint32_t height, const uint8_t *data, uint32_t data_size);

    void render(std::vector<RenderModel> models);

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE, presentQueue = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkExtent2D swapchainExtent;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkRenderPass renderPass;
    std::array<VkCommandBuffer, 3> commandBuffers;
    std::array<VkFence, 3> inFlightFences;
    std::array<VkSemaphore, 3> imageAvailableSemaphores, renderFinishedSemaphores;
    uint32_t currentFrame = 0;

    VkImage depthImage;
    VkDeviceMemory depthMemory;
    VkImageView depthView;

    VkImage dummyImage;
    VkDeviceMemory dummyMemory;
    VkImageView dummyView;
    VkSampler dummySampler;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;

    std::map<uint64_t, VkDescriptorSet> cachedDescriptors;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    std::tuple<VkBuffer, VkDeviceMemory> createBuffer(size_t size, VkBufferUsageFlags usageFlags);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkShaderModule createShaderModule(const uint32_t *code, const int length);

    VkShaderModule loadShaderFromDisk(const std::string_view path);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer pT);

    void inlineTransitionImageLayout(VkCommandBuffer commandBuffer,
                                     VkImage image,
                                     VkFormat format,
                                     VkImageAspectFlags aspect,
                                     VkImageSubresourceRange range,
                                     VkImageLayout oldLayout,
                                     VkImageLayout newLayout,
                                     VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                     VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VkDescriptorSet createDescriptorFor(const RenderModel &model, const RenderMaterial &material);

    uint64_t hash(const RenderModel &model, const RenderMaterial &material);

    glm::mat4 view;

    ImGuiContext *ctx = nullptr;

private:
    void createDummyTexture();

    ImGuiPass *imGuiPass = nullptr;
};