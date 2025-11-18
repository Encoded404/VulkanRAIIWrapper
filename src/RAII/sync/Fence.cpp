#include "Fence.hpp"

#include "../core/Device.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>


namespace VulkanEngine::RAII {

Fence::Fence(const Device& device, VkFenceCreateFlags flags)
    : device_(device.get_handle()) {
    create_fence(flags);
}

Fence::~Fence() {
    cleanup();
}

Fence::Fence(Fence&& other) noexcept
    : fence_(other.fence_),
      device_(other.device_) {
    other.fence_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

Fence& Fence::operator=(Fence&& other) noexcept {
    if (this != &other) {
        cleanup();
        fence_ = other.fence_;
        device_ = other.device_;
        other.fence_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

VkResult Fence::wait(uint64_t timeout) const {
    return vkWaitForFences(device_, 1, &fence_, VK_TRUE, timeout);
}

VkResult Fence::get_status() const {
    return vkGetFenceStatus(device_, fence_);
}

VkResult Fence::reset() const {
    return vkResetFences(device_, 1, &fence_);
}

bool Fence::is_signaled() const {
    return get_status() == VK_SUCCESS;
}

VkResult Fence::wait_for_fences(const Device& device,
                              const std::vector<VkFence>& fences,
                              bool wait_all,
                              uint64_t timeout) {
    return vkWaitForFences(device.get_handle(),
                           static_cast<uint32_t>(fences.size()),
                           fences.data(),
                           wait_all ? VK_TRUE : VK_FALSE,
                           timeout);
}

VkResult Fence::reset_fences(const Device& device,
                            const std::vector<VkFence>& fences) {
    return vkResetFences(device.get_handle(),
                         static_cast<uint32_t>(fences.size()),
                         fences.data());
}

VkResult Fence::wait_and_reset(uint64_t timeout) {
    VkResult result = wait(timeout);
    if (result == VK_SUCCESS) {
        VkResult reset_result = reset();
        if (reset_result != VK_SUCCESS) {
            return reset_result;
        }
    }
    return result;
}

void Fence::create_fence(VkFenceCreateFlags flags) {
    VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fence_info.flags = flags;

    if (vkCreateFence(device_, &fence_info, nullptr, &fence_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence");
    }
}

void Fence::cleanup() {
    if (fence_ != VK_NULL_HANDLE) {
        vkDestroyFence(device_, fence_, nullptr);
        fence_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

