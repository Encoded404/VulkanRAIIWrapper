#include "DebugUtils.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>



namespace VulkanEngine::RAII::Utils {

std::string StringUtils::result_to_string(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
        default: return "Unknown VkResult";
    }
}

std::string StringUtils::format_to_string(VkFormat format) {
    return std::to_string(format);
}

std::string StringUtils::present_mode_to_string(VkPresentModeKHR present_mode) {
    return std::to_string(present_mode);
}

std::string StringUtils::color_space_to_string(VkColorSpaceKHR color_space) {
    return std::to_string(color_space);
}

std::string StringUtils::image_layout_to_string(VkImageLayout layout) {
    return std::to_string(layout);
}

std::string StringUtils::memory_property_flags_to_string(VkMemoryPropertyFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

std::string StringUtils::buffer_usage_flags_to_string(VkBufferUsageFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

std::string StringUtils::image_usage_flags_to_string(VkImageUsageFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

std::string StringUtils::shader_stage_flags_to_string(VkShaderStageFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

std::string StringUtils::queue_flags_to_string(VkQueueFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

void DebugPrinter::print_instance_extensions() {
    std::cout << "Instance Extensions:" << '\n';
}

void DebugPrinter::print_instance_layers() {
    std::cout << "Instance Layers:" << '\n';
}

void DebugPrinter::print_physical_device_properties(VkPhysicalDevice /*device*/) {
    std::cout << "Physical device properties printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::print_physical_device_features(VkPhysicalDevice /*device*/) {
    std::cout << "Physical device features printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::print_physical_device_memory_properties(VkPhysicalDevice /*device*/) {
    std::cout << "Physical device memory properties printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::print_queue_family_properties(VkPhysicalDevice /*device*/) {
    std::cout << "Queue family properties printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::print_surface_capabilities(VkPhysicalDevice /*device*/, VkSurfaceKHR /*surface*/) {
    std::cout << "Surface capabilities printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::print_swapchain_support(VkPhysicalDevice /*device*/, VkSurfaceKHR /*surface*/) {
    std::cout << "Swapchain support printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::print_device_extensions(VkPhysicalDevice /*device*/) {
    std::cout << "Device extensions printing requires enumeration, not implemented." << '\n';
}

bool ValidationUtils::check_validation_layer_support(const std::vector<const char*>& /*validationLayers*/) {
    return true;
}

std::vector<const char*> ValidationUtils::get_required_extensions(bool enable_validation_layers) {
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    if (enable_validation_layers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

bool ValidationUtils::check_device_extension_support(VkPhysicalDevice /*device*/,
                                                  const std::vector<const char*>& /*requiredExtensions*/) {
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL ValidationUtils::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* /*pUserData*/) {
    std::cerr << "Validation Layer [Severity: " << message_severity
              << ", Type: " << message_type
              << "]: " << (p_callback_data ? p_callback_data->pMessage : "<null>")
              << '\n';
    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT ValidationUtils::create_debug_messenger_create_info() {
    VkDebugUtilsMessengerCreateInfoEXT create_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
    create_info.pUserData = nullptr;
    return create_info;
}

VkResult ValidationUtils::create_debug_utils_messenger_ext(VkInstance instance,
                                                       const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
                                                       const VkAllocationCallbacks* p_allocator,
                                                       VkDebugUtilsMessengerEXT* p_debug_messenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func == nullptr) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return func(instance, p_create_info, p_allocator, p_debug_messenger);
}

void ValidationUtils::destroy_debug_utils_messenger_ext(VkInstance instance,
                                                    VkDebugUtilsMessengerEXT debug_messenger,
                                                    const VkAllocationCallbacks* p_allocator) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        func(instance, debug_messenger, p_allocator);
    }
}

void ErrorUtils::check_result(VkResult result, const std::string& operation) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(operation + " failed with " + StringUtils::result_to_string(result));
    }
}

bool ErrorUtils::is_success(VkResult result) {
    return result == VK_SUCCESS;
}

bool ErrorUtils::is_recoverable_error(VkResult result) {
    switch (result) {
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
            return true;
        default:
            return false;
    }
}

std::string ErrorUtils::get_error_description(VkResult result) {
    return StringUtils::result_to_string(result);
}

void ErrorUtils::throw_on_error(VkResult result, const std::string& operation) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(operation + " failed with " + get_error_description(result));
    }
}

} // namespace VulkanEngine::RAII::Utils


