#ifndef VULKAN_RAII_RESOURCES_BUFFER_HPP
#define VULKAN_RAII_RESOURCES_BUFFER_HPP

#include <volk.h>
#include <vk_mem_alloc.h>
#include <string>

namespace VulkanEngine::RAII {

class Device; // Forward declaration
class VmaAllocator; // Forward declaration

class Buffer {
public:
    // Constructor that creates a buffer with VMA
    Buffer(const VmaAllocator& allocator,
           VkDeviceSize size,
           VkBufferUsageFlags usage,
           VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO,
           VmaAllocationCreateFlags flags = 0,
           const char* name = nullptr);

    // Constructor that creates a buffer with traditional Vulkan memory management
    Buffer(const Device& device,
           VkDeviceSize size,
           VkBufferUsageFlags usage,
           VkMemoryPropertyFlags properties,
           const char* name = nullptr);

    // Destructor
    ~Buffer();

    // Move constructor and assignment
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkBuffer by only allowing moving.
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    [[nodiscard]] VkBuffer GetHandle() const { return buffer_; }
    
    // Implicit conversion to VkBuffer
    operator VkBuffer() const { return buffer_; }

    // Check if the buffer is valid
    [[nodiscard]] bool IsValid() const { return buffer_ != VK_NULL_HANDLE; }

    // Get buffer size
    [[nodiscard]] VkDeviceSize GetSize() const { return size_; }

    // Get buffer usage flags
    [[nodiscard]] VkBufferUsageFlags GetUsage() const { return usage_; }

    // Memory mapping (for VMA)
    void* Map();
    void Unmap();

    // Memory mapping (for traditional Vulkan)
    void* MapMemory();
    void UnmapMemory();

    // Copy data to buffer (assumes buffer is mappable)
    void WriteData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

    // Read data from buffer (assumes buffer is mappable)
    void ReadData(void* data, VkDeviceSize size, VkDeviceSize offset = 0);

    // Get memory requirements
    [[nodiscard]] VkMemoryRequirements GetMemoryRequirements() const;

    // Bind buffer memory (for traditional Vulkan)
    void BindMemory(VkDeviceMemory memory, VkDeviceSize offset = 0);

    // Get device memory (for traditional Vulkan)
    [[nodiscard]] VkDeviceMemory GetMemory() const { return memory_; }

    // Flush memory (for non-coherent memory)
    void Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    // Invalidate memory (for non-coherent memory)
    void Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    // Copy from another buffer
    void CopyFrom(const Buffer& src_buffer, VkDeviceSize size = VK_WHOLE_SIZE, 
                  VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0);

    // Copy from another buffer using command buffer
    void CopyFrom(VkCommandBuffer command_buffer, const Buffer& src_buffer, 
                  VkDeviceSize size = VK_WHOLE_SIZE, 
                  VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0);

    // Set debug name (for debugging)
    void SetDebugName(const char* name);

    // Get debug name
    [[nodiscard]] const char* GetDebugName() const { return debugName_.c_str(); }

    // Helper to create staging buffer
    static Buffer CreateStaging(const VmaAllocator& allocator, VkDeviceSize size);
    static Buffer CreateStaging(const Device& device, VkDeviceSize size);

    // Helper to create vertex buffer
    static Buffer CreateVertexBuffer(const VmaAllocator& allocator, VkDeviceSize size);
    static Buffer CreateVertexBuffer(const Device& device, VkDeviceSize size);

    // Helper to create index buffer
    static Buffer CreateIndexBuffer(const VmaAllocator& allocator, VkDeviceSize size);
    static Buffer CreateIndexBuffer(const Device& device, VkDeviceSize size);

    // Helper to create uniform buffer
    static Buffer CreateUniformBuffer(const VmaAllocator& allocator, VkDeviceSize size);
    static Buffer CreateUniformBuffer(const Device& device, VkDeviceSize size);

    // Helper to create storage buffer
    static Buffer CreateStorageBuffer(const VmaAllocator& allocator, VkDeviceSize size);
    static Buffer CreateStorageBuffer(const Device& device, VkDeviceSize size);

private:
    VkBuffer buffer_{VK_NULL_HANDLE};
    VkDeviceSize size_{0};
    VkBufferUsageFlags usage_{0};

    // VMA allocation (raw VMA handle, not the RAII wrapper)
    ::VmaAllocator vmaAllocator_{VK_NULL_HANDLE};
    VmaAllocation allocation_{VK_NULL_HANDLE};
    VmaAllocationInfo allocationInfo_{};

    // Traditional Vulkan memory management
    VkDevice device_{VK_NULL_HANDLE};
    VkDeviceMemory memory_{VK_NULL_HANDLE};
    VkMemoryPropertyFlags memoryProperties_{0};

    bool usingVMA_{false};
    void* mappedData_{nullptr};
    std::string debugName_;

    // Helper methods
    void CreateBuffer();
    void AllocateMemory(VkMemoryPropertyFlags properties);
    void Cleanup();
};

} // namespace VulkanEngine::RAII

#endif // VULKAN_RAII_RESOURCES_BUFFER_HPP