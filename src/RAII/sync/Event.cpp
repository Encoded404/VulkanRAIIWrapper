#include "Event.hpp"

#include "../core/Device.hpp"
#include <stdexcept>


namespace VulkanEngine::RAII {

Event::Event(const Device& device, VkEventCreateFlags flags)
    : device_(device.get_handle()) {
    create_event(flags);
}

Event::~Event() {
    cleanup();
}

Event::Event(Event&& other) noexcept
    : event_(other.event_),
      device_(other.device_) {
    other.event_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

Event& Event::operator=(Event&& other) noexcept {
    if (this != &other) {
        cleanup();
        event_ = other.event_;
        device_ = other.device_;
        other.event_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

VkResult Event::set() const {
    return vkSetEvent(device_, event_);
}

VkResult Event::reset() const {
    return vkResetEvent(device_, event_);
}

VkResult Event::get_status() const {
    return vkGetEventStatus(device_, event_);
}

bool Event::is_signaled() const {
    return get_status() == VK_EVENT_SET;
}

bool Event::is_unsignaled() const {
    return get_status() == VK_EVENT_RESET;
}

void Event::create_event(VkEventCreateFlags flags) {
    VkEventCreateInfo event_info{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
    event_info.flags = flags;

    if (vkCreateEvent(device_, &event_info, nullptr, &event_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create event");
    }
}

void Event::cleanup() {
    if (event_ != VK_NULL_HANDLE) {
        vkDestroyEvent(device_, event_, nullptr);
        event_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

