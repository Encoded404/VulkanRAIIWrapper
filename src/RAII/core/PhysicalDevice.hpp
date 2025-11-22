#ifndef VULKAN_RAII_CORE_PHYSICAL_DEVICE_HPP
#define VULKAN_RAII_CORE_PHYSICAL_DEVICE_HPP

#include <volk.h>
#include <vector>
#include <set>

#include "types/QueueFamilyIndices.hpp" // Include the header that defines QueueFamilyIndices


namespace VulkanEngine::RAII {

class Instance; // Forward declaration

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
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

    [[nodiscard]] VkPhysicalDevice GetHandle() const { return physicalDevice_; }
    
    // Implicit conversion to VkPhysicalDevice
    operator VkPhysicalDevice() const { return physicalDevice_; }

    // Check if the physical device is valid
    [[nodiscard]] bool IsValid() const { return physicalDevice_ != VK_NULL_HANDLE; }

    // Get device properties
    [[nodiscard]] VkPhysicalDeviceProperties GetProperties() const;

    // Get device features
    [[nodiscard]] VkPhysicalDeviceFeatures GetFeatures() const;

    // Get device memory properties
    [[nodiscard]] VkPhysicalDeviceMemoryProperties GetMemoryProperties() const;

    // Get queue family properties
    [[nodiscard]] std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const;

    // Find queue families
    QueueFamilyIndices FindQueueFamilies(VkSurfaceKHR surface = VK_NULL_HANDLE) const;

    // Check device extension support
    [[nodiscard]] bool CheckDeviceExtensionSupport(const std::vector<const char*>& required_extensions) const;

    // Get available device extensions
    [[nodiscard]] std::vector<VkExtensionProperties> GetAvailableExtensions() const;

    // Query swap chain support
    SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface) const;

    // Check if device is suitable for our needs
    bool IsDeviceSuitable(VkSurfaceKHR surface = VK_NULL_HANDLE,
                         const std::vector<const char*>& required_extensions = {}) const;

    // Get all physical devices from instance
    static std::vector<PhysicalDevice> EnumeratePhysicalDevices(const Instance& instance);

    // Find memory type index
    [[nodiscard]] uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    // Get device score for device selection
    [[nodiscard]] int GetDeviceScore() const;

private:
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
    VkInstance instance_{VK_NULL_HANDLE}; // Reference to instance

    // Helper methods
    static PhysicalDevice SelectBestDevice(const std::vector<PhysicalDevice>& devices, 
                                         VkSurfaceKHR surface);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_CORE_PHYSICAL_DEVICE_HPP