#ifndef VULKAN_RAII_UTILS_DEBUG_UTILS_HPP
#define VULKAN_RAII_UTILS_DEBUG_UTILS_HPP

#include <volk.h>
#include <string>
#include <vector>



namespace VulkanEngine::RAII::Utils {

// String conversion utilities for debugging
class StringUtils {
public:
    static std::string ResultToString(VkResult result);
    static std::string FormatToString(VkFormat format);
    static std::string PresentModeToString(VkPresentModeKHR present_mode);
    static std::string ColorSpaceToString(VkColorSpaceKHR color_space);
    static std::string ImageLayoutToString(VkImageLayout layout);
    static std::string MemoryPropertyFlagsToString(VkMemoryPropertyFlags flags);
    static std::string BufferUsageFlagsToString(VkBufferUsageFlags flags);
    static std::string ImageUsageFlagsToString(VkImageUsageFlags flags);
    static std::string ShaderStageFlagsToString(VkShaderStageFlags flags);
    static std::string QueueFlagsToString(VkQueueFlags flags);
};

// Debug printing utilities
class DebugPrinter {
public:
    static void PrintInstanceExtensions();
    static void PrintInstanceLayers();
    static void PrintPhysicalDeviceProperties(VkPhysicalDevice device);
    static void PrintPhysicalDeviceFeatures(VkPhysicalDevice device);
    static void PrintPhysicalDeviceMemoryProperties(VkPhysicalDevice device);
    static void PrintQueueFamilyProperties(VkPhysicalDevice device);
    static void PrintSurfaceCapabilities(VkPhysicalDevice device, VkSurfaceKHR surface);
    static void PrintSwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    static void PrintDeviceExtensions(VkPhysicalDevice device);
};

// Validation utilities
class ValidationUtils {
public:
    // Check if validation layers are available
    static bool CheckValidationLayerSupport(const std::vector<const char*>& validation_layers);
    
    // Get required extensions for validation
    static std::vector<const char*> GetRequiredExtensions(bool enable_validation_layers);
    
    // Check if device extensions are supported
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device,
                                           const std::vector<const char*>& required_extensions);
    
    // Validation layer debug callback
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* p_user_data);

    // Create debug messenger create info
    static VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo();
    
    // Setup debug messenger
    static VkResult CreateDebugUtilsMessengerExt(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
                                                 const VkAllocationCallbacks* p_allocator,
                                                 VkDebugUtilsMessengerEXT* p_debug_messenger);
    
    // Cleanup debug messenger
    static void DestroyDebugUtilsMessengerExt(VkInstance instance,
                                             VkDebugUtilsMessengerEXT debug_messenger,
                                             const VkAllocationCallbacks* p_allocator);
};

// Error handling utilities
class ErrorUtils {
public:
    // Throw exception with Vulkan result
    static void CheckResult(VkResult result, const std::string& operation);
    
    // Check result and return bool
    static bool IsSuccess(VkResult result);
    
    // Check if result is a recoverable error
    static bool IsRecoverableError(VkResult result);
    
    // Get error description
    static std::string GetErrorDescription(VkResult result);
    
    // Check and throw on error
    static void ThrowOnError(VkResult result, const std::string& operation);
};

} // namespace VulkanEngine::RAII::Utils



#endif // VULKAN_RAII_UTILS_DEBUG_UTILS_HPP