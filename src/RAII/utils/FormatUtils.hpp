#ifndef VULKAN_RAII_UTILS_FORMAT_UTILS_HPP
#define VULKAN_RAII_UTILS_FORMAT_UTILS_HPP

#include <volk.h>
#include <vector>



namespace VulkanEngine::RAII::Utils {

// Format utilities
class FormatUtils {
public:
    // Check if format is depth format
    static bool IsDepthFormat(VkFormat format);
    
    // Check if format is stencil format
    static bool IsStencilFormat(VkFormat format);
    
    // Check if format is depth-stencil format
    static bool IsDepthStencilFormat(VkFormat format);

    // Check if format is color format
    static bool IsColorFormat(VkFormat format);
    
    // Get format size in bytes
    static uint32_t GetFormatSize(VkFormat format);
    
    // Get number of components in format
    static uint32_t GetFormatComponentCount(VkFormat format);
    
    // Check if format is compressed
    static bool IsCompressedFormat(VkFormat format);

    // Check if format is integer format
    static bool IsIntegerFormat(VkFormat format);

    // Check if format is normalized format
    static bool IsNormalizedFormat(VkFormat format);

    // Check if format is signed format
    static bool IsSignedFormat(VkFormat format);

    // Check if format is sRGB format
    static bool IsSrgbFormat(VkFormat format);
    
    // Find supported format from candidates
    static VkFormat FindSupportedFormat(VkPhysicalDevice physical_device,
                                       const std::vector<VkFormat>& candidates,
                                       VkImageTiling tiling,
                                       VkFormatFeatureFlags features);

    // Find depth format
    static VkFormat FindDepthFormat(VkPhysicalDevice physical_device);

    // Find depth stencil format
    static VkFormat FindDepthStencilFormat(VkPhysicalDevice physical_device);

    // Get format features
    static VkFormatProperties GetFormatProperties(VkPhysicalDevice physical_device, VkFormat format);

    // Check format feature support
    static bool SupportsFormatFeature(VkPhysicalDevice physical_device,
                                     VkFormat format,
                                     VkImageTiling tiling,
                                     VkFormatFeatureFlags feature);

    // Get bytes per pixel for uncompressed formats
    static uint32_t GetBytesPerPixel(VkFormat format);

    // Get block size for compressed formats
    static void GetBlockSize(VkFormat format, uint32_t& block_width, uint32_t& block_height, uint32_t& block_size);
};

} // namespace VulkanEngine::RAII::Utils



#endif // VULKAN_RAII_UTILS_FORMAT_UTILS_HPP