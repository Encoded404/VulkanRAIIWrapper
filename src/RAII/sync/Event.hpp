#ifndef VULKAN_RAII_SYNC_EVENT_HPP
#define VULKAN_RAII_SYNC_EVENT_HPP

#include <volk.h>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

class Event {
public:
    // Constructor that creates an event
    explicit Event(const Device& device, VkEventCreateFlags flags = 0);

    // Destructor
    ~Event();

    // Move constructor and assignment
    Event(Event&& other) noexcept;
    Event& operator=(Event&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkEvent by only allowing moving.
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    [[nodiscard]] VkEvent GetHandle() const { return event_; }
    
    // Implicit conversion to VkEvent
    operator VkEvent() const { return event_; }

    // Check if the event is valid
    [[nodiscard]] bool IsValid() const { return event_ != VK_NULL_HANDLE; }

    // Set event to signaled state
    [[nodiscard]] VkResult Set() const;

    // Reset event to unsignaled state
    [[nodiscard]] VkResult Reset() const;

    // Get event status (signaled or unsignaled)
    [[nodiscard]] VkResult GetStatus() const;

    // Check if event is signaled
    [[nodiscard]] bool IsSignaled() const;

    // Check if event is unsignaled
    [[nodiscard]] bool IsUnsignaled() const;

private:
    VkEvent event_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device

    // Helper methods
    void CreateEvent(VkEventCreateFlags flags);
    void Cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_SYNC_EVENT_HPP