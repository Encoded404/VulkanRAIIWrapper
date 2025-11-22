#include "DebugUtils.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>



namespace VulkanEngine::RAII::Utils {

std::string StringUtils::ResultToString(VkResult result) {
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

std::string StringUtils::FormatToString(VkFormat format) {
    return std::to_string(format);
}

std::string StringUtils::PresentModeToString(VkPresentModeKHR present_mode) {
    return std::to_string(present_mode);
}

std::string StringUtils::ColorSpaceToString(VkColorSpaceKHR color_space) {
    return std::to_string(color_space);
}

std::string StringUtils::ImageLayoutToString(VkImageLayout layout) {
    return std::to_string(layout);
}

std::string StringUtils::MemoryPropertyFlagsToString(VkMemoryPropertyFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

std::string StringUtils::BufferUsageFlagsToString(VkBufferUsageFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

std::string StringUtils::ImageUsageFlagsToString(VkImageUsageFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

std::string StringUtils::ShaderStageFlagsToString(VkShaderStageFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

std::string StringUtils::QueueFlagsToString(VkQueueFlags flags) {
    std::ostringstream oss;
    oss << std::hex << flags;
    return oss.str();
}

void DebugPrinter::PrintInstanceExtensions() {
    std::cout << "Instance Extensions:" << '\n';
}

void DebugPrinter::PrintInstanceLayers() {
    std::cout << "Instance Layers:" << '\n';
}

void DebugPrinter::PrintPhysicalDeviceProperties(VkPhysicalDevice /*device*/) {
    std::cout << "Physical device properties printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::PrintPhysicalDeviceFeatures(VkPhysicalDevice /*device*/) {
    std::cout << "Physical device features printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::PrintPhysicalDeviceMemoryProperties(VkPhysicalDevice /*device*/) {
    std::cout << "Physical device memory properties printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::PrintQueueFamilyProperties(VkPhysicalDevice /*device*/) {
    std::cout << "Queue family properties printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::PrintSurfaceCapabilities(VkPhysicalDevice /*device*/, VkSurfaceKHR /*surface*/) {
    std::cout << "Surface capabilities printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::PrintSwapchainSupport(VkPhysicalDevice /*device*/, VkSurfaceKHR /*surface*/) {
    std::cout << "Swapchain support printing requires enumeration, not implemented." << '\n';
}

void DebugPrinter::PrintDeviceExtensions(VkPhysicalDevice /*device*/) {
    std::cout << "Device extensions printing requires enumeration, not implemented." << '\n';
}

bool ValidationUtils::CheckValidationLayerSupport(const std::vector<const char*>& /*validationLayers*/) {
    return true;
}

std::vector<const char*> ValidationUtils::GetRequiredExtensions(bool enable_validation_layers) {
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    if (enable_validation_layers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

bool ValidationUtils::CheckDeviceExtensionSupport(VkPhysicalDevice /*device*/,
                                                  const std::vector<const char*>& /*requiredExtensions*/) {
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL ValidationUtils::DebugCallback(
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

VkDebugUtilsMessengerCreateInfoEXT ValidationUtils::CreateDebugMessengerCreateInfo() {
    VkDebugUtilsMessengerCreateInfoEXT create_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = DebugCallback;
    create_info.pUserData = nullptr;
    return create_info;
}

VkResult ValidationUtils::CreateDebugUtilsMessengerExt(VkInstance instance,
                                                       const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
                                                       const VkAllocationCallbacks* p_allocator,
                                                       VkDebugUtilsMessengerEXT* p_debug_messenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func == nullptr) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return func(instance, p_create_info, p_allocator, p_debug_messenger);
}

void ValidationUtils::DestroyDebugUtilsMessengerExt(VkInstance instance,
                                                    VkDebugUtilsMessengerEXT debug_messenger,
                                                    const VkAllocationCallbacks* p_allocator) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        func(instance, debug_messenger, p_allocator);
    }
}

void ErrorUtils::CheckResult(VkResult result, const std::string& operation) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(operation + " failed with " + StringUtils::ResultToString(result));
    }
}

bool ErrorUtils::IsSuccess(VkResult result) {
    return result == VK_SUCCESS;
}

bool ErrorUtils::IsRecoverableError(VkResult result) {
    switch (result) {
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
            return true;
        default:
            return false;
    }
}

std::string ErrorUtils::GetErrorDescription(VkResult result) {
    return StringUtils::ResultToString(result);
}

void ErrorUtils::ThrowOnError(VkResult result, const std::string& operation) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(operation + " failed with " + GetErrorDescription(result));
    }
}

} // namespace VulkanEngine::RAII::Utils


