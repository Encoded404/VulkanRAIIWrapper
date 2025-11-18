#include "Device.hpp"

#include "PhysicalDevice.hpp"

#include "../rendering/CommandPool.hpp"

#include <cstdint>
#include <vector>
#include <volk.h>
#include <stdexcept>
#include <set>
#include <cassert>
#include <memory> // Added for std::make_unique
#include <utility> // Added for std::move


namespace VulkanEngine::RAII {

Device::Device(const PhysicalDevice& physical_device,
               const std::vector<const char*>& required_extensions,
               const VkPhysicalDeviceFeatures& required_features,
               const std::vector<const char*>& validation_layers)
    : physicalDevice_(physical_device) {
    queueFamilyIndices_ = physicalDevice_.find_queue_families();
    create_logical_device(required_extensions, required_features, validation_layers);
    // Create the transient/resettable command pool using graphics queue family
    if (!queueFamilyIndices_.graphicsFamily_.has_value()) {
        throw std::runtime_error("Graphics queue family not available for command pool creation");
    }
    singleUseCommandPool_ = std::make_unique<CommandPool>(
        *this,
        queueFamilyIndices_.graphicsFamily_.value(),
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

Device::~Device() {
    if (device_ != VK_NULL_HANDLE) {
        // Ensure all work is complete before tearing down dependent resources
        vkDeviceWaitIdle(device_);
        // Destroy resources that depend on the device first
        singleUseCommandPool_.reset();
        // Destroy the logical device
        vkDestroyDevice(device_, nullptr);
        // Invalidate the handle
        device_ = VK_NULL_HANDLE;
    }
}

Device::Device(Device&& other) noexcept
    : device_(other.device_),
      physicalDevice_(other.physicalDevice_),
      queueFamilyIndices_(other.queueFamilyIndices_),
      singleUseCommandPool_(std::move(other.singleUseCommandPool_)) {
    other.device_ = VK_NULL_HANDLE;
}

Device& Device::operator=(Device&& other) noexcept {
    if (this != &other) {
        assert(&physicalDevice_ == &other.physicalDevice_ && "Move-assigning between different physical devices is not supported");
        if (device_ != VK_NULL_HANDLE) {
            // Make sure any in-flight work is finished before destruction
            vkDeviceWaitIdle(device_);
            // Destroy resources tied to the current device before destroying the device itself
            singleUseCommandPool_.reset();
            vkDestroyDevice(device_, nullptr);
        }
        device_ = other.device_;
        queueFamilyIndices_ = other.queueFamilyIndices_;
        singleUseCommandPool_ = std::move(other.singleUseCommandPool_);
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

void Device::wait_idle() const {
    vkDeviceWaitIdle(device_);
}

VkQueue Device::get_queue(uint32_t queue_family_index, uint32_t queue_index) const {
    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device_, queue_family_index, queue_index, &queue);
    return queue;
}

VkQueue Device::get_graphics_queue() const {
    if (!queueFamilyIndices_.graphicsFamily_.has_value()) {
        throw std::runtime_error("Graphics queue family not available");
    }
    return get_queue(queueFamilyIndices_.graphicsFamily_.value());
}

VkQueue Device::get_present_queue() const {
    if (!queueFamilyIndices_.presentFamily_.has_value()) {
        throw std::runtime_error("Present queue family not available");
    }
    return get_queue(queueFamilyIndices_.presentFamily_.value());
}

VkQueue Device::get_compute_queue() const {
    if (!queueFamilyIndices_.computeFamily_.has_value()) {
        return get_graphics_queue();
    }
    return get_queue(queueFamilyIndices_.computeFamily_.value());
}

VkQueue Device::get_transfer_queue() const {
    if (!queueFamilyIndices_.transferFamily_.has_value()) {
        return get_graphics_queue();
    }
    return get_queue(queueFamilyIndices_.transferFamily_.value());
}

uint32_t Device::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    return physicalDevice_.find_memory_type(type_filter, properties);
}

VkResult Device::create_buffer(VkDeviceSize size,
                              VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags properties,
                              VkBuffer& buffer,
                              VkDeviceMemory& buffer_memory) const {
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(device_, &buffer_info, nullptr, &buffer);
    if (result != VK_SUCCESS) {
        return result;
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device_, buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

    result = vkAllocateMemory(device_, &alloc_info, nullptr, &buffer_memory);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(device_, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
        return result;
    }

    vkBindBufferMemory(device_, buffer, buffer_memory, 0);
    return VK_SUCCESS;
}

VkResult Device::create_image(uint32_t width,
                             uint32_t height,
                             VkFormat format,
                             VkImageTiling tiling,
                             VkImageUsageFlags usage,
                             VkMemoryPropertyFlags properties,
                             VkImage& image,
                             VkDeviceMemory& image_memory) const {
    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;

    VkResult result = vkCreateImage(device_, &image_info, nullptr, &image);
    if (result != VK_SUCCESS) {
        return result;
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device_, image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

    result = vkAllocateMemory(device_, &alloc_info, nullptr, &image_memory);
    if (result != VK_SUCCESS) {
        vkDestroyImage(device_, image, nullptr);
        image = VK_NULL_HANDLE;
        return result;
    }

    vkBindImageMemory(device_, image, image_memory, 0);
    return VK_SUCCESS;
}

VkCommandBuffer Device::begin_single_time_commands() const {
    // Allocate from the internal transient/resettable pool via RAII helper
    VkCommandBuffer command_buffer = singleUseCommandPool_->allocate_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void Device::end_single_time_commands(VkCommandBuffer command_buffer,
                                   VkQueue submit_queue) const {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(submit_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(submit_queue);

    singleUseCommandPool_->free_command_buffer(command_buffer);
}

VkFormat Device::find_supported_format(const std::vector<VkFormat>& candidates,
                                     VkImageTiling tiling,
                                     VkFormatFeatureFlags features) const {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice_.get_handle(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        }
        if (tiling == VK_IMAGE_TILING_OPTIMAL &&
            (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported format");
}

VkFormat Device::find_depth_format() const {
    return find_supported_format(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool Device::has_stencil_component(VkFormat format) const {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Device::create_logical_device(const std::vector<const char*>& required_extensions,
                                 const VkPhysicalDeviceFeatures& required_features,
                                 const std::vector<const char*>& validation_layers) {
    auto queue_create_infos = create_queue_create_infos(queueFamilyIndices_);
    VkPhysicalDeviceFeatures enabled_features = required_features;
    enabled_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &enabled_features;
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.empty() ? nullptr : required_extensions.data();
    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.empty() ? nullptr : validation_layers.data();

    if (vkCreateDevice(physicalDevice_.get_handle(), &create_info, nullptr, &device_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    volkLoadDevice(device_);
}

std::vector<VkDeviceQueueCreateInfo> Device::create_queue_create_infos(const QueueFamilyIndices& indices) const {
    std::set<uint32_t> unique_families = indices.get_unique_indices();
    float queue_priority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> infos;
    infos.reserve(unique_families.size());

    for (uint32_t family : unique_families) {
        VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue_info.queueFamilyIndex = family;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &queue_priority;
        infos.push_back(queue_info);
    }

    return infos;
}

} // namespace VulkanEngine::RAII

