// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rendermanager.h"

#include <QDebug>
#include <QFile>
#include <array>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <valarray>
#include <vector>
#include <vulkan/vulkan.h>

#include "gamerenderer.h"
#include "imgui.h"
#include "imguipass.h"
#include "pass.h"
#include "simplerenderer.h"
#include "swapchain.h"

#include <magic_enum/include/magic_enum.hpp>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pCallback)
{
    // Note: It seems that static_cast<...> doesn't work. Use the C-style forced
    // cast.
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                             void *pUserData)
{
    Q_UNUSED(messageSeverity)
    Q_UNUSED(messageType)
    Q_UNUSED(pUserData)

    qInfo() << pCallbackData->pMessage;

    return VK_FALSE;
}

RenderManager::RenderManager(GameData *data)
    : m_data(data)
{
    Q_INIT_RESOURCE(shaders);

    m_device = new Device();

    ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(ctx);

    ImGui::GetIO().IniFilename = "";

    ImGui::StyleColorsDark();

    std::vector<const char *> instanceExtensions = {"VK_EXT_debug_utils"};

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    for (auto &extension : extensions) {
        if (strstr(extension.extensionName, "surface") != nullptr) {
            instanceExtensions.push_back(extension.extensionName);
        }

        if (strstr(extension.extensionName, "VK_KHR_get_physical_device_properties2") != nullptr) {
            instanceExtensions.push_back(extension.extensionName);
        }

        if (strstr(extension.extensionName, "VK_EXT_debug_utils") != nullptr) {
            instanceExtensions.push_back(extension.extensionName);
        }

        // needed for VK_EXT_display_surface_counter
        if (strstr(extension.extensionName, "VK_KHR_display") != nullptr) {
            instanceExtensions.push_back(extension.extensionName);
        }
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = DebugCallback;

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    const char *layers[] = {"VK_LAYER_KHRONOS_validation"};

    VkInstanceCreateInfo createInfo = {};
    createInfo.pNext = &debugCreateInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();
    createInfo.enabledExtensionCount = instanceExtensions.size();
    createInfo.pApplicationInfo = &applicationInfo;

    if (qgetenv("NOVUS_ENABLE_VALIDATION") == QByteArrayLiteral("1")) {
        createInfo.ppEnabledLayerNames = layers;
        createInfo.enabledLayerCount = 1;
    }

    vkCreateInstance(&createInfo, nullptr, &m_device->instance);

    VkDebugUtilsMessengerEXT callback;
    CreateDebugUtilsMessengerEXT(m_device->instance, &debugCreateInfo, nullptr, &callback);

    // pick physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_device->instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_device->instance, &deviceCount, devices.data());

    int preferredDevice = 0;
    int deviceIndex = 0;
    for (auto device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            preferredDevice = deviceIndex;
        }
        deviceIndex++;
    }

    m_device->physicalDevice = devices[preferredDevice];

    extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(m_device->physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateDeviceExtensionProperties(m_device->physicalDevice, nullptr, &extensionCount, extensionProperties.data());

    // we want to choose the portability subset on platforms that
    // support it, this is a requirement of the portability spec
    std::vector<const char *> deviceExtensions = {"VK_KHR_swapchain"};
    for (auto extension : extensionProperties) {
        if (!strcmp(extension.extensionName, "VK_KHR_portability_subset"))
            deviceExtensions.push_back("VK_KHR_portability_subset");
    }

    uint32_t graphicsFamilyIndex = 0, presentFamilyIndex = 0;

    // create logical device
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_device->physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_device->physicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
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

    VkPhysicalDeviceFeatures enabledFeatures{};
    enabledFeatures.shaderClipDistance = VK_TRUE;
    enabledFeatures.shaderCullDistance = VK_TRUE;
    enabledFeatures.fillModeNonSolid = VK_TRUE;
    enabledFeatures.imageCubeArray = VK_TRUE;

    // TODO: Update the Flatpak Vulkan SDK to support this
#if defined(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME)
    VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR localReadFeaturesKhr{};
    localReadFeaturesKhr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR;
    localReadFeaturesKhr.dynamicRenderingLocalRead = VK_TRUE;
#endif

    VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT unusedAttachmentsFeaturesExt{};
    unusedAttachmentsFeaturesExt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT;
    unusedAttachmentsFeaturesExt.dynamicRenderingUnusedAttachments = VK_TRUE;

#if defined(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME)
    // TODO: do we really need this?
    // unusedAttachmentsFeaturesExt.pNext = &localReadFeaturesKhr;
#endif

    VkPhysicalDeviceVulkan11Features enabled11Features{};
    enabled11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    enabled11Features.shaderDrawParameters = VK_TRUE;
    enabled11Features.pNext = &unusedAttachmentsFeaturesExt;

    VkPhysicalDeviceVulkan12Features enabled12Features{};
    enabled12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    enabled12Features.vulkanMemoryModel = VK_TRUE;
    enabled12Features.pNext = &enabled11Features;

    VkPhysicalDeviceVulkan13Features enabled13Features{};
    enabled13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    enabled13Features.shaderDemoteToHelperInvocation = VK_TRUE;
    enabled13Features.dynamicRendering = VK_TRUE;
    enabled13Features.synchronization2 = VK_TRUE;
    enabled13Features.pNext = &enabled12Features;

    VkDeviceCreateInfo deviceCeateInfo = {};
    deviceCeateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCeateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCeateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCeateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCeateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCeateInfo.pEnabledFeatures = &enabledFeatures;
    deviceCeateInfo.pNext = &enabled13Features;

    vkCreateDevice(m_device->physicalDevice, &deviceCeateInfo, nullptr, &m_device->device);

    // get queues
    vkGetDeviceQueue(m_device->device, graphicsFamilyIndex, 0, &m_device->graphicsQueue);
    vkGetDeviceQueue(m_device->device, presentFamilyIndex, 0, &m_device->presentQueue);

    // command pool
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCreateCommandPool(m_device->device, &poolInfo, nullptr, &m_device->commandPool);

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = 150;

    VkDescriptorPoolSize poolSize2 = {};
    poolSize2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize2.descriptorCount = 150;

    VkDescriptorPoolSize poolSize3 = {};
    poolSize3.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSize3.descriptorCount = 150;

    VkDescriptorPoolSize poolSize4 = {};
    poolSize4.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize4.descriptorCount = 150;

    VkDescriptorPoolSize poolSize5 = {};
    poolSize5.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSize5.descriptorCount = 150;

    const std::array poolSizes = {poolSize, poolSize2, poolSize3, poolSize4, poolSize5};

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = poolSizes.size();
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.maxSets = 150;

    vkCreateDescriptorPool(m_device->device, &poolCreateInfo, nullptr, &m_device->descriptorPool);

    qInfo() << "Initialized renderer!";
}

bool RenderManager::initSwapchain(VkSurfaceKHR surface, int width, int height)
{
    if (m_device->swapChain == nullptr) {
        m_device->swapChain = new SwapChain(*m_device, surface, width, height);
    } else {
        m_device->swapChain->resize(surface, width, height);
    }

    // allocate command buffers
    for (int i = 0; i < 3; i++) {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_device->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(m_device->device, &allocInfo, &m_commandBuffers[i]);
    }

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_device->swapChain->surfaceFormat;
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
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vkCreateRenderPass(m_device->device, &renderPassInfo, nullptr, &m_renderPass);

    ImGui::SetCurrentContext(ctx);
    m_imGuiPass = new ImGuiPass(*this);

    if (qgetenv("NOVUS_USE_NEW_RENDERER") == QByteArrayLiteral("1")) {
        m_renderer = new GameRenderer(*m_device, m_data);
    } else {
        m_renderer = new SimpleRenderer(*m_device);
    }

    m_renderer->resize();
    initBlitPipeline(); // this creates a desc set for the renderer's offscreen texture. need to make sure we regen it

    m_framebuffers.resize(m_device->swapChain->swapchainImages.size());
    for (size_t i = 0; i < m_device->swapChain->swapchainImages.size(); i++) {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &m_device->swapChain->swapchainViews[i];
        framebufferInfo.width = m_device->swapChain->extent.width;
        framebufferInfo.height = m_device->swapChain->extent.height;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(m_device->device, &framebufferInfo, nullptr, &m_framebuffers[i]);
    }

    return true;
}

void RenderManager::resize(VkSurfaceKHR surface, int width, int height)
{
    initSwapchain(surface, width, height);
}

void RenderManager::destroySwapchain()
{
    if (m_device->swapChain->swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device->device, m_device->swapChain->swapchain, nullptr);
        m_device->swapChain->swapchain = VK_NULL_HANDLE;
    }
}

