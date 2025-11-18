#include "Image.hpp"

#include "VmaAllocator.hpp"
#include "../core/Device.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>


namespace VulkanEngine::RAII {

Image::Image(const VmaAllocator& allocator,
             uint32_t width,
             uint32_t height,
             uint32_t depth,
             uint32_t mip_levels,
             uint32_t array_layers,
             VkFormat format,
             VkImageType image_type,
             VkImageTiling tiling,
             VkImageUsageFlags usage,
             VkSampleCountFlagBits samples,
             VmaMemoryUsage memory_usage)
    : width_(width),
      height_(height),
      depth_(depth),
      mipLevels_(mip_levels),
      arrayLayers_(array_layers),
      format_(format),
      imageType_(image_type),
      tiling_(tiling),
      usage_(usage),
      samples_(samples),
      vmaAllocator_(allocator.get_handle()),
      device_(allocator.get_device()),
      deviceRef_(allocator.get_device_ref()),
      usingVMA_(true) {
    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = image_type;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = depth;
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = array_layers;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = samples;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = memory_usage;

    if (allocator.create_image(image_info, alloc_info, image_, allocation_, &allocationInfo_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image with VMA");
    }
}

Image::Image(const Device& device,
             uint32_t width,
             uint32_t height,
             uint32_t depth,
             uint32_t mip_levels,
             uint32_t array_layers,
             VkFormat format,
             VkImageType image_type,
             VkImageTiling tiling,
             VkImageUsageFlags usage,
             VkSampleCountFlagBits samples,
             VkMemoryPropertyFlags properties)
    : width_(width),
      height_(height),
      depth_(depth),
      mipLevels_(mip_levels),
      arrayLayers_(array_layers),
      format_(format),
      imageType_(image_type),
      tiling_(tiling),
      usage_(usage),
      samples_(samples),
      device_(device.get_handle()),
      deviceRef_(&device),
      memoryProperties_(properties) {
    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = image_type;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = depth;
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = array_layers;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = samples;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device_, &image_info, nullptr, &image_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device_, image_, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = device.find_memory_type(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &alloc_info, nullptr, &memory_) != VK_SUCCESS) {
        vkDestroyImage(device_, image_, nullptr);
        image_ = VK_NULL_HANDLE;
        throw std::runtime_error("Failed to allocate image memory");
    }

    vkBindImageMemory(device_, image_, memory_, 0);
}

Image::Image(VkImage image,
             VkFormat format,
             uint32_t width,
             uint32_t height,
             uint32_t depth,
             uint32_t mip_levels,
             uint32_t array_layers)
    : image_(image),
      width_(width),
      height_(height),
      depth_(depth),
      mipLevels_(mip_levels),
      arrayLayers_(array_layers),
      format_(format),
      ownsImage_(false) {}

Image::~Image() {
    cleanup();
}

Image::Image(Image&& other) noexcept
    : image_(other.image_),
      width_(other.width_),
      height_(other.height_),
      depth_(other.depth_),
      mipLevels_(other.mipLevels_),
      arrayLayers_(other.arrayLayers_),
      format_(other.format_),
      imageType_(other.imageType_),
      tiling_(other.tiling_),
      usage_(other.usage_),
      samples_(other.samples_),
      vmaAllocator_(other.vmaAllocator_),
      allocation_(other.allocation_),
      allocationInfo_(other.allocationInfo_),
      device_(other.device_),
      deviceRef_(other.deviceRef_),
      memory_(other.memory_),
      memoryProperties_(other.memoryProperties_),
      usingVMA_(other.usingVMA_),
      ownsImage_(other.ownsImage_) {
    other.image_ = VK_NULL_HANDLE;
    other.allocation_ = VK_NULL_HANDLE;
    other.memory_ = VK_NULL_HANDLE;
    other.ownsImage_ = false;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        cleanup();
        image_ = other.image_;
        width_ = other.width_;
        height_ = other.height_;
        depth_ = other.depth_;
        mipLevels_ = other.mipLevels_;
        arrayLayers_ = other.arrayLayers_;
        format_ = other.format_;
        imageType_ = other.imageType_;
        tiling_ = other.tiling_;
        usage_ = other.usage_;
        samples_ = other.samples_;
        vmaAllocator_ = other.vmaAllocator_;
        allocation_ = other.allocation_;
        allocationInfo_ = other.allocationInfo_;
        device_ = other.device_;
        deviceRef_ = other.deviceRef_;
        memory_ = other.memory_;
        memoryProperties_ = other.memoryProperties_;
        usingVMA_ = other.usingVMA_;
        ownsImage_ = other.ownsImage_;

        other.image_ = VK_NULL_HANDLE;
        other.allocation_ = VK_NULL_HANDLE;
        other.memory_ = VK_NULL_HANDLE;
        other.ownsImage_ = false;
    }
    return *this;
}

