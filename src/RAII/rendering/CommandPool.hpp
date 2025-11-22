#ifndef VULKAN_RAII_RENDERING_COMMAND_POOL_HPP
#define VULKAN_RAII_RENDERING_COMMAND_POOL_HPP

#include <volk.h>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

class CommandPool {
public:
    // Constructor that creates a command pool for the given device and queue family
    CommandPool(const Device& device, 
                uint32_t queue_family_index, 
                VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Destructor
    ~CommandPool();

    // Move constructor and assignment
    CommandPool(CommandPool&& other) noexcept;
    CommandPool& operator=(CommandPool&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkCommandPool by only allowing moving.
    CommandPool(const CommandPool&) = delete;
    CommandPool& operator=(const CommandPool&) = delete;

    [[nodiscard]] VkCommandPool GetHandle() const { return commandPool_; }
    
    // Implicit conversion to VkCommandPool
    operator VkCommandPool() const { return commandPool_; }

    [[nodiscard]] VkDevice GetDevice() const { return device_; }

    // Check if the command pool is valid
    [[nodiscard]] bool IsValid() const { return commandPool_ != VK_NULL_HANDLE; }

    // Allocate command buffers from this pool
    [[nodiscard]] std::vector<VkCommandBuffer> AllocateCommandBuffers(uint32_t count, 
                                                        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

    // Allocate a single command buffer from this pool
    [[nodiscard]] VkCommandBuffer AllocateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

    // Free command buffers back to this pool
    void FreeCommandBuffers(const std::vector<VkCommandBuffer>& command_buffers) const;

    // Free a single command buffer back to this pool
    void FreeCommandBuffer(VkCommandBuffer command_buffer) const;

    // Reset the command pool
    void Reset(VkCommandPoolResetFlags flags = 0) const;

    // Trim the command pool (requires VK_KHR_maintenance1)
    void Trim(VkCommandPoolTrimFlags flags = 0) const;

    // Get queue family index
    [[nodiscard]] uint32_t GetQueueFamilyIndex() const { return queueFamilyIndex_; }

private:
    VkCommandPool commandPool_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    uint32_t queueFamilyIndex_{0};

    // Helper methods
    void CreateCommandPool(VkCommandPoolCreateFlags flags);
    void Cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_COMMAND_POOL_HPP