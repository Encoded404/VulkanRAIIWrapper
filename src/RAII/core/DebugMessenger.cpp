#include "DebugMessenger.hpp"

#include "instance.hpp"
#include <iostream>
#include <stdexcept>

namespace VulkanEngine::RAII {

DebugMessenger::DebugMessenger(const Instance& instance,
                               VkDebugUtilsMessageSeverityFlagsEXT message_severity,
                               VkDebugUtilsMessageTypeFlagsEXT message_type)
    : instance_(instance.get_handle()) {
    create_debug_messenger(instance, message_severity, message_type);
}

DebugMessenger::~DebugMessenger() {
    if (debugMessenger_ != VK_NULL_HANDLE) {
        vkDestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
        debugMessenger_ = VK_NULL_HANDLE;
    }
}

DebugMessenger::DebugMessenger(DebugMessenger&& other) noexcept
    : debugMessenger_(other.debugMessenger_), instance_(other.instance_) {
    other.debugMessenger_ = VK_NULL_HANDLE;
    other.instance_ = VK_NULL_HANDLE;
}

DebugMessenger& DebugMessenger::operator=(DebugMessenger&& other) noexcept {
    if (this != &other) {
        if (debugMessenger_ != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
        }
        debugMessenger_ = other.debugMessenger_;
        instance_ = other.instance_;
        other.debugMessenger_ = VK_NULL_HANDLE;
        other.instance_ = VK_NULL_HANDLE;
    }
    return *this;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* /*pUserData*/) {
    std::cerr << "[Vulkan Validation] " << p_callback_data->pMessage << '\n';
    (void)message_severity;
    (void)message_type;
    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT DebugMessenger::get_create_info(
    VkDebugUtilsMessageSeverityFlagsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type) {
    VkDebugUtilsMessengerCreateInfoEXT create_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    create_info.messageSeverity = message_severity;
    create_info.messageType = message_type;
    create_info.pfnUserCallback = debug_callback;
    create_info.pUserData = nullptr;
    return create_info;
}

void DebugMessenger::create_debug_messenger(const Instance& instance,
                                          VkDebugUtilsMessageSeverityFlagsEXT message_severity,
                                          VkDebugUtilsMessageTypeFlagsEXT message_type) {
    auto create_info = get_create_info(message_severity, message_type);
    VkResult result = vkCreateDebugUtilsMessengerEXT(instance.get_handle(), &create_info, nullptr, &debugMessenger_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create debug messenger");
    }
}

} // namespace VulkanEngine::RAII

