#include "renderer.hpp"

#include <vulkan/vulkan.h>
#include <fmt/core.h>
#include <array>
#include <vector>
#include <valarray>
#include <fstream>

Renderer::Renderer() {
    VkApplicationInfo applicationInfo = {};

    const std::array<const char*, 4> instanceExtensions = {"VK_KHR_surface", "VK_KHR_xcb_surface", "VK_EXT_debug_utils", "VK_KHR_xlib_surface"};

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();
    createInfo.enabledExtensionCount = instanceExtensions.size();

    vkCreateInstance(&createInfo, nullptr, &instance);

    // pick physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (auto device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
    }

    physicalDevice = devices[0];

    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                         &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateDeviceExtensionProperties(
            physicalDevice, nullptr, &extensionCount, extensionProperties.data());

    // we want to choose the portability subset on platforms that
    // support it, this is a requirement of the portability spec
    std::vector<const char*> deviceExtensions = {"VK_KHR_swapchain"};
        for (auto extension : extensionProperties) {
        if (!strcmp(extension.extensionName, "VK_KHR_portability_subset"))
            deviceExtensions.push_back("VK_KHR_portability_subset");
    }

    uint32_t graphicsFamilyIndex = 0, presentFamilyIndex = 0;

    // create logical device
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 &&
            queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamilyIndex = i;
        }

        i++;
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    if (graphicsFamilyIndex == presentFamilyIndex) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicsFamilyIndex;
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    } else {
        // graphics
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = graphicsFamilyIndex;
            queueCreateInfo.queueCount = 1;

            float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        // present
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = presentFamilyIndex;
            queueCreateInfo.queueCount = 1;

            float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }
    }

    VkDeviceCreateInfo deviceCeateInfo = {};
    deviceCeateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCeateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCeateInfo.queueCreateInfoCount =
            static_cast<uint32_t>(queueCreateInfos.size());
    deviceCeateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCeateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());

    VkPhysicalDeviceFeatures enabledFeatures = {};

    vkCreateDevice(physicalDevice, &deviceCeateInfo, nullptr, &device);

    // get queues
    vkGetDeviceQueue(device, graphicsFamilyIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentFamilyIndex, 0, &presentQueue);

    // command pool
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);

    initPipeline();

    fmt::print("Initialized renderer!\n");
}

void Renderer::initSwapchain(VkSurfaceKHR surface, int width, int height) {
    vkQueueWaitIdle(presentQueue);

    // TODO: fix this pls
    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0,
                                         surface, &supported);

    // query swapchain support
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physicalDevice, surface, &capabilities);

    std::vector<VkSurfaceFormatKHR> formats;

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice, surface, &formatCount, nullptr);

    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice, surface, &formatCount, formats.data());

    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice, surface, &presentModeCount, nullptr);

    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice, surface, &presentModeCount,
            presentModes.data());

    // choosing swapchain features
    VkSurfaceFormatKHR swapchainSurfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchainSurfaceFormat = availableFormat;
        }
    }

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapchainPresentMode = availablePresentMode;
        }
    }

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 &&
        imageCount > capabilities.maxImageCount) {
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

    VkSwapchainKHR oldSwapchain = swapchain;
    createInfo.oldSwapchain = oldSwapchain;

    vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);

    if(oldSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(device, oldSwapchain, nullptr);

    swapchainExtent.width = width;
    swapchainExtent.height = height;

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount,
                            nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

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

        vkCreateImageView(device, &view_create_info, nullptr,&swapchainViews[i]);
    }

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchainSurfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);

    swapchainFramebuffers.resize(swapchainViews.size());

    for (size_t i = 0; i < swapchainViews.size(); i++) {
        VkImageView attachments[] = {swapchainViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]);
    }

    // allocate command buffers
    for(int i = 0; i < 3; i++) {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[i]);
    }

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < 3; i++) {
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
        vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFences[i]);
    }
}

void Renderer::resize(VkSurfaceKHR surface, int width, int height) {
    initSwapchain(surface, width, height);
}

void Renderer::render() {
    vkWaitForFences(
            device, 1,
            &inFlightFences[currentFrame],
            VK_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(
            device, swapchain,
            std::numeric_limits<uint64_t>::max(),
            imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        fmt::print("error out of date\n");
        return;
    }

    VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchainFramebuffers[imageIndex];

    static float i = 0;
    VkClearValue clearValue = {};
    clearValue.color.float32[0] = sin(i);
    i += 0.01;
    clearValue.color.float32[3] = 1;

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;
    renderPassInfo.renderArea.extent = swapchainExtent;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1,&inFlightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        return;

    // present
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % 3;
}

std::tuple<VkBuffer, VkDeviceMemory> Renderer::createBuffer(size_t size, VkBufferUsageFlags usageFlags) {
    vkDeviceWaitIdle(device);

    // create buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer handle;
    vkCreateBuffer(device, &bufferInfo, nullptr, &handle);

    // allocate memory
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, handle, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
            findMemoryType(memRequirements.memoryTypeBits,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDeviceMemory memory;
    vkAllocateMemory(device, &allocInfo, nullptr, &memory);

    vkBindBufferMemory(device, handle, memory, 0);

    return {handle, memory};
}

uint32_t Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
            return i;
        }
    }

    return -1;
}

RenderModel Renderer::addModel(const Model& model) {
    RenderModel renderModel;

    size_t vertexSize = model.lods[0].parts[0].vertices.size() * sizeof(float) * 3;
    auto [vertexBuffer, vertexMemory] = createBuffer(vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    size_t indexSize = model.lods[0].parts[0].indices.size() * sizeof(uint16_t);
    auto [indexBuffer, indexMemory] = createBuffer(indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    // copy vertex data
    {
        void* mapped_data = nullptr;
        vkMapMemory(device, vertexMemory, 0, vertexSize, 0, &mapped_data);

        for(int i = 0; i < model.lods[0].parts[0].vertices.size(); i++) {
            memcpy(mapped_data, model.lods[0].parts[0].vertices[i].position.data(), sizeof(float) * 3);
        }

        VkMappedMemoryRange range = {};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = vertexMemory;
        range.size = vertexSize;
        vkFlushMappedMemoryRanges(device, 1, &range);

        vkUnmapMemory(device, vertexMemory);
    }

    // copy index data
    {
        void* mapped_data = nullptr;
        vkMapMemory(device, indexMemory, 0, indexSize, 0, &mapped_data);

        memcpy(mapped_data, model.lods[0].parts[0].indices.data(), indexSize);

        VkMappedMemoryRange range = {};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = indexMemory;
        range.size = indexSize;
        vkFlushMappedMemoryRanges(device, 1, &range);

        vkUnmapMemory(device, indexMemory);
    }

    return renderModel;
}

void Renderer::initPipeline() {
    auto vertexModule = loadShaderFromDisk("mesh.vert.spv");
    auto fragmentModule = loadShaderFromDisk("mesh.frag.spv");
}

VkShaderModule Renderer::createShaderModule(const uint32_t* code, const int length) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = length;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

    return shaderModule;
}

VkShaderModule Renderer::loadShaderFromDisk(const std::string_view path) {
    std::ifstream file(path.data(), std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("failed to open shader file {}", path));
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    return createShaderModule(reinterpret_cast<const uint32_t *>(buffer.data()), fileSize);
}