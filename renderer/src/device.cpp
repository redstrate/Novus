// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device.h"

#include <QDebug>
#include <QFile>

Buffer Device::createBuffer(const size_t size, const VkBufferUsageFlags usageFlags)
{
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
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDeviceMemory memory;
    vkAllocateMemory(device, &allocInfo, nullptr, &memory);

    vkBindBufferMemory(device, handle, memory, 0);

    return {handle, memory, size};
}

void Device::copyToBuffer(Buffer &buffer, void *data, const size_t size)
{
    void *mapped_data;
    vkMapMemory(device, buffer.memory, 0, size, 0, &mapped_data);
    memcpy(mapped_data, data, size);
    vkUnmapMemory(device, buffer.memory);
}

void Device::destroyBuffer(Buffer &buffer)
{
    if (buffer.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, buffer.buffer, nullptr);
        buffer.buffer = VK_NULL_HANDLE;
    }
    if (buffer.memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, buffer.memory, nullptr);
        buffer.memory = VK_NULL_HANDLE;
    }
}

uint32_t Device::findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return -1;
}

VkShaderModule Device::createShaderModule(const uint32_t *code, const int length)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = length;
    createInfo.pCode = code;

    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

    return shaderModule;
}

VkShaderModule Device::loadShaderFromDisk(const std::string_view path)
{
    QFile file((QLatin1String(path)));
    if (!file.open(QFile::ReadOnly)) {
        qFatal("Failed to open shader file: %s", path.data());
    }

    auto contents = file.readAll();
    return createShaderModule(reinterpret_cast<const uint32_t *>(contents.data()), contents.size());
}

Texture Device::createTexture(const int width, const int height, const VkFormat format, const VkImageUsageFlags usage)
{
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory imageMemory;

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateImage(device, &imageCreateInfo, nullptr, &image);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device, &allocateInfo, nullptr, &imageMemory);

    vkBindImageMemory(device, image, imageMemory, 0);

    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = image;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        ? VK_IMAGE_ASPECT_DEPTH_BIT
        : VK_IMAGE_ASPECT_COLOR_BIT; // TODO: hardcoded
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(device, &viewCreateInfo, nullptr, &imageView);

    return {format, viewCreateInfo.subresourceRange, image, imageView, imageMemory};
}

void Device::destroyTexture(Texture &texture)
{
    if (texture.imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, texture.imageView, nullptr);
        texture.imageView = VK_NULL_HANDLE;
    }
    if (texture.image != VK_NULL_HANDLE) {
        vkDestroyImage(device, texture.image, nullptr);
        texture.image = VK_NULL_HANDLE;
    }
    if (texture.imageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, texture.imageMemory, nullptr);
        texture.imageMemory = VK_NULL_HANDLE;
    }
}

