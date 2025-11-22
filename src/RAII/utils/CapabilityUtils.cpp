#include "CapabilityUtils.hpp"

#include <array>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <cstdint>

#include <volk.h>

namespace VulkanEngine::RAII::Utils {

namespace {

struct FeatureFlagEntry {
    const char* name;
    VkBool32 VkPhysicalDeviceFeatures::* member;
};

constexpr std::array<FeatureFlagEntry, 55> kFeatureFlagTable = {{
    {"robustBufferAccess", &VkPhysicalDeviceFeatures::robustBufferAccess},
    {"fullDrawIndexUint32", &VkPhysicalDeviceFeatures::fullDrawIndexUint32},
    {"imageCubeArray", &VkPhysicalDeviceFeatures::imageCubeArray},
    {"independentBlend", &VkPhysicalDeviceFeatures::independentBlend},
    {"geometryShader", &VkPhysicalDeviceFeatures::geometryShader},
    {"tessellationShader", &VkPhysicalDeviceFeatures::tessellationShader},
    {"sampleRateShading", &VkPhysicalDeviceFeatures::sampleRateShading},
    {"dualSrcBlend", &VkPhysicalDeviceFeatures::dualSrcBlend},
    {"logicOp", &VkPhysicalDeviceFeatures::logicOp},
    {"multiDrawIndirect", &VkPhysicalDeviceFeatures::multiDrawIndirect},
    {"drawIndirectFirstInstance", &VkPhysicalDeviceFeatures::drawIndirectFirstInstance},
    {"depthClamp", &VkPhysicalDeviceFeatures::depthClamp},
    {"depthBiasClamp", &VkPhysicalDeviceFeatures::depthBiasClamp},
    {"fillModeNonSolid", &VkPhysicalDeviceFeatures::fillModeNonSolid},
    {"depthBounds", &VkPhysicalDeviceFeatures::depthBounds},
    {"wideLines", &VkPhysicalDeviceFeatures::wideLines},
    {"largePoints", &VkPhysicalDeviceFeatures::largePoints},
    {"alphaToOne", &VkPhysicalDeviceFeatures::alphaToOne},
    {"multiViewport", &VkPhysicalDeviceFeatures::multiViewport},
    {"samplerAnisotropy", &VkPhysicalDeviceFeatures::samplerAnisotropy},
    {"textureCompressionETC2", &VkPhysicalDeviceFeatures::textureCompressionETC2},
    {"textureCompressionASTC_LDR", &VkPhysicalDeviceFeatures::textureCompressionASTC_LDR},
    {"textureCompressionBC", &VkPhysicalDeviceFeatures::textureCompressionBC},
    {"occlusionQueryPrecise", &VkPhysicalDeviceFeatures::occlusionQueryPrecise},
    {"pipelineStatisticsQuery", &VkPhysicalDeviceFeatures::pipelineStatisticsQuery},
    {"vertexPipelineStoresAndAtomics", &VkPhysicalDeviceFeatures::vertexPipelineStoresAndAtomics},
    {"fragmentStoresAndAtomics", &VkPhysicalDeviceFeatures::fragmentStoresAndAtomics},
    {"shaderTessellationAndGeometryPointSize", &VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize},
    {"shaderImageGatherExtended", &VkPhysicalDeviceFeatures::shaderImageGatherExtended},
    {"shaderStorageImageExtendedFormats", &VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats},
    {"shaderStorageImageMultisample", &VkPhysicalDeviceFeatures::shaderStorageImageMultisample},
    {"shaderStorageImageReadWithoutFormat", &VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat},
    {"shaderStorageImageWriteWithoutFormat", &VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat},
    {"shaderUniformBufferArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing},
    {"shaderSampledImageArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing},
    {"shaderStorageBufferArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing},
    {"shaderStorageImageArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing},
    {"shaderClipDistance", &VkPhysicalDeviceFeatures::shaderClipDistance},
    {"shaderCullDistance", &VkPhysicalDeviceFeatures::shaderCullDistance},
    {"shaderFloat64", &VkPhysicalDeviceFeatures::shaderFloat64},
    {"shaderInt64", &VkPhysicalDeviceFeatures::shaderInt64},
    {"shaderInt16", &VkPhysicalDeviceFeatures::shaderInt16},
    {"shaderResourceResidency", &VkPhysicalDeviceFeatures::shaderResourceResidency},
    {"shaderResourceMinLod", &VkPhysicalDeviceFeatures::shaderResourceMinLod},
    {"sparseBinding", &VkPhysicalDeviceFeatures::sparseBinding},
    {"sparseResidencyBuffer", &VkPhysicalDeviceFeatures::sparseResidencyBuffer},
    {"sparseResidencyImage2D", &VkPhysicalDeviceFeatures::sparseResidencyImage2D},
    {"sparseResidencyImage3D", &VkPhysicalDeviceFeatures::sparseResidencyImage3D},
    {"sparseResidency2Samples", &VkPhysicalDeviceFeatures::sparseResidency2Samples},
    {"sparseResidency4Samples", &VkPhysicalDeviceFeatures::sparseResidency4Samples},
    {"sparseResidency8Samples", &VkPhysicalDeviceFeatures::sparseResidency8Samples},
    {"sparseResidency16Samples", &VkPhysicalDeviceFeatures::sparseResidency16Samples},
    {"sparseResidencyAliased", &VkPhysicalDeviceFeatures::sparseResidencyAliased},
    {"variableMultisampleRate", &VkPhysicalDeviceFeatures::variableMultisampleRate},
    {"inheritedQueries", &VkPhysicalDeviceFeatures::inheritedQueries}
}};

std::unordered_map<std::string, CapabilityRequirement> AggregateRequests(
    const std::vector<NamedCapabilityRequest>& requests,
    std::vector<std::string>& ordered_names) {
    std::unordered_map<std::string, CapabilityRequirement> merged;
    ordered_names.clear();
    ordered_names.reserve(requests.size());

    for (const NamedCapabilityRequest& request : requests) {
        if (request.name.empty()) {
            continue;
        }

        auto [it, inserted] = merged.emplace(request.name, request.requirement);
        if (inserted) {
            ordered_names.push_back(request.name);
        } else if (request.requirement == CapabilityRequirement::REQUIRED &&
                   it->second != CapabilityRequirement::REQUIRED) {
            it->second = CapabilityRequirement::REQUIRED;
        }
    }

    return merged;
}

std::unordered_set<std::string> MakeAvailableSet(const std::vector<std::string>& available) {
    std::unordered_set<std::string> available_set;
    available_set.reserve(available.size());
    for (const std::string& name : available) {
        available_set.insert(name);
    }
    return available_set;
}

void EnsureVolkInitialized() {
    const VkResult result = volkInitialize();
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to initialize volk for capability queries");
    }
}

} // namespace

NamedCapabilityResolution ResolveNamedCapabilities(const std::vector<NamedCapabilityRequest>& requests,
                                                     const std::vector<std::string>& available) {
    NamedCapabilityResolution resolution;
    if (requests.empty()) {
        return resolution;
    }

    std::vector<std::string> ordered_names;
    auto merged = AggregateRequests(requests, ordered_names);
    auto available_set = MakeAvailableSet(available);

    resolution.enabled.reserve(ordered_names.size());

    for (const std::string& name : ordered_names) {
        const CapabilityRequirement requirement = merged[name];
        if (available_set.contains(name)) {
            resolution.enabled.push_back(name);
            continue;
        }

        if (requirement == CapabilityRequirement::REQUIRED) {
            resolution.missingRequired.push_back(name);
        } else {
            resolution.missingOptional.push_back(name);
        }
    }

    return resolution;
}

FeatureResolution ResolveDeviceFeatures(const VkPhysicalDeviceFeatures& supported,
                                          const VkPhysicalDeviceFeatures& required,
                                          const VkPhysicalDeviceFeatures& optional) {
    FeatureResolution resolution{};

    for (const FeatureFlagEntry& entry : kFeatureFlagTable) {
        const VkBool32 supported_flag = supported.*(entry.member);
        const VkBool32 required_flag = required.*(entry.member);
        const VkBool32 optional_flag = optional.*(entry.member);

        if (required_flag == VK_TRUE) {
            if (supported_flag == VK_TRUE) {
                resolution.enabled.*(entry.member) = VK_TRUE;
            } else {
                resolution.missingRequired.emplace_back(entry.name);
            }
            continue;
        }

        if (optional_flag == VK_TRUE) {
            if (supported_flag == VK_TRUE) {
                resolution.enabled.*(entry.member) = VK_TRUE;
            } else {
                resolution.missingOptional.emplace_back(entry.name);
            }
        }
    }

    return resolution;
}

std::vector<std::string> EnumerateInstanceExtensionNames() {
    EnsureVolkInitialized();

    uint32_t count = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to enumerate instance extensions");
    }

    std::vector<VkExtensionProperties> properties(count);
    if (count > 0) {
        result = vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to enumerate instance extensions");
        }
    }

    std::vector<std::string> names;
    names.reserve(properties.size());
    for (const VkExtensionProperties& prop : properties) {
        names.emplace_back(prop.extensionName);
    }
    return names;
}

std::vector<std::string> EnumerateInstanceLayerNames() {
    EnsureVolkInitialized();

    uint32_t count = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&count, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to enumerate instance layers");
    }

    std::vector<VkLayerProperties> properties(count);
    if (count > 0) {
        result = vkEnumerateInstanceLayerProperties(&count, properties.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to enumerate instance layers");
        }
    }

    std::vector<std::string> names;
    names.reserve(properties.size());
    for (const VkLayerProperties& prop : properties) {
        names.emplace_back(prop.layerName);
    }
    return names;
}

} // namespace VulkanEngine::RAII::Utils