void RenderManager::render(const std::vector<DrawObjectInstance> &models)
{
    vkWaitForFences(m_device->device,
                    1,
                    &m_device->swapChain->inFlightFences[m_device->swapChain->currentFrame],
                    VK_TRUE,
                    std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(m_device->device,
                                            m_device->swapChain->swapchain,
                                            std::numeric_limits<uint64_t>::max(),
                                            m_device->swapChain->imageAvailableSemaphores[m_device->swapChain->currentFrame],
                                            VK_NULL_HANDLE,
                                            &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }

    VkCommandBuffer commandBuffer = m_commandBuffers[m_device->swapChain->currentFrame];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    updateCamera(camera);

    m_renderer->render(commandBuffer, camera, scene, models);

    // render extra passes
    // TODO: support the new renderer
    if (qgetenv("NOVUS_USE_NEW_RENDERER") != QByteArrayLiteral("1")) {
        for (const auto &pass : m_passes) {
            pass->render(commandBuffer, camera);
        }

        vkCmdEndRenderPass(commandBuffer);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffers[imageIndex];

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color.float32[0] = 0.24;
    clearValues[0].color.float32[1] = 0.24;
    clearValues[0].color.float32[2] = 0.24;
    clearValues[0].color.float32[3] = 1.0;
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();
    renderPassInfo.renderArea.extent = m_device->swapChain->extent;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);

    vkCmdDraw(commandBuffer, 4, 1, 0, 0);

    // Render offscreen texture, and overlay imgui
    if (m_imGuiPass != nullptr) {
        ImGui::SetCurrentContext(ctx);
        m_imGuiPass->render(commandBuffer);
    }

    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_device->swapChain->imageAvailableSemaphores[m_device->swapChain->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {m_device->swapChain->renderFinishedSemaphores[m_device->swapChain->currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(m_device->device, 1, &m_device->swapChain->inFlightFences[m_device->swapChain->currentFrame]);

    if (vkQueueSubmit(m_device->graphicsQueue, 1, &submitInfo, m_device->swapChain->inFlightFences[m_device->swapChain->currentFrame]) != VK_SUCCESS)
        return;

    // present
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {m_device->swapChain->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_device->presentQueue, &presentInfo);

    m_device->swapChain->currentFrame = (m_device->swapChain->currentFrame + 1) % 3;
}

VkRenderPass RenderManager::presentationRenderPass() const
{
    return m_renderPass;
}

DrawObject *RenderManager::addDrawObject(const physis_MDL &model, int lod)
{
    auto DrawObject = new ::DrawObject();
    DrawObject->model = model;

    reloadDrawObject(*DrawObject, lod);

    return DrawObject;
}

void RenderManager::reloadDrawObject(DrawObject &DrawObject, uint32_t lod)
{
    if (lod > DrawObject.model.num_lod)
        return;

    DrawObject.parts.clear();

    for (uint32_t i = 0; i < DrawObject.model.lods[lod].num_parts; i++) {
        RenderPart renderPart;

        const physis_Part part = DrawObject.model.lods[lod].parts[i];

        renderPart.originalPart = part;
        renderPart.materialIndex = part.material_index;

        if (qgetenv("NOVUS_USE_NEW_RENDERER") == QByteArrayLiteral("1")) {
            renderPart.streamBuffer.resize(DrawObject.model.lods[lod].num_vertex_elements);
            for (uint32_t j = 0; j < part.num_streams; j++) {
                size_t size = part.stream_sizes[j];
                auto buffer = m_device->createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
                m_device->copyToBuffer(buffer, (void *)part.streams[j], size);

                renderPart.streamBuffer[j] = buffer;
            }
        } else {
            size_t vertexSize = part.num_vertices * sizeof(Vertex);
            renderPart.vertexBuffer = m_device->createBuffer(vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            m_device->copyToBuffer(renderPart.vertexBuffer, (void *)part.vertices, vertexSize);
        }

        size_t indexSize = part.num_indices * sizeof(uint16_t);
        renderPart.indexBuffer = m_device->createBuffer(indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        m_device->copyToBuffer(renderPart.indexBuffer, (void *)part.indices, indexSize);

        renderPart.numIndices = part.num_indices;

        DrawObject.parts.push_back(renderPart);
    }

    const size_t bufferSize = sizeof(glm::mat4) * JOINT_MATRIX_SIZE_DAWNTRAIL;
    DrawObject.boneInfoBuffer = m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
}

Texture RenderManager::addGameTexture(VkFormat format, physis_Texture gameTexture)
{
    return m_device->addGameTexture(format, gameTexture);
}

Device &RenderManager::device()
{
    return *m_device;
}

VkSampler RenderManager::defaultSampler()
{
    return m_sampler;
}

void RenderManager::updateCamera(Camera &camera)
{
    camera.aspectRatio = static_cast<float>(m_device->swapChain->extent.width) / static_cast<float>(m_device->swapChain->extent.height);
    camera.perspective = glm::perspective(glm::radians(camera.fieldOfView), camera.aspectRatio, camera.nearPlane, camera.farPlane);
}

void RenderManager::initBlitPipeline()
{
    VkDescriptorSetLayoutBinding binding = {};
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.binding = 0;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;

    vkCreateDescriptorSetLayout(m_device->device, &layoutInfo, nullptr, &m_setLayout);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = m_device->loadShaderFromDisk(":/shaders/blit.vert.spv");
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = m_device->loadShaderFromDisk(":/shaders/blit.frag.spv");
    fragmentShaderStageInfo.pName = "main";

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStageInfo, fragmentShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport = {};
    viewport.width = m_device->swapChain->extent.width;
    viewport.height = m_device->swapChain->extent.height;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.extent = m_device->swapChain->extent;

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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_setLayout;

    vkCreatePipelineLayout(m_device->device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

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
    createInfo.layout = m_pipelineLayout;
    createInfo.renderPass = m_renderPass;

    vkCreateGraphicsPipelines(m_device->device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_pipeline);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.maxLod = 1.0f;

    vkCreateSampler(m_device->device, &samplerInfo, nullptr, &m_sampler);

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = m_device->descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &m_setLayout;

    vkAllocateDescriptorSets(m_device->device, &allocateInfo, &m_descriptorSet);

    VkDescriptorImageInfo multiImageInfo = {};
    multiImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    multiImageInfo.imageView = m_renderer->getCompositeTexture().imageView;
    multiImageInfo.sampler = m_sampler;

    VkWriteDescriptorSet multiDescriptorWrite2 = {};
    multiDescriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    multiDescriptorWrite2.dstSet = m_descriptorSet;
    multiDescriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    multiDescriptorWrite2.descriptorCount = 1;
    multiDescriptorWrite2.pImageInfo = &multiImageInfo;
    multiDescriptorWrite2.dstBinding = 0;

    vkUpdateDescriptorSets(m_device->device, 1, &multiDescriptorWrite2, 0, nullptr);
}

void RenderManager::addPass(RendererPass *pass)
{
    m_passes.push_back(pass);
}

BaseRenderer *RenderManager::renderer()
{
    return m_renderer;
}
