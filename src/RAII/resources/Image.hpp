#ifndef VULKAN_RAII_RESOURCES_IMAGE_HPP
#define VULKAN_RAII_RESOURCES_IMAGE_HPP

#include <volk.h>
#include <vk_mem_alloc.h>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration
class VmaAllocator; // Forward declaration

class Image {
public:
    // Constructor that creates an image with VMA
    Image(const VmaAllocator& allocator,
          uint32_t width,
          uint32_t height,
          uint32_t depth = 1,
          uint32_t mip_levels = 1,
          uint32_t array_layers = 1,
          VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
          VkImageType image_type = VK_IMAGE_TYPE_2D,
          VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
          VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
          VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
          VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO);

    // Constructor that creates an image with traditional Vulkan memory management
    Image(const Device& device,
          uint32_t width,
          uint32_t height,
          uint32_t depth = 1,
          uint32_t mip_levels = 1,
          uint32_t array_layers = 1,
          VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
          VkImageType image_type = VK_IMAGE_TYPE_2D,
          VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
          VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
          VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
          VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Constructor that wraps an existing VkImage (doesn't own memory)
    Image(VkImage image,
          VkFormat format,
          uint32_t width,
          uint32_t height,
          uint32_t depth = 1,
          uint32_t mip_levels = 1,
          uint32_t array_layers = 1);

    // Destructor
    ~Image();

    // Move constructor and assignment
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkImage by only allowing moving.
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    [[nodiscard]] VkImage get_handle() const { return image_; }
    
    // Implicit conversion to VkImage
    operator VkImage() const { return image_; }

    // Check if the image is valid
    [[nodiscard]] bool is_valid() const { return image_ != VK_NULL_HANDLE; }

    // Get image properties
    [[nodiscard]] uint32_t get_width() const { return width_; }
    [[nodiscard]] uint32_t get_height() const { return height_; }
    [[nodiscard]] uint32_t get_depth() const { return depth_; }
    [[nodiscard]] uint32_t get_mip_levels() const { return mipLevels_; }
    [[nodiscard]] uint32_t get_array_layers() const { return arrayLayers_; }
    [[nodiscard]] VkFormat get_format() const { return format_; }
    [[nodiscard]] VkImageType get_image_type() const { return imageType_; }
    [[nodiscard]] VkImageUsageFlags get_usage() const { return usage_; }
    [[nodiscard]] VkSampleCountFlagBits get_samples() const { return samples_; }

    // Create image view
    [[nodiscard]] VkImageView create_image_view(VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D,
                               VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                               uint32_t base_mip_level = 0,
                               uint32_t level_count = VK_REMAINING_MIP_LEVELS,
                               uint32_t base_array_layer = 0,
                               uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS) const;

    // Transition image layout
    void transition_layout(VkImageLayout old_layout,
                         VkImageLayout new_layout,
                         VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                         uint32_t base_mip_level = 0,
                         uint32_t level_count = VK_REMAINING_MIP_LEVELS,
                         uint32_t base_array_layer = 0,
                         uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS,
                         VkCommandBuffer cmd = VK_NULL_HANDLE);

    // Copy from buffer
    void copy_from_buffer(VkBuffer buffer,
                       const std::vector<VkBufferImageCopy>& regions);

    // Copy to buffer
    void copy_to_buffer(VkBuffer buffer,
                     const std::vector<VkBufferImageCopy>& regions);

    // Generate mipmaps
    void generate_mipmaps(VkFilter filter = VK_FILTER_LINEAR);

    // Get memory requirements
    [[nodiscard]] VkMemoryRequirements get_memory_requirements() const;

    // Get device memory (for traditional Vulkan)
    [[nodiscard]] VkDeviceMemory get_memory() const { return memory_; }

private:
    VkImage image_{VK_NULL_HANDLE};
    uint32_t width_{0};
    uint32_t height_{0};
    uint32_t depth_{1};
    uint32_t mipLevels_{1};
    uint32_t arrayLayers_{1};
    VkFormat format_{VK_FORMAT_UNDEFINED};
    VkImageType imageType_{VK_IMAGE_TYPE_2D};
    VkImageTiling tiling_{VK_IMAGE_TILING_OPTIMAL};
    VkImageUsageFlags usage_{0};
    VkSampleCountFlagBits samples_{VK_SAMPLE_COUNT_1_BIT};

    // VMA allocation
      ::VmaAllocator vmaAllocator_{VK_NULL_HANDLE};
    VmaAllocation allocation_{VK_NULL_HANDLE};
    VmaAllocationInfo allocationInfo_{};

    // Traditional Vulkan memory management
    VkDevice device_{VK_NULL_HANDLE};
            const Device* deviceRef_{nullptr};
    VkDeviceMemory memory_{VK_NULL_HANDLE};
    VkMemoryPropertyFlags memoryProperties_{0};

    bool usingVMA_{false};
    bool ownsImage_{true}; // Whether we created the image or just wrap it

    // Helper methods
    void create_image();
    void allocate_memory(VkMemoryPropertyFlags properties);
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_IMAGE_HPP