#ifndef VULKAN_RAII_CORE_PHYSICAL_DEVICE_HPP
#define VULKAN_RAII_CORE_PHYSICAL_DEVICE_HPP

#include <volk.h>
#include <vector>
#include <set>

#include "types/QueueFamilyIndices.hpp" // Include the header that defines QueueFamilyIndices


namespace VulkanEngine::RAII {

class Instance; // Forward declaration

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities_;
    std::vector<VkSurfaceFormatKHR> formats_;
    std::vector<VkPresentModeKHR> presentModes_;
};

class PhysicalDevice {
public:
    // Constructor that selects the best physical device from available devices
    explicit PhysicalDevice(const Instance& instance, VkSurfaceKHR surface = VK_NULL_HANDLE);

    // Constructor that wraps an existing VkPhysicalDevice
    PhysicalDevice(VkPhysicalDevice physical_device, const Instance& instance);

    // Default constructor (creates invalid device)
    PhysicalDevice() = default;

    // Move constructor and assignment
    PhysicalDevice(PhysicalDevice&& other) noexcept;
    PhysicalDevice& operator=(PhysicalDevice&& other) noexcept;

    // Copy constructor and assignment (copy allowed for PhysicalDevice)
    PhysicalDevice(const PhysicalDevice&) = default;
    PhysicalDevice& operator=(const PhysicalDevice&) = default;

    [[nodiscard]] VkPhysicalDevice get_handle() const { return physicalDevice_; }
    
    // Implicit conversion to VkPhysicalDevice
    operator VkPhysicalDevice() const { return physicalDevice_; }

    // Check if the physical device is valid
    [[nodiscard]] bool is_valid() const { return physicalDevice_ != VK_NULL_HANDLE; }

    // Get device properties
    [[nodiscard]] VkPhysicalDeviceProperties get_properties() const;

    // Get device features
    [[nodiscard]] VkPhysicalDeviceFeatures get_features() const;

    // Get device memory properties
    [[nodiscard]] VkPhysicalDeviceMemoryProperties get_memory_properties() const;

    // Get queue family properties
    [[nodiscard]] std::vector<VkQueueFamilyProperties> get_queue_family_properties() const;

    // Find queue families
    QueueFamilyIndices find_queue_families(VkSurfaceKHR surface = VK_NULL_HANDLE) const;

    // Check device extension support
    [[nodiscard]] bool check_device_extension_support(const std::vector<const char*>& required_extensions) const;

    // Get available device extensions
    [[nodiscard]] std::vector<VkExtensionProperties> get_available_extensions() const;

    // Query swap chain support
    SwapChainSupportDetails query_swap_chain_support(VkSurfaceKHR surface) const;

    // Check if device is suitable for our needs
    bool is_device_suitable(VkSurfaceKHR surface = VK_NULL_HANDLE,
                         const std::vector<const char*>& required_extensions = {}) const;

    // Get all physical devices from instance
    static std::vector<PhysicalDevice> enumerate_physical_devices(const Instance& instance);

    // Find memory type index
    [[nodiscard]] uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    // Get device score for device selection
    [[nodiscard]] int get_device_score() const;

private:
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
    VkInstance instance_{VK_NULL_HANDLE}; // Reference to instance

    // Helper methods
    static PhysicalDevice select_best_device(const std::vector<PhysicalDevice>& devices, 
                                         VkSurfaceKHR surface);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_CORE_PHYSICAL_DEVICE_HPP