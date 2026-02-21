#ifndef VULKAN_RAII_RESOURCES_BUFFER_HPP
#define VULKAN_RAII_RESOURCES_BUFFER_HPP

#include <volk.h>
#include <string>

namespace VulkanEngine::RAII {

class Device;

class Buffer {
public:
    Buffer(const Device& device,
           VkDeviceSize size,
           VkBufferUsageFlags usage,
           VkMemoryPropertyFlags properties,
           const char* name = nullptr);

    ~Buffer();
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    [[nodiscard]] VkBuffer GetHandle() const { return buffer_; }
    operator VkBuffer() const { return buffer_; }
    [[nodiscard]] bool IsValid() const { return buffer_ != VK_NULL_HANDLE; }
    [[nodiscard]] VkDeviceSize GetSize() const { return size_; }
    [[nodiscard]] VkBufferUsageFlags GetUsage() const { return usage_; }

    void* Map();
    void Unmap();

    void* MapMemory();
    void UnmapMemory();

    void WriteData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    void ReadData(void* data, VkDeviceSize size, VkDeviceSize offset = 0);

    [[nodiscard]] VkMemoryRequirements GetMemoryRequirements() const;

    void BindMemory(VkDeviceMemory memory, VkDeviceSize offset = 0);
    [[nodiscard]] VkDeviceMemory GetMemory() const { return memory_; }

    void Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void CopyFrom(const Buffer& src_buffer, VkDeviceSize size = VK_WHOLE_SIZE,
                  VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0);

    void CopyFrom(VkCommandBuffer command_buffer, const Buffer& src_buffer,
                  VkDeviceSize size = VK_WHOLE_SIZE,
                  VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0);

    void SetDebugName(const char* name);
    [[nodiscard]] const char* GetDebugName() const { return debugName_.c_str(); }

    static Buffer CreateStaging(const Device& device, VkDeviceSize size);
    static Buffer CreateVertexBuffer(const Device& device, VkDeviceSize size);
    static Buffer CreateIndexBuffer(const Device& device, VkDeviceSize size);
    static Buffer CreateUniformBuffer(const Device& device, VkDeviceSize size);
    static Buffer CreateStorageBuffer(const Device& device, VkDeviceSize size);

private:
    VkBuffer buffer_{VK_NULL_HANDLE};
    VkDeviceSize size_{0};
    VkBufferUsageFlags usage_{0};

    VkDevice device_{VK_NULL_HANDLE};
    VkDeviceMemory memory_{VK_NULL_HANDLE};
    VkMemoryPropertyFlags memoryProperties_{0};

    void* mappedData_{nullptr};
    std::string debugName_;

    void Cleanup();
};

} // namespace VulkanEngine::RAII

#endif // VULKAN_RAII_RESOURCES_BUFFER_HPP
