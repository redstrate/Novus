// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "swapchain.h"

#include "device.h"

SwapChain::SwapChain(Device &device, VkSurfaceKHR surface, int width, int height)
    : m_device(device)
{
    resize(surface, width, height);
}

void SwapChain::resize(VkSurfaceKHR surface, int width, int height)
{
    vkQueueWaitIdle(m_device.presentQueue);

    if (width == 0 || height == 0)
        return;

    // TODO: fix this pls
    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_device.physicalDevice, 0, surface, &supported);

    // query swapchain support
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device.physicalDevice, surface, &capabilities);

    std::vector<VkSurfaceFormatKHR> formats;

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_device.physicalDevice, surface, &formatCount, nullptr);

    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_device.physicalDevice, surface, &formatCount, formats.data());

    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_device.physicalDevice, surface, &presentModeCount, nullptr);

    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_device.physicalDevice, surface, &presentModeCount, presentModes.data());

    // choosing swapchain features
    VkSurfaceFormatKHR swapchainSurfaceFormat = formats[0];
    for (const auto &availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchainSurfaceFormat = availableFormat;
        }
    }

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto &availablePresentMode : presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapchainPresentMode = availablePresentMode;
        }
    }

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    // create swapchain
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = swapchainSurfaceFormat.format;
    createInfo.imageColorSpace = swapchainSurfaceFormat.colorSpace;
    createInfo.imageExtent.width = width;
    createInfo.imageExtent.height = height;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = swapchainPresentMode;
    createInfo.clipped = VK_TRUE;

    surfaceFormat = swapchainSurfaceFormat.format;

    VkSwapchainKHR oldSwapchain = swapchain;
    createInfo.oldSwapchain = oldSwapchain;

    vkCreateSwapchainKHR(m_device.device, &createInfo, nullptr, &swapchain);

    if (oldSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(m_device.device, oldSwapchain, nullptr);

    extent.width = width;
    extent.height = height;

    vkGetSwapchainImagesKHR(m_device.device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device.device, swapchain, &imageCount, swapchainImages.data());

    swapchainViews.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo view_create_info = {};
        view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.image = swapchainImages[i];
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_create_info.format = swapchainSurfaceFormat.format;
        view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_create_info.subresourceRange.baseMipLevel = 0;
        view_create_info.subresourceRange.levelCount = 1;
        view_create_info.subresourceRange.baseArrayLayer = 0;
        view_create_info.subresourceRange.layerCount = 1;

        vkCreateImageView(m_device.device, &view_create_info, nullptr, &swapchainViews[i]);
    }

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < 3; i++) {
        vkCreateSemaphore(m_device.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
        vkCreateSemaphore(m_device.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
        vkCreateFence(m_device.device, &fenceCreateInfo, nullptr, &inFlightFences[i]);
    }
}
