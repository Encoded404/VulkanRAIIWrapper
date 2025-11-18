#ifndef VULKAN_RAII_UTILS_FORMAT_UTILS_HPP
#define VULKAN_RAII_UTILS_FORMAT_UTILS_HPP

#include <volk.h>
#include <vector>



namespace VulkanEngine::RAII::Utils {

// Format utilities
class FormatUtils {
public:
    // Check if format is depth format
    static bool is_depth_format(VkFormat format);
    
    // Check if format is stencil format
    static bool is_stencil_format(VkFormat format);
    
    // Check if format is depth-stencil format
    static bool is_depth_stencil_format(VkFormat format);

    // Check if format is color format
    static bool is_color_format(VkFormat format);
    
    // Get format size in bytes
    static uint32_t get_format_size(VkFormat format);
    
    // Get number of components in format
    static uint32_t get_format_component_count(VkFormat format);
    
    // Check if format is compressed
    static bool is_compressed_format(VkFormat format);

    // Check if format is integer format
    static bool is_integer_format(VkFormat format);

    // Check if format is normalized format
    static bool is_normalized_format(VkFormat format);

    // Check if format is signed format
    static bool is_signed_format(VkFormat format);

    // Check if format is sRGB format
    static bool is_srgb_format(VkFormat format);
    
    // Find supported format from candidates
    static VkFormat find_supported_format(VkPhysicalDevice physical_device,
                                       const std::vector<VkFormat>& candidates,
                                       VkImageTiling tiling,
                                       VkFormatFeatureFlags features);

    // Find depth format
    static VkFormat find_depth_format(VkPhysicalDevice physical_device);

    // Find depth stencil format
    static VkFormat find_depth_stencil_format(VkPhysicalDevice physical_device);

    // Get format features
    static VkFormatProperties get_format_properties(VkPhysicalDevice physical_device, VkFormat format);

    // Check format feature support
    static bool supports_format_feature(VkPhysicalDevice physical_device,
                                     VkFormat format,
                                     VkImageTiling tiling,
                                     VkFormatFeatureFlags feature);

    // Get bytes per pixel for uncompressed formats
    static uint32_t get_bytes_per_pixel(VkFormat format);

    // Get block size for compressed formats
    static void get_block_size(VkFormat format, uint32_t& block_width, uint32_t& block_height, uint32_t& block_size);
};

} // namespace VulkanEngine::RAII::Utils



#endif // VULKAN_RAII_UTILS_FORMAT_UTILS_HPP