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

    [[nodiscard]] ::VmaAllocator GetHandle() const { return allocator_; }
    
    // Implicit conversion to VmaAllocator
    operator ::VmaAllocator() const { return allocator_; }

    [[nodiscard]] VkDevice GetDevice() const { return device_; }
    [[nodiscard]] const Device* GetDeviceRef() const { return deviceRef_; }

    // Check if the allocator is valid
    [[nodiscard]] bool IsValid() const { return allocator_ != VK_NULL_HANDLE; }

    // Buffer allocation
    VkResult CreateBuffer(const VkBufferCreateInfo& buffer_create_info,
                         const VmaAllocationCreateInfo& allocation_create_info,
                         VkBuffer& buffer,
                         VmaAllocation& allocation,
                         VmaAllocationInfo* allocation_info = nullptr) const;

    void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) const;

    // Image allocation
    VkResult CreateImage(const VkImageCreateInfo& image_create_info,
                        const VmaAllocationCreateInfo& allocation_create_info,
                        VkImage& image,
                        VmaAllocation& allocation,
                        VmaAllocationInfo* allocation_info = nullptr) const;

    void DestroyImage(VkImage image, VmaAllocation allocation) const;

    // Memory mapping
    VkResult MapMemory(VmaAllocation allocation, void** data) const;
    void UnmapMemory(VmaAllocation allocation) const;

    // Memory flushing and invalidation
    VkResult FlushAllocation(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const;
    VkResult InvalidateAllocation(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const;

    // Allocation info
    void GetAllocationInfo(VmaAllocation allocation, VmaAllocationInfo& allocation_info) const;

    // Statistics
    void CalculateStatistics(::VmaTotalStatistics& stats) const;

    // Budget information
    void GetHeapBudgets(::VmaBudget* budgets) const;

    // Memory type utilities
    VkResult FindMemoryTypeIndexForBufferInfo(const VkBufferCreateInfo& buffer_create_info,
                                             const VmaAllocationCreateInfo& allocation_create_info,
                                             uint32_t& memory_type_index) const;

    VkResult FindMemoryTypeIndexForImageInfo(const VkImageCreateInfo& image_create_info,
                                            const VmaAllocationCreateInfo& allocation_create_info,
                                            uint32_t& memory_type_index) const;

    // Pool management
    VkResult CreatePool(const ::VmaPoolCreateInfo& create_info, ::VmaPool& pool) const;
    void DestroyPool(::VmaPool pool) const;

    // Defragmentation (signatures aligned with current VMA API)
    VkResult BeginDefragmentation(const ::VmaDefragmentationInfo& info,
                                 ::VmaDefragmentationContext& context) const;

    VkResult EndDefragmentation(::VmaDefragmentationContext context,
                               ::VmaDefragmentationStats* stats) const;

private:
    ::VmaAllocator allocator_{VK_NULL_HANDLE};
    VkInstance instance_{VK_NULL_HANDLE}; // Reference for cleanup order
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE}; // Reference for cleanup order
    VkDevice device_{VK_NULL_HANDLE}; // Reference for cleanup order
    const Device* deviceRef_{nullptr};

    // Helper methods
    void CreateAllocator(VmaAllocatorCreateFlags flags, uint32_t vulkan_api_version);
    void Cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_VMA_ALLOCATOR_HPP