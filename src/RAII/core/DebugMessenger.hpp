#ifndef VULKAN_RAII_CORE_DEBUG_MESSENGER_HPP
#define VULKAN_RAII_CORE_DEBUG_MESSENGER_HPP

#include <volk.h>
#include <iostream>


namespace VulkanEngine::RAII {

class Instance; // Forward declaration

class DebugMessenger {
public:
    // Constructor that creates a debug messenger for the given instance
    explicit DebugMessenger(const Instance& instance,
                           VkDebugUtilsMessageSeverityFlagsEXT message_severity = 
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                           VkDebugUtilsMessageTypeFlagsEXT message_type = 
                               VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);

    // Destructor
    ~DebugMessenger();

    // Move constructor and assignment
    DebugMessenger(DebugMessenger&& other) noexcept;
    DebugMessenger& operator=(DebugMessenger&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the DebugMessenger by only allowing moving.
    DebugMessenger(const DebugMessenger&) = delete;
    DebugMessenger& operator=(const DebugMessenger&) = delete;

    [[nodiscard]] VkDebugUtilsMessengerEXT get_handle() const { return debugMessenger_; }
    
    // Implicit conversion to VkDebugUtilsMessengerEXT
    operator VkDebugUtilsMessengerEXT() const { return debugMessenger_; }

    // Check if the debug messenger is valid
    [[nodiscard]] bool is_valid() const { return debugMessenger_ != VK_NULL_HANDLE; }

    // Static debug callback function
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* p_user_data);

    // Get debug messenger create info (useful for instance creation)
    static VkDebugUtilsMessengerCreateInfoEXT get_create_info(
        VkDebugUtilsMessageSeverityFlagsEXT message_severity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        VkDebugUtilsMessageTypeFlagsEXT message_type = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);

private:
    VkDebugUtilsMessengerEXT debugMessenger_{VK_NULL_HANDLE};
    VkInstance instance_{VK_NULL_HANDLE}; // Reference to the instance for cleanup

    // Helper methods
    void create_debug_messenger(const Instance& instance,
                             VkDebugUtilsMessageSeverityFlagsEXT message_severity,
                             VkDebugUtilsMessageTypeFlagsEXT message_type);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_CORE_DEBUG_MESSENGER_HPP