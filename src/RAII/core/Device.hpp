#ifndef VULKAN_RAII_CORE_DEVICE_HPP
#define VULKAN_RAII_CORE_DEVICE_HPP

#include <volk.h>
#include <vector>
#include <memory>
#include <unordered_set>
#include <utility>

#include "../types/QueueFamilyIndices.hpp" // Include the header that defines QueueFamilyIndices
#include "../rendering/CommandPool.hpp"


namespace VulkanEngine::RAII {

class PhysicalDevice; // Forward declaration

class Device {
public:
    // Constructor that creates a logical device from a physical device
    Device(const PhysicalDevice& physical_device,
           const std::vector<const char*>& required_extensions = {},
           const VkPhysicalDeviceFeatures& required_features = {},
           const std::vector<const char*>& validation_layers = {});

    // Destructor
    ~Device();

    // Move constructor and assignment
    Device(Device&& other) noexcept;
    Device& operator=(Device&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkDevice by only allowing moving.
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    [[nodiscard]]VkDevice GetHandle() const { return device_; }
    
    // Implicit conversion to VkDevice
    operator VkDevice() const { return device_; }

    // Check if the device is valid
    [[nodiscard]]bool IsValid() const { return device_ != VK_NULL_HANDLE; }

    // Get the physical device used to create this logical device
    [[nodiscard]]const PhysicalDevice& GetPhysicalDevice() const { return physicalDevice_; }

    // Get queue family indices
    [[nodiscard]]const QueueFamilyIndices& GetQueueFamilyIndices() const { return queueFamilyIndices_; }

    // Wait for device to be idle
    void WaitIdle() const;

    // Get a queue from a specific family
    [[nodiscard]]VkQueue GetQueue(uint32_t queue_family_index, uint32_t queue_index = 0) const;

    // Get graphics queue
    [[nodiscard]]VkQueue GetGraphicsQueue() const;

    // Get present queue
    [[nodiscard]]VkQueue GetPresentQueue() const;

    // Get compute queue (if available)
    [[nodiscard]]VkQueue GetComputeQueue() const;

    // Get transfer queue (if available)
    [[nodiscard]]VkQueue GetTransferQueue() const;

    // Memory allocation helpers
    [[nodiscard]]uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    // Buffer creation helpers
    VkResult CreateBuffer(VkDeviceSize size,
                         VkBufferUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkBuffer& buffer,
                         VkDeviceMemory& buffer_memory) const;

    // Image creation helpers
    VkResult CreateImage(uint32_t width,
                        uint32_t height,
                        VkFormat format,
                        VkImageTiling tiling,
                        VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        VkImage& image,
                        VkDeviceMemory& image_memory) const;

    // Command buffer helpers (uses an internal transient/resettable command pool)
    [[nodiscard]] VkCommandBuffer BeginSingleTimeCommands() const;
    void EndSingleTimeCommands(VkCommandBuffer command_buffer,
                                  VkQueue submit_queue) const;

    // Templated convenience wrappers to allow local lambdas and functors
    template <typename F>
    void WithSingleTimeCommands(F&& record) const {
        VkQueue queue = GetGraphicsQueue();
        WithSingleTimeCommands(queue, std::forward<F>(record));
    }

    template <typename F>
    void WithSingleTimeCommands(VkQueue submit_queue, F&& record) const {
        VkCommandBuffer cmd = BeginSingleTimeCommands();
        // Let the callable record into the command buffer
        record(cmd);
        EndSingleTimeCommands(cmd, submit_queue);
    }

    // Format support queries
    [[nodiscard]]VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                VkImageTiling tiling,
                                VkFormatFeatureFlags features) const;

    [[nodiscard]]VkFormat FindDepthFormat() const;

    [[nodiscard]]bool HasStencilComponent(VkFormat format) const;

private:
    VkDevice device_{VK_NULL_HANDLE};
    const PhysicalDevice& physicalDevice_;
    QueueFamilyIndices queueFamilyIndices_;
    // Transient/resettable command pool for one-off submissions
    std::unique_ptr<CommandPool> singleUseCommandPool_{};

    // Helper methods
    void CreateLogicalDevice(const std::vector<const char*>& required_extensions,
                           const VkPhysicalDeviceFeatures& required_features,
                           const std::vector<const char*>& validation_layers);

    [[nodiscard]]std::vector<VkDeviceQueueCreateInfo> CreateQueueCreateInfos(
        const QueueFamilyIndices& indices) const;
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_CORE_DEVICE_HPP