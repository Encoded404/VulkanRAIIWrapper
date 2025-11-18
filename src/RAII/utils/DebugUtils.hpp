#ifndef VULKAN_RAII_UTILS_DEBUG_UTILS_HPP
#define VULKAN_RAII_UTILS_DEBUG_UTILS_HPP

#include <volk.h>
#include <string>
#include <vector>



namespace VulkanEngine::RAII::Utils {

// String conversion utilities for debugging
class StringUtils {
public:
    static std::string result_to_string(VkResult result);
    static std::string format_to_string(VkFormat format);
    static std::string present_mode_to_string(VkPresentModeKHR present_mode);
    static std::string color_space_to_string(VkColorSpaceKHR color_space);
    static std::string image_layout_to_string(VkImageLayout layout);
    static std::string memory_property_flags_to_string(VkMemoryPropertyFlags flags);
    static std::string buffer_usage_flags_to_string(VkBufferUsageFlags flags);
    static std::string image_usage_flags_to_string(VkImageUsageFlags flags);
    static std::string shader_stage_flags_to_string(VkShaderStageFlags flags);
    static std::string queue_flags_to_string(VkQueueFlags flags);
};

// Debug printing utilities
class DebugPrinter {
public:
    static void print_instance_extensions();
    static void print_instance_layers();
    static void print_physical_device_properties(VkPhysicalDevice device);
    static void print_physical_device_features(VkPhysicalDevice device);
    static void print_physical_device_memory_properties(VkPhysicalDevice device);
    static void print_queue_family_properties(VkPhysicalDevice device);
    static void print_surface_capabilities(VkPhysicalDevice device, VkSurfaceKHR surface);
    static void print_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface);
    static void print_device_extensions(VkPhysicalDevice device);
};

// Validation utilities
class ValidationUtils {
public:
    // Check if validation layers are available
    static bool check_validation_layer_support(const std::vector<const char*>& validation_layers);
    
    // Get required extensions for validation
    static std::vector<const char*> get_required_extensions(bool enable_validation_layers);
    
    // Check if device extensions are supported
    static bool check_device_extension_support(VkPhysicalDevice device,
                                           const std::vector<const char*>& required_extensions);
    
    // Validation layer debug callback
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* p_user_data);

    // Create debug messenger create info
    static VkDebugUtilsMessengerCreateInfoEXT create_debug_messenger_create_info();
    
    // Setup debug messenger
    static VkResult create_debug_utils_messenger_ext(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
                                                 const VkAllocationCallbacks* p_allocator,
                                                 VkDebugUtilsMessengerEXT* p_debug_messenger);
    
    // Cleanup debug messenger
    static void destroy_debug_utils_messenger_ext(VkInstance instance,
                                             VkDebugUtilsMessengerEXT debug_messenger,
                                             const VkAllocationCallbacks* p_allocator);
};

// Error handling utilities
class ErrorUtils {
public:
    // Throw exception with Vulkan result
    static void check_result(VkResult result, const std::string& operation);
    
    // Check result and return bool
    static bool is_success(VkResult result);
    
    // Check if result is a recoverable error
    static bool is_recoverable_error(VkResult result);
    
    // Get error description
    static std::string get_error_description(VkResult result);
    
    // Check and throw on error
    static void throw_on_error(VkResult result, const std::string& operation);
};

} // namespace VulkanEngine::RAII::Utils



#endif // VULKAN_RAII_UTILS_DEBUG_UTILS_HPP