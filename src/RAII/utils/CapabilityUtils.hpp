#ifndef VULKAN_RAII_UTILS_CAPABILITY_UTILS_HPP
#define VULKAN_RAII_UTILS_CAPABILITY_UTILS_HPP

#include <volk.h>

#include <cstdint>
#include <string>
#include <vector>

namespace VulkanEngine::RAII::Utils {

enum class CapabilityRequirement : std::uint8_t {
    REQUIRED,
    OPTIONAL
};

struct NamedCapabilityRequest {
    std::string name;
    CapabilityRequirement requirement{CapabilityRequirement::REQUIRED};
};

struct NamedCapabilityResolution {
    std::vector<std::string> enabled;
    std::vector<std::string> missingRequired;
    std::vector<std::string> missingOptional;
};

struct FeatureResolution {
    VkPhysicalDeviceFeatures enabled{};
    std::vector<std::string> missingRequired;
    std::vector<std::string> missingOptional;
};

NamedCapabilityResolution ResolveNamedCapabilities(const std::vector<NamedCapabilityRequest>& requests,
                                                     const std::vector<std::string>& available);

FeatureResolution ResolveDeviceFeatures(const VkPhysicalDeviceFeatures& supported,
                                          const VkPhysicalDeviceFeatures& required,
                                          const VkPhysicalDeviceFeatures& optional);

std::vector<std::string> EnumerateInstanceExtensionNames();
std::vector<std::string> EnumerateInstanceLayerNames();

} // namespace VulkanEngine::RAII::Utils

#endif // VULKAN_RAII_UTILS_CAPABILITY_UTILS_HPP
