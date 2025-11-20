#include "PhysicalDevice.hpp"

#include "instance.hpp"
#include "types/QueueFamilyIndices.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>


namespace VulkanEngine::RAII {

PhysicalDevice::PhysicalDevice(const Instance& instance, VkSurfaceKHR surface)
    : instance_(instance.get_handle()) {
    auto devices = enumerate_physical_devices(instance);
    if (devices.empty()) {
        throw std::runtime_error("No Vulkan physical devices found");
    }

    PhysicalDevice selected = select_best_device(devices, surface);
    if (!selected.is_valid()) {
        throw std::runtime_error("Failed to select a suitable physical device");
    }

    *this = selected;
}

PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device, const Instance& instance)
    : physicalDevice_(physical_device), instance_(instance.get_handle()) {}

PhysicalDevice::PhysicalDevice(PhysicalDevice&& other) noexcept
    : physicalDevice_(other.physicalDevice_), instance_(other.instance_) {
    other.physicalDevice_ = VK_NULL_HANDLE;
    other.instance_ = VK_NULL_HANDLE;
}

PhysicalDevice& PhysicalDevice::operator=(PhysicalDevice&& other) noexcept {
    if (this != &other) {
        physicalDevice_ = other.physicalDevice_;
        instance_ = other.instance_;
        other.physicalDevice_ = VK_NULL_HANDLE;
        other.instance_ = VK_NULL_HANDLE;
    }
    return *this;
}

VkPhysicalDeviceProperties PhysicalDevice::get_properties() const {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice_, &properties);
    return properties;
}

VkPhysicalDeviceFeatures PhysicalDevice::get_features() const {
    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(physicalDevice_, &features);
    return features;
}

VkPhysicalDeviceMemoryProperties PhysicalDevice::get_memory_properties() const {
    VkPhysicalDeviceMemoryProperties memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memory_properties);
    return memory_properties;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::get_queue_family_properties() const {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    if (queue_family_count > 0) {
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queue_family_count, queue_families.data());
    }
    return queue_families;
}

QueueFamilyIndices PhysicalDevice::find_queue_families(VkSurfaceKHR surface) const {
    QueueFamilyIndices indices;
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; ++i) {
        const auto& queue_family = queue_families[i];

        if (queue_family.queueCount > 0 && (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.graphicsFamily_ = i;
        }

        if (queue_family.queueCount > 0 && (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            indices.computeFamily_ = i;
        }

        if (queue_family.queueCount > 0 && (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            !(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.transferFamily_ = i;
        }

        if (surface != VK_NULL_HANDLE) {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, surface, &present_support);
            if (queue_family.queueCount > 0 && present_support) {
                indices.presentFamily_ = i;
            }
        }

        if (indices.is_complete()) {
            break;
        }
    }

    if (!indices.presentFamily_.has_value() && indices.graphicsFamily_.has_value()) {
        // Fallback: assume graphics queue supports present if surface not provided
        if (surface == VK_NULL_HANDLE) {
            indices.presentFamily_ = indices.graphicsFamily_;
        }
    }

    if (!indices.transferFamily_.has_value()) {
        indices.transferFamily_ = indices.graphicsFamily_;
    }

    if (!indices.computeFamily_.has_value()) {
        indices.computeFamily_ = indices.graphicsFamily_;
    }

    return indices;
}

bool PhysicalDevice::check_device_extension_support(const std::vector<const char*>& required_extensions) const {
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extension_count, available_extensions.data());

    std::vector<const char*> missing = required_extensions;
    for (const VkExtensionProperties& extension : available_extensions) {
        std::vector<const char*>::iterator it = std::remove_if(missing.begin(), missing.end(), [&](const char* name) -> bool { // NOLINT
            return std::string(name) == extension.extensionName;
        });
        missing.erase(it, missing.end());
        if (missing.empty()) {
            break;
        }
    }

    return missing.empty();
}

std::vector<VkExtensionProperties> PhysicalDevice::get_available_extensions() const {
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extension_count, extensions.data());
    return extensions;
}

SwapChainSupportDetails PhysicalDevice::query_swap_chain_support(VkSurfaceKHR surface) const {
    SwapChainSupportDetails details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface, &details.capabilities_);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats_.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &format_count, details.formats_.data());
    }

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.presentModes_.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface, &present_mode_count, details.presentModes_.data());
    }

    return details;
}

bool PhysicalDevice::is_device_suitable(VkSurfaceKHR surface,
                                      const std::vector<const char*>& required_extensions) const {
    QueueFamilyIndices indices = find_queue_families(surface);

    bool extensions_supported = check_device_extension_support(required_extensions);

    bool swap_chain_adequate = true;
    if (extensions_supported && surface != VK_NULL_HANDLE) {
        auto swapchain_support = query_swap_chain_support(surface);
        swap_chain_adequate = !swapchain_support.formats_.empty() && !swapchain_support.presentModes_.empty();
    }

    VkPhysicalDeviceFeatures supported_features = get_features();

    return indices.is_complete() && extensions_supported && swap_chain_adequate && supported_features.samplerAnisotropy;
}

std::vector<PhysicalDevice> PhysicalDevice::enumerate_physical_devices(const Instance& instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance.get_handle(), &device_count, nullptr);

    if (device_count == 0) {
        return {};
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance.get_handle(), &device_count, devices.data());

    std::vector<PhysicalDevice> result;
    result.reserve(device_count);
    for (VkPhysicalDevice device : devices) {
        result.emplace_back(device, instance);
    }
    return result;
}

uint32_t PhysicalDevice::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties mem_properties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
        if ((type_filter & (1u << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

int PhysicalDevice::get_device_score() const {
    int score = 0;
    VkPhysicalDeviceProperties properties = get_properties();
    VkPhysicalDeviceFeatures features = get_features();

    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }
    if (features.multiDrawIndirect) {
        score += 200;
    }

    score += static_cast<int>(properties.limits.maxImageDimension2D) / 4;

    if (!features.geometryShader) {
        score = 0;
    }

    return score;
}

PhysicalDevice PhysicalDevice::select_best_device(const std::vector<PhysicalDevice>& devices,
                                               VkSurfaceKHR surface) {
    PhysicalDevice best;
    int best_score = -1;

    for (const auto& device : devices) {
        int score = device.get_device_score();
        if (score <= 0) {
            continue;
        }

        if (surface != VK_NULL_HANDLE) {
            auto swapchain_support = device.query_swap_chain_support(surface);
            if (swapchain_support.formats_.empty() || swapchain_support.presentModes_.empty()) {
                continue;
            }
        }

        if (score > best_score) {
            best_score = score;
            best = device;
        }
    }

    return best;
}

} // namespace VulkanEngine::RAII