Texture Device::createDummyTexture(std::array<uint8_t, 4> values)
{
    auto texture = createTexture(1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    // copy image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = 4;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer);

    // allocate staging memory
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory);

    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    // copy to staging buffer
    void *mapped_data;
    vkMapMemory(device, stagingBufferMemory, 0, 4 * sizeof(uint8_t), 0, &mapped_data);
    memcpy(mapped_data, values.data(), 4 * sizeof(uint8_t));
    vkUnmapMemory(device, stagingBufferMemory);

    // copy staging buffer to image
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageSubresourceRange range = {};
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    inlineTransitionImageLayout(commandBuffer,
                                texture.image,
                                VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                range,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {(uint32_t)1, (uint32_t)1, 1};

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    inlineTransitionImageLayout(commandBuffer,
                                texture.image,
                                VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                range,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    endSingleTimeCommands(commandBuffer);

    // free staging resources
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    return texture;
}

Buffer Device::createDummyBuffer()
{
    auto buffer = createBuffer(655360, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    nameBuffer(buffer, "Dummy Buffer");

    // TODO: fill with data?

    return buffer;
}

VkCommandBuffer Device::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Device::inlineTransitionImageLayout(VkCommandBuffer commandBuffer,
                                         VkImage image,
                                         VkFormat format,
                                         VkImageAspectFlags aspect,
                                         VkImageSubresourceRange range,
                                         VkImageLayout oldLayout,
                                         VkImageLayout newLayout,
                                         VkPipelineStageFlags src_stage_mask,
                                         VkPipelineStageFlags dst_stage_mask)
{
    Q_UNUSED(format)

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = range;
    barrier.subresourceRange.aspectMask = aspect;

    switch (oldLayout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        barrier.srcAccessMask = 0;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_GENERAL:
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        break;
    default:
        break;
    }

    switch (newLayout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_GENERAL:
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        break;
    }

    vkCmdPipelineBarrier(commandBuffer, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Device::transitionTexture(VkCommandBuffer commandBuffer, Texture &texture, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    inlineTransitionImageLayout(commandBuffer, texture.image, texture.format, texture.range.aspectMask, texture.range, oldLayout, newLayout);
}

VkResult Device::nameObject(VkObjectType type, uint64_t object, std::string_view name)
{
    if (object == 0x0) {
        return VK_ERROR_DEVICE_LOST;
    }

    VkDebugUtilsObjectNameInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = type;
    info.pObjectName = name.data();
    info.objectHandle = object;

    auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
    if (func != nullptr)
        return func(device, &info);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Device::nameTexture(Texture &texture, std::string_view name)
{
    nameObject(VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(texture.image), name.data());
    nameObject(VK_OBJECT_TYPE_IMAGE_VIEW, reinterpret_cast<uint64_t>(texture.imageView), name.data());
    nameObject(VK_OBJECT_TYPE_DEVICE_MEMORY, reinterpret_cast<uint64_t>(texture.imageMemory), name.data());
}

void Device::nameBuffer(Buffer &buffer, std::string_view name)
{
    nameObject(VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(buffer.buffer), name.data());
    nameObject(VK_OBJECT_TYPE_DEVICE_MEMORY, reinterpret_cast<uint64_t>(buffer.memory), name.data());
}

Texture Device::addGameTexture(physis_Texture gameTexture)
{
    Texture newTexture = {};

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = gameTexture.mip_levels;
    imageInfo.arrayLayers = 1;

    if (gameTexture.attribute & TextureAttribute_TEXTURE_TYPE1_D) {
        imageInfo.imageType = VK_IMAGE_TYPE_1D;
    }
    if (gameTexture.attribute & TextureAttribute_TEXTURE_TYPE2_D) {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
    }
    if (gameTexture.attribute & TextureAttribute_TEXTURE_TYPE3_D) {
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.extent.depth = gameTexture.depth;
    }
    if (gameTexture.attribute & TextureAttribute_TEXTURE_TYPE2_D_ARRAY) {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = gameTexture.depth; // just a guess
    }
    imageInfo.extent.width = gameTexture.width;
    imageInfo.extent.height = gameTexture.height;
    switch (gameTexture.format) {
    case TextureFormat::A8_UNORM:
        imageInfo.format = VK_FORMAT_A8_UNORM;
        break;
    case TextureFormat::R8_UNORM:
        imageInfo.format = VK_FORMAT_R8_UNORM;
        break;
    case TextureFormat::R8_UINT:
        imageInfo.format = VK_FORMAT_R8_UINT;
        break;
    case TextureFormat::R16_UINT:
        imageInfo.format = VK_FORMAT_R16_UINT;
        break;
    case TextureFormat::R32_UINT:
        imageInfo.format = VK_FORMAT_R32_UINT;
        break;
    case TextureFormat::R8G8_UNORM:
        imageInfo.format = VK_FORMAT_R8G8_UNORM;
        break;
    case TextureFormat::B4G4R4A4_UNORM:
        imageInfo.format = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        break;
    case TextureFormat::B8G8R8A8_UNORM:
        imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
        break;
    case TextureFormat::R16_FLOAT:
        imageInfo.format = VK_FORMAT_R16_SFLOAT;
        break;
    case TextureFormat::R32_FLOAT:
        imageInfo.format = VK_FORMAT_R32_SFLOAT;
        break;
    case TextureFormat::R16G16_FLOAT:
        imageInfo.format = VK_FORMAT_R16G16_SFLOAT;
        break;
    case TextureFormat::R32G32_FLOAT:
        imageInfo.format = VK_FORMAT_R32G32_SFLOAT;
        break;
    case TextureFormat::R16G16B16A16_FLOAT:
        imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        break;
    case TextureFormat::R32G32B32A32_FLOAT:
        imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        break;
    case TextureFormat::BC1_UNORM:
        imageInfo.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        break;
    case TextureFormat::BC2_UNORM:
        imageInfo.format = VK_FORMAT_BC2_UNORM_BLOCK;
        break;
    case TextureFormat::BC3_UNORM:
        imageInfo.format = VK_FORMAT_BC3_UNORM_BLOCK;
        break;
    case TextureFormat::D16_UNORM:
        imageInfo.format = VK_FORMAT_D16_UNORM;
        break;
    case TextureFormat::D24_UNORM_S8_UINT:
        imageInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
        break;
    case TextureFormat::D16_UNORM_2:
        imageInfo.format = VK_FORMAT_D16_UNORM;
        break;
    case TextureFormat::D24_UNORM_S8_UINT_2:
        imageInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
        break;
    case TextureFormat::BC4_UNORM:
        imageInfo.format = VK_FORMAT_BC4_UNORM_BLOCK;
        break;
    case TextureFormat::BC5_UNORM:
        imageInfo.format = VK_FORMAT_BC5_UNORM_BLOCK;
        break;
    case TextureFormat::BC6H_SF16:
        imageInfo.format = VK_FORMAT_BC6H_SFLOAT_BLOCK;
        break;
    case TextureFormat::BC7_UNORM:
        imageInfo.format = VK_FORMAT_BC7_UNORM_BLOCK;
        break;
    case TextureFormat::R16_UNORM:
        imageInfo.format = VK_FORMAT_R16_UNORM;
        break;
    case TextureFormat::R16G16_UNORM:
        imageInfo.format = VK_FORMAT_R16G16_UNORM;
        break;
    default:
        qWarning() << "Unknown texture format:" << static_cast<int>(gameTexture.format);
        break;
    }
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    vkCreateImage(device, &imageInfo, nullptr, &newTexture.image);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, newTexture.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device, &allocInfo, nullptr, &newTexture.imageMemory);

    vkBindImageMemory(device, newTexture.image, newTexture.imageMemory, 0);

    // copy image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = gameTexture.data_size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer);

    // allocate staging memory
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

    allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory);

    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    // copy to staging buffer
    void *mapped_data;
    vkMapMemory(device, stagingBufferMemory, 0, gameTexture.data_size, 0, &mapped_data);
    memcpy(mapped_data, gameTexture.data, gameTexture.data_size);
    vkUnmapMemory(device, stagingBufferMemory);

    // copy staging buffer to image
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageSubresourceRange range = {};
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    inlineTransitionImageLayout(commandBuffer,
                                newTexture.image,
                                imageInfo.format,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                range,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {static_cast<uint32_t>(gameTexture.width), static_cast<uint32_t>(gameTexture.height), static_cast<uint32_t>(gameTexture.depth)};

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, newTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    inlineTransitionImageLayout(commandBuffer,
                                newTexture.image,
                                imageInfo.format,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                range,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    endSingleTimeCommands(commandBuffer);

    // free staging resources
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    range = {};
    range.levelCount = 1;
    range.layerCount = 1;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = newTexture.image;
    if (gameTexture.attribute & TextureAttribute_TEXTURE_TYPE1_D) {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
    }
    if (gameTexture.attribute & TextureAttribute_TEXTURE_TYPE2_D) {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }
    if (gameTexture.attribute & TextureAttribute_TEXTURE_TYPE3_D) {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    }
    if (gameTexture.attribute & TextureAttribute_TEXTURE_TYPE2_D_ARRAY) {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange = range;

    vkCreateImageView(device, &viewInfo, nullptr, &newTexture.imageView);

    return newTexture;
}

void Device::beginDebugMarker(VkCommandBuffer command_buffer, VkDebugUtilsLabelEXT marker_info)
{
    auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT");
    if (func != nullptr)
        func(command_buffer, &marker_info);
}

void Device::endDebugMarker(VkCommandBuffer command_buffer)
{
    auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT");
    if (func != nullptr)
        func(command_buffer);
}

void Device::insertDebugLabel(VkCommandBuffer command_buffer, VkDebugUtilsLabelEXT label_info)
{
    auto func = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdInsertDebugUtilsLabelEXT");
    if (func != nullptr)
        func(command_buffer, &label_info);
}

void Device::waitForIdle()
{
    // Wait until everything is done...
    vkDeviceWaitIdle(device);
    vkQueueWaitIdle(graphicsQueue);
    vkQueueWaitIdle(presentQueue);
}
