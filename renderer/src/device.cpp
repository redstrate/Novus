// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device.h"

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
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code);

    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

    return shaderModule;
}

VkShaderModule Device::loadShaderFromDisk(const std::string_view path)
{
    QFile file((QLatin1String(path)));
    file.open(QFile::ReadOnly);

    if (!file.isOpen()) {
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

    return texture;
}

Buffer Device::createDummyBuffer()
{
    auto buffer = createBuffer(655360, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

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

Texture Device::addGameTexture(physis_Texture gameTexture)
{
    Texture newTexture = {};

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    switch (gameTexture.texture_type) {
    case TextureType::TwoDimensional:
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        break;
    case TextureType::ThreeDimensional:
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        break;
    }
    imageInfo.extent.width = gameTexture.width;
    imageInfo.extent.height = gameTexture.height;
    imageInfo.extent.depth = gameTexture.depth;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
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
    bufferInfo.size = gameTexture.rgba_size;
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
    vkMapMemory(device, stagingBufferMemory, 0, gameTexture.rgba_size, 0, &mapped_data);
    memcpy(mapped_data, gameTexture.rgba, gameTexture.rgba_size);
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
    region.imageExtent = {(uint32_t)gameTexture.width, (uint32_t)gameTexture.height, (uint32_t)gameTexture.depth};

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, newTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    inlineTransitionImageLayout(commandBuffer,
                                newTexture.image,
                                imageInfo.format,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                range,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    endSingleTimeCommands(commandBuffer);

    range = {};
    range.levelCount = 1;
    range.layerCount = 1;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = newTexture.image;
    switch (gameTexture.texture_type) {
    case TextureType::TwoDimensional:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case TextureType::ThreeDimensional:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    }
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange = range;

    vkCreateImageView(device, &viewInfo, nullptr, &newTexture.imageView);

    return newTexture;
}