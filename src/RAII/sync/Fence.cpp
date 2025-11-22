#include "Fence.hpp"

#include "../core/Device.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>


namespace VulkanEngine::RAII {

Fence::Fence(const Device& device, VkFenceCreateFlags flags)
    : device_(device.GetHandle()) {
    CreateFence(flags);
}

Fence::~Fence() {
    Cleanup();
}

Fence::Fence(Fence&& other) noexcept
    : fence_(other.fence_),
      device_(other.device_) {
    other.fence_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

Fence& Fence::operator=(Fence&& other) noexcept {
    if (this != &other) {
        Cleanup();
        fence_ = other.fence_;
        device_ = other.device_;
        other.fence_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

VkResult Fence::Wait(uint64_t timeout) const {
    return vkWaitForFences(device_, 1, &fence_, VK_TRUE, timeout);
}

VkResult Fence::GetStatus() const {
    return vkGetFenceStatus(device_, fence_);
}

VkResult Fence::Reset() const {
    return vkResetFences(device_, 1, &fence_);
}

bool Fence::IsSignaled() const {
    return GetStatus() == VK_SUCCESS;
}

VkResult Fence::WaitForFences(const Device& device,
                              const std::vector<VkFence>& fences,
                              bool wait_all,
                              uint64_t timeout) {
    return vkWaitForFences(device.GetHandle(),
                           static_cast<uint32_t>(fences.size()),
                           fences.data(),
                           wait_all ? VK_TRUE : VK_FALSE,
                           timeout);
}

VkResult Fence::ResetFences(const Device& device,
                            const std::vector<VkFence>& fences) {
    return vkResetFences(device.GetHandle(),
                         static_cast<uint32_t>(fences.size()),
                         fences.data());
}

VkResult Fence::WaitAndReset(uint64_t timeout) {
    VkResult result = Wait(timeout);
    if (result == VK_SUCCESS) {
        VkResult reset_result = Reset();
        if (reset_result != VK_SUCCESS) {
            return reset_result;
        }
    }
    return result;
}

void Fence::CreateFence(VkFenceCreateFlags flags) {
    VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fence_info.flags = flags;

    if (vkCreateFence(device_, &fence_info, nullptr, &fence_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence");
    }
}

void Fence::Cleanup() {
    if (fence_ != VK_NULL_HANDLE) {
        vkDestroyFence(device_, fence_, nullptr);
        fence_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

