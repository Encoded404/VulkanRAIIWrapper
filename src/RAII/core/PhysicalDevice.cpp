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
    : instance_(instance.GetHandle()) {
    auto devices = EnumeratePhysicalDevices(instance);
    if (devices.empty()) {
        throw std::runtime_error("No Vulkan physical devices found");
    }

    PhysicalDevice selected = SelectBestDevice(devices, surface);
    if (!selected.IsValid()) {
        throw std::runtime_error("Failed to select a suitable physical device");
    }

    *this = selected;
}

PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device, const Instance& instance)
    : physicalDevice_(physical_device), instance_(instance.GetHandle()) {}

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

VkPhysicalDeviceProperties PhysicalDevice::GetProperties() const {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice_, &properties);
    return properties;
}

VkPhysicalDeviceFeatures PhysicalDevice::GetFeatures() const {
    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(physicalDevice_, &features);
    return features;
}

VkPhysicalDeviceMemoryProperties PhysicalDevice::GetMemoryProperties() const {
    VkPhysicalDeviceMemoryProperties memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memory_properties);
    return memory_properties;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::GetQueueFamilyProperties() const {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    if (queue_family_count > 0) {
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queue_family_count, queue_families.data());
    }
    return queue_families;
}

QueueFamilyIndices PhysicalDevice::FindQueueFamilies(VkSurfaceKHR surface) const {
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

        if (indices.IsComplete()) {
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

bool PhysicalDevice::CheckDeviceExtensionSupport(const std::vector<const char*>& required_extensions) const {
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

std::vector<VkExtensionProperties> PhysicalDevice::GetAvailableExtensions() const {
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extension_count, extensions.data());
    return extensions;
}

SwapChainSupportDetails PhysicalDevice::QuerySwapChainSupport(VkSurfaceKHR surface) const {
    SwapChainSupportDetails details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface, &details.capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.presentModes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface, &present_mode_count, details.presentModes.data());
    }

    return details;
}

bool PhysicalDevice::IsDeviceSuitable(VkSurfaceKHR surface,
                                      const std::vector<const char*>& required_extensions) const {
    QueueFamilyIndices indices = FindQueueFamilies(surface);

    bool extensions_supported = CheckDeviceExtensionSupport(required_extensions);

    bool swap_chain_adequate = true;
    if (extensions_supported && surface != VK_NULL_HANDLE) {
        auto swapchain_support = QuerySwapChainSupport(surface);
        swap_chain_adequate = !swapchain_support.formats.empty() && !swapchain_support.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supported_features = GetFeatures();

    return indices.IsComplete() && extensions_supported && swap_chain_adequate && supported_features.samplerAnisotropy;
}

std::vector<PhysicalDevice> PhysicalDevice::EnumeratePhysicalDevices(const Instance& instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance.GetHandle(), &device_count, nullptr);

    if (device_count == 0) {
        return {};
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance.GetHandle(), &device_count, devices.data());

    std::vector<PhysicalDevice> result;
    result.reserve(device_count);
    for (VkPhysicalDevice device : devices) {
        result.emplace_back(device, instance);
    }
    return result;
}

uint32_t PhysicalDevice::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties mem_properties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
        if ((type_filter & (1u << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

int PhysicalDevice::GetDeviceScore() const {
    int score = 0;
    VkPhysicalDeviceProperties properties = GetProperties();
    VkPhysicalDeviceFeatures features = GetFeatures();

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

PhysicalDevice PhysicalDevice::SelectBestDevice(const std::vector<PhysicalDevice>& devices,
                                               VkSurfaceKHR surface) {
    PhysicalDevice best;
    int best_score = -1;

    for (const auto& device : devices) {
        int score = device.GetDeviceScore();
        if (score <= 0) {
            continue;
        }

        if (surface != VK_NULL_HANDLE) {
            auto swapchain_support = device.QuerySwapChainSupport(surface);
            if (swapchain_support.formats.empty() || swapchain_support.presentModes.empty()) {
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

