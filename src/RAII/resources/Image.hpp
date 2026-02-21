#ifndef VULKAN_RAII_RESOURCES_IMAGE_HPP
#define VULKAN_RAII_RESOURCES_IMAGE_HPP

#include <volk.h>
#include <vector>

namespace VulkanEngine::RAII {

class Device;

class Image {
public:
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

    Image(VkImage image,
          VkFormat format,
          uint32_t width,
          uint32_t height,
          uint32_t depth = 1,
          uint32_t mip_levels = 1,
          uint32_t array_layers = 1);

    ~Image();

    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    [[nodiscard]] VkImage GetHandle() const { return image_; }
    operator VkImage() const { return image_; }
    [[nodiscard]] bool IsValid() const { return image_ != VK_NULL_HANDLE; }

    [[nodiscard]] uint32_t GetWidth() const { return width_; }
    [[nodiscard]] uint32_t GetHeight() const { return height_; }
    [[nodiscard]] uint32_t GetDepth() const { return depth_; }
    [[nodiscard]] uint32_t GetMipLevels() const { return mipLevels_; }
    [[nodiscard]] uint32_t GetArrayLayers() const { return arrayLayers_; }
    [[nodiscard]] VkFormat GetFormat() const { return format_; }
    [[nodiscard]] VkImageType GetImageType() const { return imageType_; }
    [[nodiscard]] VkImageUsageFlags GetUsage() const { return usage_; }
    [[nodiscard]] VkSampleCountFlagBits GetSamples() const { return samples_; }

    [[nodiscard]] VkImageView CreateImageView(VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D,
                                               VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                                               uint32_t base_mip_level = 0,
                                               uint32_t level_count = VK_REMAINING_MIP_LEVELS,
                                               uint32_t base_array_layer = 0,
                                               uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS) const;

    void TransitionLayout(VkImageLayout old_layout,
                          VkImageLayout new_layout,
                          VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                          uint32_t base_mip_level = 0,
                          uint32_t level_count = VK_REMAINING_MIP_LEVELS,
                          uint32_t base_array_layer = 0,
                          uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS,
                          VkCommandBuffer cmd = VK_NULL_HANDLE);

    void CopyFromBuffer(VkBuffer buffer,
                        const std::vector<VkBufferImageCopy>& regions);

    void CopyToBuffer(VkBuffer buffer,
                      const std::vector<VkBufferImageCopy>& regions);

    void GenerateMipmaps(VkFilter filter = VK_FILTER_LINEAR);

    [[nodiscard]] VkMemoryRequirements GetMemoryRequirements() const;
    [[nodiscard]] VkDeviceMemory GetMemory() const { return memory_; }

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

    VkDevice device_{VK_NULL_HANDLE};
    VkDeviceMemory memory_{VK_NULL_HANDLE};

    bool ownsImage_{true};

    void Cleanup();
};

} // namespace VulkanEngine::RAII

#endif // VULKAN_RAII_RESOURCES_IMAGE_HPP
