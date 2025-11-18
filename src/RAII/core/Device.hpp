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

    [[nodiscard]]VkDevice get_handle() const { return device_; }
    
    // Implicit conversion to VkDevice
    operator VkDevice() const { return device_; }

    // Check if the device is valid
    [[nodiscard]]bool is_valid() const { return device_ != VK_NULL_HANDLE; }

    // Get the physical device used to create this logical device
    [[nodiscard]]const PhysicalDevice& get_physical_device() const { return physicalDevice_; }

    // Get queue family indices
    [[nodiscard]]const QueueFamilyIndices& get_queue_family_indices() const { return queueFamilyIndices_; }

    // Wait for device to be idle
    void wait_idle() const;

    // Get a queue from a specific family
    [[nodiscard]]VkQueue get_queue(uint32_t queue_family_index, uint32_t queue_index = 0) const;

    // Get graphics queue
    [[nodiscard]]VkQueue get_graphics_queue() const;

    // Get present queue
    [[nodiscard]]VkQueue get_present_queue() const;

    // Get compute queue (if available)
    [[nodiscard]]VkQueue get_compute_queue() const;

    // Get transfer queue (if available)
    [[nodiscard]]VkQueue get_transfer_queue() const;

    // Memory allocation helpers
    [[nodiscard]]uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    // Buffer creation helpers
    VkResult create_buffer(VkDeviceSize size,
                         VkBufferUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkBuffer& buffer,
                         VkDeviceMemory& buffer_memory) const;

    // Image creation helpers
    VkResult create_image(uint32_t width,
                        uint32_t height,
                        VkFormat format,
                        VkImageTiling tiling,
                        VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        VkImage& image,
                        VkDeviceMemory& image_memory) const;

    // Command buffer helpers (uses an internal transient/resettable command pool)
    [[nodiscard]] VkCommandBuffer begin_single_time_commands() const;
    void end_single_time_commands(VkCommandBuffer command_buffer,
                                  VkQueue submit_queue) const;

    // Templated convenience wrappers to allow local lambdas and functors
    template <typename F>
    void with_single_time_commands(F&& record) const {
        VkQueue queue = get_graphics_queue();
        with_single_time_commands(queue, std::forward<F>(record));
    }

    template <typename F>
    void with_single_time_commands(VkQueue submit_queue, F&& record) const {
        VkCommandBuffer cmd = begin_single_time_commands();
        // Let the callable record into the command buffer
        record(cmd);
        end_single_time_commands(cmd, submit_queue);
    }

    // Format support queries
    [[nodiscard]]VkFormat find_supported_format(const std::vector<VkFormat>& candidates,
                                VkImageTiling tiling,
                                VkFormatFeatureFlags features) const;

    [[nodiscard]]VkFormat find_depth_format() const;

    [[nodiscard]]bool has_stencil_component(VkFormat format) const;

private:
    VkDevice device_{VK_NULL_HANDLE};
    const PhysicalDevice& physicalDevice_;
    QueueFamilyIndices queueFamilyIndices_;
    // Transient/resettable command pool for one-off submissions
    std::unique_ptr<CommandPool> singleUseCommandPool_{};

    // Helper methods
    void create_logical_device(const std::vector<const char*>& required_extensions,
                           const VkPhysicalDeviceFeatures& required_features,
                           const std::vector<const char*>& validation_layers);

    [[nodiscard]]std::vector<VkDeviceQueueCreateInfo> create_queue_create_infos(
        const QueueFamilyIndices& indices) const;
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_CORE_DEVICE_HPP