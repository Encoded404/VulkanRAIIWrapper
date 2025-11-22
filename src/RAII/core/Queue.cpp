#include "Queue.hpp"

#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "types/QueueFamilyIndices.hpp"

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace VulkanEngine::RAII {

Queue::Queue(VkQueue queue, uint32_t family_index, QueueType type)
    : queue_(queue), familyIndex_(family_index), type_(type) {
    switch (type) {
        case QueueType::GRAPHICS:
            capabilities_ = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            break;
        case QueueType::PRESENT:
            capabilities_ = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
            break;
        case QueueType::COMPUTE:
            capabilities_ = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            break;
        case QueueType::TRANSFER:
            capabilities_ = VK_QUEUE_TRANSFER_BIT;
            break;
    }
}

Queue::Queue(Queue&& other) noexcept
    : queue_(other.queue_),
      familyIndex_(other.familyIndex_),
      type_(other.type_),
      capabilities_(other.capabilities_) {
    other.queue_ = VK_NULL_HANDLE;
    other.capabilities_ = 0;
}

Queue& Queue::operator=(Queue&& other) noexcept {
    if (this != &other) {
        queue_ = other.queue_;
        familyIndex_ = other.familyIndex_;
        type_ = other.type_;
        capabilities_ = other.capabilities_;
        other.queue_ = VK_NULL_HANDLE;
        other.capabilities_ = 0;
    }
    return *this;
}

VkResult Queue::Submit(const std::vector<VkCommandBuffer>& command_buffers,
                       const std::vector<VkSemaphore>& wait_semaphores,
                       const std::vector<VkPipelineStageFlags>& wait_stages,
                       const std::vector<VkSemaphore>& signal_semaphores,
                       VkFence fence) const {
    if (wait_semaphores.size() != wait_stages.size()) {
        throw std::runtime_error("Wait semaphores and stage masks size mismatch");
    }

    VkSubmitInfo submit_info = CreateSubmitInfo(command_buffers, wait_semaphores, wait_stages, signal_semaphores);
    return vkQueueSubmit(queue_, 1, &submit_info, fence);
}

VkResult Queue::Submit(VkCommandBuffer command_buffer,
                       VkSemaphore wait_semaphore,
                       VkPipelineStageFlags wait_stage,
                       VkSemaphore signal_semaphore,
                       VkFence fence) const {
    std::vector<VkCommandBuffer> command_buffers{command_buffer};
    std::vector<VkSemaphore> wait_semaphores;
    std::vector<VkPipelineStageFlags> wait_stages;
    std::vector<VkSemaphore> signal_semaphores;

    if (wait_semaphore != VK_NULL_HANDLE) {
        wait_semaphores.push_back(wait_semaphore);
        wait_stages.push_back(wait_stage);
    }
    if (signal_semaphore != VK_NULL_HANDLE) {
        signal_semaphores.push_back(signal_semaphore);
    }

    return Submit(command_buffers, wait_semaphores, wait_stages, signal_semaphores, fence);
}

VkResult Queue::Present(const std::vector<VkSwapchainKHR>& swap_chains,
                        const std::vector<uint32_t>& image_indices,
                        const std::vector<VkSemaphore>& wait_semaphores) const {
    if (swap_chains.size() != image_indices.size()) {
        throw std::runtime_error("Swapchain and image index size mismatch");
    }

    VkPresentInfoKHR present_info = CreatePresentInfo(swap_chains, image_indices, wait_semaphores);
    return vkQueuePresentKHR(queue_, &present_info);
}

VkResult Queue::Present(VkSwapchainKHR swap_chain,
                        uint32_t image_index,
                        VkSemaphore wait_semaphore) const {
    std::vector<VkSwapchainKHR> swap_chains{swap_chain};
    std::vector<uint32_t> image_indices{image_index};
    std::vector<VkSemaphore> wait_semaphores;
    if (wait_semaphore != VK_NULL_HANDLE) {
        wait_semaphores.push_back(wait_semaphore);
    }
    return Present(swap_chains, image_indices, wait_semaphores);
}

VkResult Queue::WaitIdle() const {
    return vkQueueWaitIdle(queue_);
}

VkResult Queue::BindSparse(const std::vector<VkBindSparseInfo>& bind_info,
                           VkFence fence) const {
    return vkQueueBindSparse(queue_, static_cast<uint32_t>(bind_info.size()), bind_info.data(), fence);
}

VkQueueFlags Queue::GetQueueCapabilities(const Device& device, uint32_t queue_family_index) {
    auto properties = device.GetPhysicalDevice().GetQueueFamilyProperties();
    if (queue_family_index >= properties.size()) {
        return 0;
    }
    return properties[queue_family_index].queueFlags;
}

bool Queue::SupportsGraphics() const {
    return (capabilities_ & VK_QUEUE_GRAPHICS_BIT) != 0;
}

bool Queue::SupportsCompute() const {
    return (capabilities_ & VK_QUEUE_COMPUTE_BIT) != 0;
}

bool Queue::SupportsTransfer() const {
    return (capabilities_ & VK_QUEUE_TRANSFER_BIT) != 0;
}

VkSubmitInfo Queue::CreateSubmitInfo(const std::vector<VkCommandBuffer>& command_buffers,
                                     const std::vector<VkSemaphore>& wait_semaphores,
                                     const std::vector<VkPipelineStageFlags>& wait_stages,
                                     const std::vector<VkSemaphore>& signal_semaphores) const {
    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
    submit_info.pCommandBuffers = command_buffers.data();

    submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
    submit_info.pWaitSemaphores = wait_semaphores.empty() ? nullptr : wait_semaphores.data();
    submit_info.pWaitDstStageMask = wait_stages.empty() ? nullptr : wait_stages.data();

    submit_info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());
    submit_info.pSignalSemaphores = signal_semaphores.empty() ? nullptr : signal_semaphores.data();

    return submit_info;
}

VkPresentInfoKHR Queue::CreatePresentInfo(const std::vector<VkSwapchainKHR>& swap_chains,
                                          const std::vector<uint32_t>& image_indices,
                                          const std::vector<VkSemaphore>& wait_semaphores) const {
    VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
    present_info.pWaitSemaphores = wait_semaphores.empty() ? nullptr : wait_semaphores.data();
    present_info.swapchainCount = static_cast<uint32_t>(swap_chains.size());
    present_info.pSwapchains = swap_chains.data();
    present_info.pImageIndices = image_indices.data();
    present_info.pResults = nullptr;
    return present_info;
}

QueueManager::QueueManager(const Device& device) {
    InitializeQueues(device);
}

void QueueManager::InitializeQueues(const Device& device) {
    QueueFamilyIndices indices = device.GetQueueFamilyIndices();
    if (indices.graphicsFamily_.has_value()) {
        VkQueue queue = device.GetGraphicsQueue();
        graphicsQueue_ = Queue(queue, indices.graphicsFamily_.value(), QueueType::GRAPHICS);
    }

    if (indices.presentFamily_.has_value()) {
        VkQueue queue = device.GetPresentQueue();
        presentQueue_ = Queue(queue, indices.presentFamily_.value(), QueueType::PRESENT);
    }

    if (indices.computeFamily_.has_value()) {
        VkQueue queue = device.GetComputeQueue();
        computeQueue_ = Queue(queue, indices.computeFamily_.value(), QueueType::COMPUTE);
    }

    if (indices.transferFamily_.has_value()) {
        VkQueue queue = device.GetTransferQueue();
        transferQueue_ = Queue(queue, indices.transferFamily_.value(), QueueType::TRANSFER);
    }
}

} // namespace VulkanEngine::RAII

