#ifndef VULKAN_RAII_RESOURCES_VMA_ALLOCATOR_HPP
#define VULKAN_RAII_RESOURCES_VMA_ALLOCATOR_HPP

#include <volk.h>
#include <vk_mem_alloc.h>


namespace VulkanEngine::RAII {

class Instance; // Forward declaration
class PhysicalDevice; // Forward declaration
class Device; // Forward declaration

class VmaAllocator {
public:
    // Constructor that creates a VMA allocator
    VmaAllocator(const Instance& instance,
                 const PhysicalDevice& physical_device,
                 const Device& device,
                 VmaAllocatorCreateFlags flags = 0,
                 uint32_t vulkan_api_version = VK_API_VERSION_1_0);

    // Destructor
    ~VmaAllocator();

    // Move constructor and assignment
    VmaAllocator(VmaAllocator&& other) noexcept;
    VmaAllocator& operator=(VmaAllocator&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VmaAllocator by only allowing moving.
    VmaAllocator(const VmaAllocator&) = delete;
    VmaAllocator& operator=(const VmaAllocator&) = delete;

    [[nodiscard]] ::VmaAllocator get_handle() const { return allocator_; }
    
    // Implicit conversion to VmaAllocator
    operator ::VmaAllocator() const { return allocator_; }

    [[nodiscard]] VkDevice get_device() const { return device_; }
    [[nodiscard]] const Device* get_device_ref() const { return deviceRef_; }

    // Check if the allocator is valid
    [[nodiscard]] bool is_valid() const { return allocator_ != VK_NULL_HANDLE; }

    // Buffer allocation
    VkResult create_buffer(const VkBufferCreateInfo& buffer_create_info,
                         const VmaAllocationCreateInfo& allocation_create_info,
                         VkBuffer& buffer,
                         VmaAllocation& allocation,
                         VmaAllocationInfo* allocation_info = nullptr) const;

    void destroy_buffer(VkBuffer buffer, VmaAllocation allocation) const;

    // Image allocation
    VkResult create_image(const VkImageCreateInfo& image_create_info,
                        const VmaAllocationCreateInfo& allocation_create_info,
                        VkImage& image,
                        VmaAllocation& allocation,
                        VmaAllocationInfo* allocation_info = nullptr) const;

    void destroy_image(VkImage image, VmaAllocation allocation) const;

    // Memory mapping
    VkResult map_memory(VmaAllocation allocation, void** data) const;
    void unmap_memory(VmaAllocation allocation) const;

    // Memory flushing and invalidation
    VkResult flush_allocation(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const;
    VkResult invalidate_allocation(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const;

    // Allocation info
    void get_allocation_info(VmaAllocation allocation, VmaAllocationInfo& allocation_info) const;

    // Statistics
    void calculate_statistics(::VmaTotalStatistics& stats) const;

    // Budget information
    void get_heap_budgets(::VmaBudget* budgets) const;

    // Memory type utilities
    VkResult find_memory_type_index_for_buffer_info(const VkBufferCreateInfo& buffer_create_info,
                                             const VmaAllocationCreateInfo& allocation_create_info,
                                             uint32_t& memory_type_index) const;

    VkResult find_memory_type_index_for_image_info(const VkImageCreateInfo& image_create_info,
                                            const VmaAllocationCreateInfo& allocation_create_info,
                                            uint32_t& memory_type_index) const;

    // Pool management
    VkResult create_pool(const ::VmaPoolCreateInfo& create_info, ::VmaPool& pool) const;
    void destroy_pool(::VmaPool pool) const;

    // Defragmentation (signatures aligned with current VMA API)
    VkResult begin_defragmentation(const ::VmaDefragmentationInfo& info,
                                 ::VmaDefragmentationContext& context) const;

    VkResult end_defragmentation(::VmaDefragmentationContext context,
                               ::VmaDefragmentationStats* stats) const;

private:
    ::VmaAllocator allocator_{VK_NULL_HANDLE};
    VkInstance instance_{VK_NULL_HANDLE}; // Reference for cleanup order
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE}; // Reference for cleanup order
    VkDevice device_{VK_NULL_HANDLE}; // Reference for cleanup order
    const Device* deviceRef_{nullptr};

    // Helper methods
    void create_allocator(VmaAllocatorCreateFlags flags, uint32_t vulkan_api_version);
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_VMA_ALLOCATOR_HPP