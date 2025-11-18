#include "Buffer.hpp"

#include "VmaAllocator.hpp"
#include "../core/Device.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>


namespace VulkanEngine::RAII {

namespace {
void set_debug_name_internal(VkDevice device, VkBuffer buffer, const char* name) {
    if (!device || !buffer || !name) {
        return;
    }
    if (vkSetDebugUtilsObjectNameEXT) {
        VkDebugUtilsObjectNameInfoEXT info{VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
        info.objectType = VK_OBJECT_TYPE_BUFFER;
        info.objectHandle = reinterpret_cast<uint64_t>(buffer);
        info.pObjectName = name;
        vkSetDebugUtilsObjectNameEXT(device, &info);
    }
}
} // namespace

Buffer::Buffer(const VmaAllocator& allocator,
               VkDeviceSize size,
               VkBufferUsageFlags usage,
               VmaMemoryUsage memory_usage,
               VmaAllocationCreateFlags flags,
               const char* name)
    : size_(size),
      usage_(usage),
      vmaAllocator_(allocator.get_handle()),
      device_(allocator.get_device()),
      usingVMA_(true) {
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = memory_usage;
    alloc_info.flags = flags;

    if (allocator.create_buffer(buffer_info, alloc_info, buffer_, allocation_, &allocationInfo_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create VMA buffer");
    }

    if (name) {
        set_debug_name(name);
    }
}

Buffer::Buffer(const Device& device,
               VkDeviceSize size,
               VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties,
               const char* name)
    : size_(size),
      usage_(usage),
      device_(device.get_handle()),
      memoryProperties_(properties) {
    if (device.create_buffer(size, usage, properties, buffer_, memory_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create device buffer");
    }

    if (name) {
        set_debug_name(name);
    }
}

Buffer::~Buffer() {
    cleanup();
}

Buffer::Buffer(Buffer&& other) noexcept
    : buffer_(other.buffer_),
      size_(other.size_),
      usage_(other.usage_),
      vmaAllocator_(other.vmaAllocator_),
      allocation_(other.allocation_),
      allocationInfo_(other.allocationInfo_),
      device_(other.device_),
      memory_(other.memory_),
      memoryProperties_(other.memoryProperties_),
      usingVMA_(other.usingVMA_),
      mappedData_(other.mappedData_),
      debugName_(std::move(other.debugName_)) {
    other.buffer_ = VK_NULL_HANDLE;
    other.allocation_ = VK_NULL_HANDLE;
    other.memory_ = VK_NULL_HANDLE;
    other.mappedData_ = nullptr;
    other.usingVMA_ = false;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        cleanup();
        buffer_ = other.buffer_;
        size_ = other.size_;
        usage_ = other.usage_;
        vmaAllocator_ = other.vmaAllocator_;
        allocation_ = other.allocation_;
        allocationInfo_ = other.allocationInfo_;
        device_ = other.device_;
        memory_ = other.memory_;
        memoryProperties_ = other.memoryProperties_;
        usingVMA_ = other.usingVMA_;
        mappedData_ = other.mappedData_;
        debugName_ = std::move(other.debugName_);

        other.buffer_ = VK_NULL_HANDLE;
        other.allocation_ = VK_NULL_HANDLE;
        other.memory_ = VK_NULL_HANDLE;
        other.mappedData_ = nullptr;
        other.usingVMA_ = false;
    }
    return *this;
}

void* Buffer::map() {
    if (!usingVMA_) {
        throw std::runtime_error("Buffer was not created with VMA; use mapMemory instead");
    }
    if (!mappedData_) {
        if (vmaMapMemory(vmaAllocator_, allocation_, &mappedData_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to map buffer");
        }
    }
    return mappedData_;
}

void Buffer::unmap() {
    if (usingVMA_ && mappedData_) {
        vmaUnmapMemory(vmaAllocator_, allocation_);
        mappedData_ = nullptr;
    }
}

void* Buffer::map_memory() {
    if (usingVMA_) {
        return map();
    }
    if (!mappedData_) {
        if (vkMapMemory(device_, memory_, 0, size_, 0, &mappedData_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to map buffer memory");
        }
    }
    return mappedData_;
}

void Buffer::unmap_memory() {
    if (usingVMA_) {
        unmap();
        return;
    }
    if (mappedData_) {
        vkUnmapMemory(device_, memory_);
        mappedData_ = nullptr;
    }
}

void Buffer::write_data(const void* data, VkDeviceSize size, VkDeviceSize offset) {
    if (!data) {
        throw std::invalid_argument("Data pointer must not be null");
    }
    if (size == VK_WHOLE_SIZE) {
        size = size_;
    }
    uint8_t* destination = reinterpret_cast<uint8_t*>(map_memory());
    std::memcpy(destination + offset, data, static_cast<size_t>(size));
    if (!usingVMA_ && !(memoryProperties_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        flush(size, offset);
    }
    if (!usingVMA_) {
        unmap_memory();
    }
}

void Buffer::read_data(void* data, VkDeviceSize size, VkDeviceSize offset) {
    if (!data) {
        throw std::invalid_argument("Data pointer must not be null");
    }
    if (size == VK_WHOLE_SIZE) {
        size = size_;
    }
    uint8_t* source = reinterpret_cast<uint8_t*>(map_memory());
    std::memcpy(data, source + offset, static_cast<size_t>(size));
    if (!usingVMA_) {
        unmap_memory();
    }
}

VkMemoryRequirements Buffer::get_memory_requirements() const {
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(device_, buffer_, &requirements);
    return requirements;
}

void Buffer::bind_memory(VkDeviceMemory memory, VkDeviceSize offset) {
    if (usingVMA_) {
        throw std::runtime_error("Cannot manually bind memory for VMA-managed buffer");
    }
    vkBindBufferMemory(device_, buffer_, memory, offset);
    memory_ = memory;
}

void Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    if (usingVMA_) {
        vmaFlushAllocation(vmaAllocator_, allocation_, offset, size);
        return;
    }
    VkMappedMemoryRange range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
    range.memory = memory_;
    range.offset = offset;
    range.size = size;
    vkFlushMappedMemoryRanges(device_, 1, &range);
}

void Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    if (usingVMA_) {
        vmaInvalidateAllocation(vmaAllocator_, allocation_, offset, size);
        return;
    }
    VkMappedMemoryRange range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
    range.memory = memory_;
    range.offset = offset;
    range.size = size;
    vkInvalidateMappedMemoryRanges(device_, 1, &range);
}

void Buffer::copy_from(const Buffer& /*srcBuffer*/, VkDeviceSize /*size*/, VkDeviceSize /*srcOffset*/, VkDeviceSize /*dstOffset*/) {
    throw std::runtime_error("copyFrom without command buffer is not implemented; use the command buffer overload");
}

void Buffer::copy_from(VkCommandBuffer command_buffer,
                      const Buffer& src_buffer,
                      VkDeviceSize size,
                      VkDeviceSize src_offset,
                      VkDeviceSize dst_offset) {
    VkBufferCopy copy_region{};
    copy_region.srcOffset = src_offset;
    copy_region.dstOffset = dst_offset;
    copy_region.size = (size == VK_WHOLE_SIZE) ? size_ : size;
    vkCmdCopyBuffer(command_buffer, src_buffer.get_handle(), buffer_, 1, &copy_region);
}

void Buffer::set_debug_name(const char* name) {
    if (!name) {
        return;
    }
    debugName_ = name;
    set_debug_name_internal(device_, buffer_, name);
}

Buffer Buffer::create_staging(const VmaAllocator& allocator, VkDeviceSize size) {
    return Buffer(allocator,
                  size,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VMA_MEMORY_USAGE_CPU_ONLY,
                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
}

Buffer Buffer::create_staging(const Device& device, VkDeviceSize size) {
    return Buffer(device,
                  size,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

Buffer Buffer::create_vertex_buffer(const VmaAllocator& allocator, VkDeviceSize size) {
    return Buffer(allocator,
                  size,
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VMA_MEMORY_USAGE_GPU_ONLY);
}

Buffer Buffer::create_vertex_buffer(const Device& device, VkDeviceSize size) {
    return Buffer(device,
                  size,
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

Buffer Buffer::create_index_buffer(const VmaAllocator& allocator, VkDeviceSize size) {
    return Buffer(allocator,
                  size,
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VMA_MEMORY_USAGE_GPU_ONLY);
}

Buffer Buffer::create_index_buffer(const Device& device, VkDeviceSize size) {
    return Buffer(device,
                  size,
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

Buffer Buffer::create_uniform_buffer(const VmaAllocator& allocator, VkDeviceSize size) {
    return Buffer(allocator,
                  size,
                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                  VMA_MEMORY_USAGE_CPU_TO_GPU,
                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
}

Buffer Buffer::create_uniform_buffer(const Device& device, VkDeviceSize size) {
    return Buffer(device,
                  size,
                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

Buffer Buffer::create_storage_buffer(const VmaAllocator& allocator, VkDeviceSize size) {
    return Buffer(allocator,
                  size,
                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VMA_MEMORY_USAGE_GPU_ONLY);
}

Buffer Buffer::create_storage_buffer(const Device& device, VkDeviceSize size) {
    return Buffer(device,
                  size,
                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void Buffer::cleanup() {
    if (mappedData_) {
        unmap_memory();
    }

    if (usingVMA_) {
        if (buffer_ != VK_NULL_HANDLE && vmaAllocator_ != VK_NULL_HANDLE) {
            vmaDestroyBuffer(vmaAllocator_, buffer_, allocation_);
        }
    } else {
        if (buffer_ != VK_NULL_HANDLE) {
            vkDestroyBuffer(device_, buffer_, nullptr);
        }
        if (memory_ != VK_NULL_HANDLE) {
            vkFreeMemory(device_, memory_, nullptr);
        }
    }
    buffer_ = VK_NULL_HANDLE;
    memory_ = VK_NULL_HANDLE;
    allocation_ = VK_NULL_HANDLE;
    device_ = VK_NULL_HANDLE;
    mappedData_ = nullptr;
}

} // namespace VulkanEngine::RAII