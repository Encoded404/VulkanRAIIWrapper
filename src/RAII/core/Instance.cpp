#include "instance.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <volk.h>


namespace VulkanEngine::RAII {

namespace {
constexpr const char* DEFAULT_ENGINE_NAME = "VulkanEngine";
constexpr uint32_t DEFAULT_ENGINE_VERSION = VK_MAKE_VERSION(1, 0, 0);
constexpr uint32_t DEFAULT_API_VERSION = VK_API_VERSION_1_2;
} // namespace

Instance::Instance(const std::string& application_name,
                   uint32_t application_version,
                   const std::vector<const char*>& required_extensions,
                   const std::vector<const char*>& validation_layers) {
    if (volkInitialize()) {
        throw std::runtime_error("Failed to initialize Volk");
    }

    std::vector<const char*> extensions = required_extensions;
    if (validation_layers.empty()) {
        // No-op
    } else {
        const char* debug_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        if (std::find(extensions.begin(), extensions.end(), debug_ext) == extensions.end()) {
            extensions.push_back(debug_ext);
        }
    }

    create_instance(application_name,
                   application_version,
                   DEFAULT_ENGINE_NAME,
                   DEFAULT_ENGINE_VERSION,
                   DEFAULT_API_VERSION,
                   extensions,
                   validation_layers);

    volkLoadInstance(instance_);
}

Instance::~Instance() {
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }
}

Instance::Instance(Instance&& other) noexcept : instance_(other.instance_) {
    other.instance_ = VK_NULL_HANDLE;
}

Instance& Instance::operator=(Instance&& other) noexcept {
    if (this != &other) {
        if (instance_ != VK_NULL_HANDLE) {
            vkDestroyInstance(instance_, nullptr);
        }
        instance_ = other.instance_;
        other.instance_ = VK_NULL_HANDLE;
    }
    return *this;
}

void Instance::create_instance(const std::string& application_name,
                              uint32_t application_version,
                              const std::string& engine_name,
                              uint32_t engine_version,
                              uint32_t api_version,
                              const std::vector<const char*>& required_extensions,
                              const std::vector<const char*>& validation_layers) {
    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pApplicationName = application_name.c_str();
    app_info.applicationVersion = application_version;
    app_info.pEngineName = engine_name.c_str();
    app_info.engineVersion = engine_version;
    app_info.apiVersion = api_version;

    if (!validation_layers.empty() && !check_validation_layer_support(validation_layers)) {
        throw std::runtime_error("Requested validation layers are not available");
    }

    VkInstanceCreateInfo create_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.empty() ? nullptr : required_extensions.data();
    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.empty() ? nullptr : validation_layers.data();

    VkResult result = vkCreateInstance(&create_info, nullptr, &instance_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

bool Instance::check_validation_layer_support(const std::vector<const char*>& validation_layers) {
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : validation_layers) {
        bool found = false;
        for (const auto& layer_properties : available_layers) {
            if (std::string(layer_name) == layer_properties.layerName) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

std::vector<VkExtensionProperties> Instance::get_available_extensions() {
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
    return extensions;
}

std::vector<VkLayerProperties> Instance::get_available_layers() {
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
    return layers;
}

bool Instance::is_extension_supported(const std::string& extension) {
    auto extensions = get_available_extensions();
    return std::any_of(extensions.begin(), extensions.end(), [&](const VkExtensionProperties& props) {
        return extension == props.extensionName;
    });
}

bool Instance::is_layer_supported(const std::string& layer) {
    auto layers = get_available_layers();
    return std::any_of(layers.begin(), layers.end(), [&](const VkLayerProperties& props) {
        return layer == props.layerName;
    });
}

} // namespace VulkanEngine::RAII

