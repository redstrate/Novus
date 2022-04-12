#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>

class Renderer {
public:
    Renderer();

    void initSwapchain(VkSurfaceKHR surface);

    void render();

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
};