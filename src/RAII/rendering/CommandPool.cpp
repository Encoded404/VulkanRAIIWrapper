#include "CommandPool.hpp"

#include "../core/Device.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace VulkanEngine::RAII {

CommandPool::CommandPool(const Device& device,
                         uint32_t queue_family_index,
                         VkCommandPoolCreateFlags flags)
    : device_(device.GetHandle()), queueFamilyIndex_(queue_family_index) {
    CreateCommandPool(flags);
}

CommandPool::~CommandPool() {
    Cleanup();
}

CommandPool::CommandPool(CommandPool&& other) noexcept
    : commandPool_(other.commandPool_),
      device_(other.device_),
      queueFamilyIndex_(other.queueFamilyIndex_) {
    other.commandPool_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.queueFamilyIndex_ = 0;
}

CommandPool& CommandPool::operator=(CommandPool&& other) noexcept {
    if (this != &other) {
        Cleanup();
        commandPool_ = other.commandPool_;
        device_ = other.device_;
        queueFamilyIndex_ = other.queueFamilyIndex_;
        other.commandPool_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.queueFamilyIndex_ = 0;
    }
    return *this;
}

std::vector<VkCommandBuffer> CommandPool::AllocateCommandBuffers(uint32_t count,
                                                                 VkCommandBufferLevel level) const {
    VkCommandBufferAllocateInfo alloc_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc_info.commandPool = commandPool_;
    alloc_info.level = level;
    alloc_info.commandBufferCount = count;

    std::vector<VkCommandBuffer> command_buffers(count);
    if (vkAllocateCommandBuffers(device_, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }
    return command_buffers;
}

VkCommandBuffer CommandPool::AllocateCommandBuffer(VkCommandBufferLevel level) const {
    auto buffers = AllocateCommandBuffers(1, level);
    return buffers.front();
}

void CommandPool::FreeCommandBuffers(const std::vector<VkCommandBuffer>& command_buffers) const {
    if (!command_buffers.empty()) {
        vkFreeCommandBuffers(device_, commandPool_, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
    }
}

void CommandPool::FreeCommandBuffer(VkCommandBuffer command_buffer) const {
    if (command_buffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(device_, commandPool_, 1, &command_buffer);
    }
}

void CommandPool::Reset(VkCommandPoolResetFlags flags) const {
    vkResetCommandPool(device_, commandPool_, flags);
}

void CommandPool::Trim(VkCommandPoolTrimFlags flags) const {
    if (vkTrimCommandPool) {
        vkTrimCommandPool(device_, commandPool_, flags);
    }
}

void CommandPool::CreateCommandPool(VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_info.queueFamilyIndex = queueFamilyIndex_;
    pool_info.flags = flags;

    if (vkCreateCommandPool(device_, &pool_info, nullptr, &commandPool_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

void CommandPool::Cleanup() {
    if (commandPool_ != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_, commandPool_, nullptr);
        commandPool_ = VK_NULL_HANDLE;
    }
}

} // namespace VulkanEngine::RAII