VkImageView Image::create_image_view(VkImageViewType view_type,
                                   VkImageAspectFlags aspect_flags,
                                   uint32_t base_mip_level,
                                   uint32_t level_count,
                                   uint32_t base_array_layer,
                                   uint32_t layer_count) const {
    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = image_;
    view_info.viewType = view_type;
    view_info.format = format_;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = base_mip_level;
    view_info.subresourceRange.levelCount = level_count == VK_REMAINING_MIP_LEVELS ? mipLevels_ - base_mip_level : level_count;
    view_info.subresourceRange.baseArrayLayer = base_array_layer;
    view_info.subresourceRange.layerCount = layer_count == VK_REMAINING_ARRAY_LAYERS ? arrayLayers_ - base_array_layer : layer_count;

    VkImageView image_view = VK_NULL_HANDLE;
    if (vkCreateImageView(device_, &view_info, nullptr, &image_view) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image view");
    }
    return image_view;
}

void Image::transition_layout(VkImageLayout old_layout,
                             VkImageLayout new_layout,
                             VkImageAspectFlags aspect_flags,
                             uint32_t base_mip_level,
                             uint32_t level_count,
                             uint32_t base_array_layer,
                             uint32_t layer_count,
                             VkCommandBuffer cmd) {
    if (cmd == VK_NULL_HANDLE) {
        throw std::runtime_error("Image::transition_layout called without a valid command buffer");
    }
    // Basic single-barrier implementation (no queue family ownership transfer)
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image_;
    barrier.subresourceRange.aspectMask = aspect_flags;
    barrier.subresourceRange.baseMipLevel = base_mip_level;
    barrier.subresourceRange.levelCount = (level_count == VK_REMAINING_MIP_LEVELS) ? (mipLevels_ - base_mip_level) : level_count;
    barrier.subresourceRange.baseArrayLayer = base_array_layer;
    barrier.subresourceRange.layerCount = (layer_count == VK_REMAINING_ARRAY_LAYERS) ? (arrayLayers_ - base_array_layer) : layer_count;

    auto access_for = [](VkImageLayout layout)->VkAccessFlags {
        switch(layout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return VK_ACCESS_TRANSFER_WRITE_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_ACCESS_TRANSFER_READ_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_SHADER_READ_BIT;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            default: return 0;
        }
    };

    auto stage_for = [](VkImageLayout layout, bool src)->VkPipelineStageFlags {
        switch(layout) {
            case VK_IMAGE_LAYOUT_UNDEFINED: return src ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_PIPELINE_STAGE_TRANSFER_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            default: return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
    };

    barrier.srcAccessMask = access_for(old_layout);
    barrier.dstAccessMask = access_for(new_layout);
    VkPipelineStageFlags src_stage = stage_for(old_layout, true);
    VkPipelineStageFlags dst_stage = stage_for(new_layout, false);

    vkCmdPipelineBarrier(cmd,
                         src_stage,
                         dst_stage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
}

void Image::copy_from_buffer(VkBuffer /*buffer*/, const std::vector<VkBufferImageCopy>& /*regions*/) {
    throw std::runtime_error("Image::copy_from_buffer requires command buffer parameter in future overload; not implemented here");
}

void Image::copy_to_buffer(VkBuffer /*buffer*/, const std::vector<VkBufferImageCopy>& /*regions*/) {
    throw std::runtime_error("Image::copyToBuffer requires explicit command buffer management and is not implemented in this RAII wrapper yet");
}

void Image::generate_mipmaps(VkFilter /*filter*/) {
    throw std::runtime_error("Image::generateMipmaps requires explicit command buffer management and is not implemented in this RAII wrapper yet");
}

VkMemoryRequirements Image::get_memory_requirements() const {
    VkMemoryRequirements requirements{};
    vkGetImageMemoryRequirements(device_, image_, &requirements);
    return requirements;
}

void Image::cleanup() {
    if (usingVMA_) {
        if (image_ != VK_NULL_HANDLE && vmaAllocator_ != VK_NULL_HANDLE) {
            vmaDestroyImage(vmaAllocator_, image_, allocation_);
        }
    } else if (ownsImage_) {
        if (image_ != VK_NULL_HANDLE) {
            vkDestroyImage(device_, image_, nullptr);
        }
        if (memory_ != VK_NULL_HANDLE) {
            vkFreeMemory(device_, memory_, nullptr);
        }
    }
    image_ = VK_NULL_HANDLE;
    memory_ = VK_NULL_HANDLE;
    allocation_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

