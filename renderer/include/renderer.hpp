#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>

#include "mdlparser.h"

struct RenderPart {
    size_t numIndices;

    VkBuffer vertexBuffer, indexBuffer;
    VkDeviceMemory vertexMemory, indexMemory;
};

struct RenderModel {
    Model model;
    std::vector<RenderPart> parts;
};

class Renderer {
public:
    Renderer();

    void initPipeline();
    bool initSwapchain(VkSurfaceKHR surface, int width, int height);
    void resize(VkSurfaceKHR surface, int width, int height);

    RenderModel addModel(const Model& model, int lod);

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
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    std::tuple<VkBuffer, VkDeviceMemory> createBuffer(size_t size, VkBufferUsageFlags usageFlags);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkShaderModule createShaderModule(const uint32_t *code, const int length);

    VkShaderModule loadShaderFromDisk(const std::string_view path);
};