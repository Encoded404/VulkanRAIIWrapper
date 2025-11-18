#ifndef VULKAN_RAII_UTILS_IMAGE_UTILS_HPP
#define VULKAN_RAII_UTILS_IMAGE_UTILS_HPP

#include <volk.h>



namespace VulkanEngine::RAII::Utils {

// Image utilities
class ImageUtils {
public:
    // Calculate mip levels for given dimensions
    static uint32_t calculate_mip_levels(uint32_t width, uint32_t height, uint32_t depth = 1);
    
    // Get image aspect flags for format
    static VkImageAspectFlags get_image_aspect_flags(VkFormat format);
    
    // Create image subresource range
    static VkImageSubresourceRange create_subresource_range(VkImageAspectFlags aspect_flags,
                                                          uint32_t base_mip_level = 0,
                                                          uint32_t level_count = VK_REMAINING_MIP_LEVELS,
                                                          uint32_t base_array_layer = 0,
                                                          uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS);
    
    // Create buffer image copy region
    static VkBufferImageCopy create_buffer_image_copy(uint32_t width,
                                                   uint32_t height,
                                                   VkImageAspectFlags aspect_flags,
                                                   uint32_t mip_level = 0,
                                                   uint32_t array_layer = 0,
                                                   VkDeviceSize buffer_offset = 0);

    // Create image copy region
    static VkImageCopy create_image_copy(const VkImageSubresourceLayers& src_subresource,
                                      const VkOffset3D& src_offset,
                                      const VkImageSubresourceLayers& dst_subresource,
                                      const VkOffset3D& dst_offset,
                                      const VkExtent3D& extent);

    // Create image subresource layers
    static VkImageSubresourceLayers create_subresource_layers(VkImageAspectFlags aspect_flags,
                                                           uint32_t mip_level = 0,
                                                           uint32_t base_array_layer = 0,
                                                           uint32_t layer_count = 1);

    // Calculate image size in bytes
    static VkDeviceSize calculate_image_size(uint32_t width, uint32_t height, uint32_t depth,
                                          uint32_t mip_levels, uint32_t array_layers,
                                          VkFormat format);

    // Calculate mip level dimensions
    static void get_mip_level_dimensions(uint32_t base_mip_width, uint32_t base_mip_height, uint32_t base_mip_depth,
                                     uint32_t mip_level,
                                     uint32_t& mip_width, uint32_t& mip_height, uint32_t& mip_depth);

    // Check if image usage requires specific tiling
    static bool requires_optimal_tiling(VkImageUsageFlags usage);

    // Get compatible view type for image type
    static VkImageViewType get_compatible_view_type(VkImageType image_type, uint32_t array_layers);

    // Create image view create info
    static VkImageViewCreateInfo create_image_view_create_info(VkImage image,
                                                          VkImageViewType view_type,
                                                          VkFormat format,
                                                          VkImageAspectFlags aspect_flags,
                                                          uint32_t base_mip_level = 0,
                                                          uint32_t level_count = VK_REMAINING_MIP_LEVELS,
                                                          uint32_t base_array_layer = 0,
                                                          uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS);

    // Create image memory barrier
    static VkImageMemoryBarrier create_image_memory_barrier(VkImage image,
                                                         VkImageLayout old_layout,
                                                         VkImageLayout new_layout,
                                                         VkImageAspectFlags aspect_flags,
                                                         uint32_t base_mip_level = 0,
                                                         uint32_t level_count = VK_REMAINING_MIP_LEVELS,
                                                         uint32_t base_array_layer = 0,
                                                         uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS,
                                                         uint32_t src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
                                                         uint32_t dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED);

    // Get pipeline stage flags for image layout
    static VkPipelineStageFlags get_layout_pipeline_stage_flags(VkImageLayout layout, bool is_source);

    // Get access flags for image layout
    static VkAccessFlags get_layout_access_flags(VkImageLayout layout);

    // Record a layout transition barrier on a command buffer using sensible src/dst stages
    static void cmd_transition_image_layout(VkCommandBuffer cmd,
                                            VkImage image,
                                            VkImageLayout old_layout,
                                            VkImageLayout new_layout,
                                            VkImageAspectFlags aspect_flags,
                                            uint32_t base_mip_level = 0,
                                            uint32_t level_count = VK_REMAINING_MIP_LEVELS,
                                            uint32_t base_array_layer = 0,
                                            uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS);

    // Record a basic buffer-to-image copy for a single, tightly-packed region
    // - width/height specify the copy extent (depth assumed 1)
    // - aspect_flags typically VK_IMAGE_ASPECT_COLOR_BIT
    // - mip_level/array_layer select the destination subresource
    // - buffer_offset allows offsetting within the source buffer
    static void cmd_copy_buffer_to_image(VkCommandBuffer cmd,
                                         VkBuffer src_buffer,
                                         VkImage dst_image,
                                         uint32_t width,
                                         uint32_t height,
                                         VkImageLayout dst_layout,
                                         VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                                         uint32_t mip_level = 0,
                                         uint32_t array_layer = 0,
                                         VkDeviceSize buffer_offset = 0);
};

} // namespace VulkanEngine::RAII::Utils



#endif // VULKAN_RAII_UTILS_IMAGE_UTILS_HPP