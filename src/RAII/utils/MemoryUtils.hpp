#ifndef VULKAN_RAII_UTILS_MEMORY_UTILS_HPP
#define VULKAN_RAII_UTILS_MEMORY_UTILS_HPP

#include <volk.h>
#include <vector>



namespace VulkanEngine::RAII::Utils {

// Memory utilities
class MemoryUtils {
public:
    // Find suitable memory type for given requirements
    static uint32_t FindMemoryType(VkPhysicalDevice physical_device,
                                  uint32_t type_filter,
                                  VkMemoryPropertyFlags properties);

    // Get memory budget information (if available)
    static bool GetMemoryBudget(VkPhysicalDevice physical_device,
                               std::vector<VkDeviceSize>& heap_budgets,
                               std::vector<VkDeviceSize>& heap_usages);

    // Calculate aligned size
    static VkDeviceSize AlignedSize(VkDeviceSize size, VkDeviceSize alignment);

    // Get memory type properties
    static VkMemoryPropertyFlags GetMemoryTypeProperties(VkPhysicalDevice physical_device,
                                                         uint32_t memory_type_index);

    // Check if memory type is host visible
    static bool IsMemoryTypeHostVisible(VkPhysicalDevice physical_device, uint32_t memory_type_index);

    // Check if memory type is device local
    static bool IsMemoryTypeDeviceLocal(VkPhysicalDevice physical_device, uint32_t memory_type_index);

    // Check if memory type is host coherent
    static bool IsMemoryTypeHostCoherent(VkPhysicalDevice physical_device, uint32_t memory_type_index);

    // Get optimal memory type for buffer
    static uint32_t GetOptimalBufferMemoryType(VkPhysicalDevice physical_device,
                                              VkDevice device,
                                              VkBuffer buffer,
                                              VkMemoryPropertyFlags preferred_properties,
                                              VkMemoryPropertyFlags required_properties = 0);

    // Get optimal memory type for image
    static uint32_t GetOptimalImageMemoryType(VkPhysicalDevice physical_device,
                                             VkDevice device,
                                             VkImage image,
                                             VkMemoryPropertyFlags preferred_properties,
                                             VkMemoryPropertyFlags required_properties = 0);

    // Calculate buffer size with alignment
    static VkDeviceSize CalculateAlignedBufferSize(VkDeviceSize size, VkDeviceSize min_alignment);

    // Get non-coherent atom size
    static VkDeviceSize GetNonCoherentAtomSize(VkPhysicalDevice physical_device);
};

} // namespace VulkanEngine::RAII::Utils



#endif // VULKAN_RAII_UTILS_MEMORY_UTILS_HPP