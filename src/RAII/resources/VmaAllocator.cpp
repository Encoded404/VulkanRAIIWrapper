#include "VmaAllocator.hpp"

#include "../core/instance.hpp"
#include "../core/PhysicalDevice.hpp"
#include "../core/Device.hpp"
#include <volk.h>
#include <cstdint>
#include <stdexcept>


namespace VulkanEngine::RAII {

VmaAllocator::VmaAllocator(const Instance& instance,
                           const PhysicalDevice& physical_device,
                           const Device& device,
                           VmaAllocatorCreateFlags flags,
                           uint32_t vulkan_api_version)
        : instance_(instance.get_handle()),
            physicalDevice_(physical_device.get_handle()),
            device_(device.get_handle()),
            deviceRef_(&device) {
    create_allocator(flags, vulkan_api_version);
}

VmaAllocator::~VmaAllocator() {
    cleanup();
}

VmaAllocator::VmaAllocator(VmaAllocator&& other) noexcept
        : allocator_(other.allocator_),
            instance_(other.instance_),
            physicalDevice_(other.physicalDevice_),
            device_(other.device_),
            deviceRef_(other.deviceRef_) {
    other.allocator_ = VK_NULL_HANDLE;
    other.instance_ = VK_NULL_HANDLE;
    other.physicalDevice_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
        other.deviceRef_ = nullptr;
}

VmaAllocator& VmaAllocator::operator=(VmaAllocator&& other) noexcept {
    if (this != &other) {
        cleanup();
        allocator_ = other.allocator_;
        instance_ = other.instance_;
        physicalDevice_ = other.physicalDevice_;
        device_ = other.device_;
                deviceRef_ = other.deviceRef_;
        other.allocator_ = VK_NULL_HANDLE;
        other.instance_ = VK_NULL_HANDLE;
        other.physicalDevice_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
                other.deviceRef_ = nullptr;
    }
    return *this;
}

VkResult VmaAllocator::create_buffer(const VkBufferCreateInfo& buffer_create_info,
                                    const VmaAllocationCreateInfo& allocation_create_info,
                                    VkBuffer& buffer,
                                    VmaAllocation& allocation,
                                    VmaAllocationInfo* allocation_info) const {
    return vmaCreateBuffer(allocator_,
                           &buffer_create_info,
                           &allocation_create_info,
                           &buffer,
                           &allocation,
                           allocation_info);
}

void VmaAllocator::destroy_buffer(VkBuffer buffer, VmaAllocation allocation) const {
    vmaDestroyBuffer(allocator_, buffer, allocation);
}

VkResult VmaAllocator::create_image(const VkImageCreateInfo& image_create_info,
                                   const VmaAllocationCreateInfo& allocation_create_info,
                                   VkImage& image,
                                   VmaAllocation& allocation,
                                   VmaAllocationInfo* allocation_info) const {
    return vmaCreateImage(allocator_,
                          &image_create_info,
                          &allocation_create_info,
                          &image,
                          &allocation,
                          allocation_info);
}

void VmaAllocator::destroy_image(VkImage image, VmaAllocation allocation) const {
    vmaDestroyImage(allocator_, image, allocation);
}

VkResult VmaAllocator::map_memory(VmaAllocation allocation, void** data) const {
    return vmaMapMemory(allocator_, allocation, data);
}

void VmaAllocator::unmap_memory(VmaAllocation allocation) const {
    vmaUnmapMemory(allocator_, allocation);
}

VkResult VmaAllocator::flush_allocation(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const {
    return vmaFlushAllocation(allocator_, allocation, offset, size);
}

VkResult VmaAllocator::invalidate_allocation(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const {
    return vmaInvalidateAllocation(allocator_, allocation, offset, size);
}

void VmaAllocator::get_allocation_info(VmaAllocation allocation, VmaAllocationInfo& allocation_info) const {
    vmaGetAllocationInfo(allocator_, allocation, &allocation_info);
}

void VmaAllocator::calculate_statistics(::VmaTotalStatistics& stats) const {
    vmaCalculateStatistics(allocator_, &stats);
}

void VmaAllocator::get_heap_budgets(::VmaBudget* budgets) const {
    vmaGetHeapBudgets(allocator_, budgets);
}

VkResult VmaAllocator::find_memory_type_index_for_buffer_info(const VkBufferCreateInfo& buffer_create_info,
                                                        const VmaAllocationCreateInfo& allocation_create_info,
                                                        uint32_t& memory_type_index) const {
    return vmaFindMemoryTypeIndexForBufferInfo(allocator_,
                                               &buffer_create_info,
                                               &allocation_create_info,
                                               &memory_type_index);
}

VkResult VmaAllocator::find_memory_type_index_for_image_info(const VkImageCreateInfo& image_create_info,
                                                       const VmaAllocationCreateInfo& allocation_create_info,
                                                       uint32_t& memory_type_index) const {
    return vmaFindMemoryTypeIndexForImageInfo(allocator_,
                                              &image_create_info,
                                              &allocation_create_info,
                                              &memory_type_index);
}

VkResult VmaAllocator::create_pool(const ::VmaPoolCreateInfo& create_info, ::VmaPool& pool) const {
    return vmaCreatePool(allocator_, &create_info, &pool);
}

void VmaAllocator::destroy_pool(::VmaPool pool) const {
    vmaDestroyPool(allocator_, pool);
}

VkResult VmaAllocator::begin_defragmentation(const ::VmaDefragmentationInfo& info,
                                            ::VmaDefragmentationContext& context) const {
    return vmaBeginDefragmentation(allocator_, &info, &context);
}

VkResult VmaAllocator::end_defragmentation(::VmaDefragmentationContext context,
                                          ::VmaDefragmentationStats* stats) const {
    vmaEndDefragmentation(allocator_, context, stats);
    return VK_SUCCESS;
}

void VmaAllocator::create_allocator(VmaAllocatorCreateFlags flags, uint32_t vulkan_api_version) {
    VmaAllocatorCreateInfo create_info{};
    create_info.flags = flags;
    create_info.physicalDevice = physicalDevice_;
    create_info.device = device_;
    create_info.instance = instance_;
    create_info.vulkanApiVersion = vulkan_api_version;

    if (!vkGetInstanceProcAddr || !vkGetDeviceProcAddr) {
        throw std::runtime_error("Volk global function pointers are not initialised");
    }

    // VMA compiled with VMA_DYNAMIC_VULKAN_FUNCTIONS requires these loader callbacks.
    VmaVulkanFunctions vulkan_functions{};
    vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    create_info.pVulkanFunctions = &vulkan_functions;

    if (vmaCreateAllocator(&create_info, &allocator_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create VMA allocator");
    }
}

void VmaAllocator::cleanup() {
    if (allocator_ != VK_NULL_HANDLE) {
        vmaDestroyAllocator(allocator_);
        allocator_ = VK_NULL_HANDLE;
    }
    deviceRef_ = nullptr;
}

} // namespace VulkanEngine::RAII

