#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

class Device;

class SwapChain
{
public:
    SwapChain(Device &device, VkSurfaceKHR surface, int width, int height);

    void resize(VkSurfaceKHR surface, int width, int height);

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkExtent2D extent;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainViews;
    std::array<VkFence, 3> inFlightFences;
    std::array<VkSemaphore, 3> imageAvailableSemaphores, renderFinishedSemaphores;
    uint32_t currentFrame = 0;
    VkFormat surfaceFormat;

private:
    Device &m_device;
};