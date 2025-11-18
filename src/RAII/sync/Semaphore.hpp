#ifndef VULKAN_RAII_SYNC_SEMAPHORE_HPP
#define VULKAN_RAII_SYNC_SEMAPHORE_HPP

#include <volk.h>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

class Semaphore {
public:
    // Constructor that creates a semaphore
    explicit Semaphore(const Device& device, VkSemaphoreCreateFlags flags = 0);

    // Constructor for timeline semaphore
    Semaphore(const Device& device, uint64_t initial_value, VkSemaphoreCreateFlags flags = 0);

    // Destructor
    ~Semaphore();

    // Move constructor and assignment
    Semaphore(Semaphore&& other) noexcept;
    Semaphore& operator=(Semaphore&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkSemaphore by only allowing moving.
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    [[nodiscard]] VkSemaphore get_handle() const { return semaphore_; }
    
    // Implicit conversion to VkSemaphore
    operator VkSemaphore() const { return semaphore_; }

    // Check if the semaphore is valid
    [[nodiscard]] bool is_valid() const { return semaphore_ != VK_NULL_HANDLE; }

    // Timeline semaphore operations (if supported)
    [[nodiscard]] bool is_timeline_semaphore() const { return isTimelineSemaphore_; }

    // Get counter value (timeline semaphore only)
    [[nodiscard]] uint64_t get_counter_value() const;

    // Wait for timeline semaphore to reach value
    [[nodiscard]] VkResult wait(uint64_t value, uint64_t timeout = UINT64_MAX) const;

    // Signal timeline semaphore to value
    [[nodiscard]] VkResult signal(uint64_t value) const;

    // Wait for multiple timeline semaphores
    static VkResult wait_semaphores(const Device& device,
                                  const std::vector<VkSemaphore>& semaphores,
                                  const std::vector<uint64_t>& values,
                                  bool wait_all = true,
                                  uint64_t timeout = UINT64_MAX);

    // Signal multiple timeline semaphores
    static VkResult signal_semaphores(const Device& device,
                                    const std::vector<VkSemaphore>& semaphores,
                                    const std::vector<uint64_t>& values);

private:
    VkSemaphore semaphore_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    bool isTimelineSemaphore_{false};

    // Helper methods
    void create_semaphore(VkSemaphoreCreateFlags flags, uint64_t* initial_value = nullptr);
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_SYNC_SEMAPHORE_HPP