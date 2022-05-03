#include "renderer.hpp"

#include <vulkan/vulkan.h>
#include <fmt/core.h>
#include <array>
#include <vector>
#include <valarray>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

Renderer::Renderer() {
    VkApplicationInfo applicationInfo = {};

    std::vector<const char*> instanceExtensions = {"VK_EXT_debug_utils"};

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    for(auto& extension : extensions) {
        if (strstr(extension.extensionName, "surface") != nullptr) {
            instanceExtensions.push_back(extension.extensionName);
        }

        if (strstr(extension.extensionName, "VK_KHR_get_physical_device_properties2") != nullptr) {
            instanceExtensions.push_back(extension.extensionName);
        }
    }

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

    extensionCount = 0;
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

    fmt::print("Initialized renderer!\n");
}

bool Renderer::initSwapchain(VkSurfaceKHR surface, int width, int height) {
    vkQueueWaitIdle(presentQueue);

    if(width == 0 || height == 0)
        return false;

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

    initDepth(width, height);

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

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);

    initDescriptors();
    initPipeline();

    swapchainFramebuffers.resize(swapchainViews.size());

    for (size_t i = 0; i < swapchainViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {swapchainViews[i], depthView};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
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

    return true;
}

void Renderer::resize(VkSurfaceKHR surface, int width, int height) {
    initSwapchain(surface, width, height);
}

void Renderer::render(std::vector<RenderModel> models) {
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

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color.float32[0] = 0.8;
    clearValues[0].color.float32[1] = 0.8;
    clearValues[0].color.float32[2] = 0.8;
    clearValues[0].color.float32[3] = 1.0;
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();
    renderPassInfo.renderArea.extent = swapchainExtent;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    for(auto model : models) {
        // copy bone data
        {
            const size_t bufferSize = sizeof(glm::mat4) * 128;
            void *mapped_data = nullptr;
            vkMapMemory(device, boneInfoMemory, 0, bufferSize, 0, &mapped_data);

            memcpy(mapped_data, model.boneData.data(), bufferSize);

            VkMappedMemoryRange range = {};
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.memory = boneInfoMemory;
            range.size = bufferSize;
            vkFlushMappedMemoryRanges(device, 1, &range);

            vkUnmapMemory(device, boneInfoMemory);
        }

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0, nullptr);

        for(auto part : model.parts) {
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &part.vertexBuffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, part.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

            glm::mat4 p = glm::perspective(glm::radians(45.0f), swapchainExtent.width / (float) swapchainExtent.height,
                                           0.1f, 100.0f);
            p[1][1] *= -1;
            glm::mat4 v = glm::lookAt(glm::vec3(0, 0, -3), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
            glm::mat4 vp = p * v;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &vp);

            glm::mat4 m = glm::mat4(1.0f);

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), sizeof(glm::mat4), &m);

            int test = 0;
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4) * 2, sizeof(int), &test);

            vkCmdDrawIndexed(commandBuffer, part.numIndices, 1, 0, 0, 0);
        }
    }

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

RenderModel Renderer::addModel(const Model& model, int lod) {
    RenderModel renderModel;
    renderModel.model = model;

    if(lod < 0 || lod > model.lods.size())
        return {};

    for(auto part : model.lods[lod].parts) {
        RenderPart renderPart;
        renderPart.submeshes = part.submeshes;

        size_t vertexSize = part.vertices.size() * sizeof(Vertex);
        auto[vertexBuffer, vertexMemory] = createBuffer(vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        size_t indexSize = part.indices.size() * sizeof(uint16_t);
        auto[indexBuffer, indexMemory] = createBuffer(indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        // copy vertex data
        {
            void* mapped_data = nullptr;
            vkMapMemory(device, vertexMemory, 0, vertexSize, 0, &mapped_data);

            memcpy(mapped_data, part.vertices.data(), vertexSize);

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

            memcpy(mapped_data, part.indices.data(), indexSize);

            VkMappedMemoryRange range = {};
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.memory = indexMemory;
            range.size = indexSize;
            vkFlushMappedMemoryRanges(device, 1, &range);

            vkUnmapMemory(device, indexMemory);
        }

        renderPart.numIndices = part.indices.size();

        renderPart.vertexBuffer = vertexBuffer;
        renderPart.vertexMemory = vertexMemory;

        renderPart.indexBuffer = indexBuffer;
        renderPart.indexMemory = indexMemory;

        renderModel.parts.push_back(renderPart);
    }

    return renderModel;
}

void Renderer::initPipeline() {
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = loadShaderFromDisk("mesh.vert.spv");
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = loadShaderFromDisk("mesh.frag.spv");
    fragmentShaderStageInfo.pName = "main";

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStageInfo, fragmentShaderStageInfo};

    VkVertexInputBindingDescription binding = {};
    binding.stride = sizeof(Vertex);

    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(Vertex, position);

    VkVertexInputAttributeDescription normalAttribute = {};
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.location = 1;
    normalAttribute.offset = offsetof(Vertex, normal);

    VkVertexInputAttributeDescription boneWeightAttribute = {};
    boneWeightAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    boneWeightAttribute.location = 2;
    boneWeightAttribute.offset = offsetof(Vertex, boneWeights);

    VkVertexInputAttributeDescription boneIdAttribute = {};
    boneIdAttribute.format = VK_FORMAT_R8G8B8A8_UINT;
    boneIdAttribute.location = 3;
    boneIdAttribute.offset = offsetof(Vertex, boneIds);

    std::array<VkVertexInputAttributeDescription, 4> attributes = {positionAttribute, normalAttribute, boneWeightAttribute, boneIdAttribute};

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &binding;
    vertexInputState.vertexAttributeDescriptionCount = attributes.size();
    vertexInputState.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport = {};
    viewport.width = swapchainExtent.width;
    viewport.height = swapchainExtent.height;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.extent = swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.size = (sizeof(glm::mat4) * 2) + sizeof(int);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &setLayout;

    vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.maxDepthBounds = 1.0f;

    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = shaderStages.size();
    createInfo.pStages = shaderStages.data();
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssembly;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizer;
    createInfo.pMultisampleState = &multisampling;
    createInfo.pColorBlendState = &colorBlending;
    createInfo.pDynamicState = &dynamicState;
    createInfo.pDepthStencilState = &depthStencil;
    createInfo.layout = pipelineLayout;
    createInfo.renderPass = renderPass;

    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
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

void Renderer::initDescriptors() {
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.pPoolSizes = &poolSize;
    poolCreateInfo.maxSets = 1;

    vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool);

    VkDescriptorSetLayoutBinding boneInfoBufferBinding = {};
    boneInfoBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    boneInfoBufferBinding.descriptorCount = 1;
    boneInfoBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    boneInfoBufferBinding.binding = 2;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &boneInfoBufferBinding;

    vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &setLayout);

    const size_t bufferSize = sizeof(glm::mat4) * 128;
    auto [buffer, memory] = createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    boneInfoBuffer = buffer;
    boneInfoMemory = memory;

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &setLayout;

    vkAllocateDescriptorSets(device, &allocateInfo, &set);

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = boneInfoBuffer;
    bufferInfo.range = bufferSize;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = set;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.dstBinding = 2;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void Renderer::initDepth(int width, int height) {
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateImage(device, &imageCreateInfo, nullptr, &depthImage);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, depthImage, &memRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device, &allocateInfo, nullptr, &depthMemory);

    vkBindImageMemory(device, depthImage, depthMemory, 0);

    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = depthImage;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = VK_FORMAT_D32_SFLOAT;
    viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(device, &viewCreateInfo, nullptr, &depthView);
}
