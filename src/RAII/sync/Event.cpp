#include "Event.hpp"

#include "../core/Device.hpp"
#include <stdexcept>


namespace VulkanEngine::RAII {

Event::Event(const Device& device, VkEventCreateFlags flags)
    : device_(device.GetHandle()) {
    CreateEvent(flags);
}

Event::~Event() {
    Cleanup();
}

Event::Event(Event&& other) noexcept
    : event_(other.event_),
      device_(other.device_) {
    other.event_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

Event& Event::operator=(Event&& other) noexcept {
    if (this != &other) {
        Cleanup();
        event_ = other.event_;
        device_ = other.device_;
        other.event_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

VkResult Event::Set() const {
    return vkSetEvent(device_, event_);
}

VkResult Event::Reset() const {
    return vkResetEvent(device_, event_);
}

VkResult Event::GetStatus() const {
    return vkGetEventStatus(device_, event_);
}

bool Event::IsSignaled() const {
    return GetStatus() == VK_EVENT_SET;
}

bool Event::IsUnsignaled() const {
    return GetStatus() == VK_EVENT_RESET;
}

void Event::CreateEvent(VkEventCreateFlags flags) {
    VkEventCreateInfo event_info{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
    event_info.flags = flags;

    if (vkCreateEvent(device_, &event_info, nullptr, &event_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create event");
    }
}

void Event::Cleanup() {
    if (event_ != VK_NULL_HANDLE) {
        vkDestroyEvent(device_, event_, nullptr);
        event_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

