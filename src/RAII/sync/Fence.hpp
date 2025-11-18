#ifndef VULKAN_RAII_SYNC_FENCE_HPP
#define VULKAN_RAII_SYNC_FENCE_HPP

#include <volk.h>
#include <cstdint>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

class Fence {
public:
    // Constructor that creates a fence
    explicit Fence(const Device& device, VkFenceCreateFlags flags = 0);

    // Destructor
    ~Fence();

    // Move constructor and assignment
    Fence(Fence&& other) noexcept;
    Fence& operator=(Fence&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkFence by only allowing moving.
    Fence(const Fence&) = delete;
    Fence& operator=(const Fence&) = delete;

    [[nodiscard]] VkFence get_handle() const { return fence_; }
    
    // Implicit conversion to VkFence
    operator VkFence() const { return fence_; }

    // Check if the fence is valid
    [[nodiscard]] bool is_valid() const { return fence_ != VK_NULL_HANDLE; }

    // Wait for fence to be signaled
    VkResult wait(uint64_t timeout = UINT64_MAX) const; // NOLINT(modernize-use-nodiscard)

    // Check if fence is signaled (non-blocking)
    [[nodiscard]] VkResult get_status() const;

    // Reset fence to unsignaled state
    VkResult reset() const; // NOLINT(modernize-use-nodiscard)

    // Check if fence is currently signaled
    [[nodiscard]] bool is_signaled() const;

    // Static utility functions for multiple fences
    static VkResult wait_for_fences(const Device& device,
                                 const std::vector<VkFence>& fences,
                                 bool wait_all = true,
                                 uint64_t timeout = UINT64_MAX);

    static VkResult reset_fences(const Device& device,
                               const std::vector<VkFence>& fences);

    // Helper to wait for this fence and reset it
    VkResult wait_and_reset(uint64_t timeout = UINT64_MAX);

private:
    VkFence fence_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device

    // Helper methods
    void create_fence(VkFenceCreateFlags flags);
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_SYNC_FENCE_HPP