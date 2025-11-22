#ifndef VULKAN_RAII_CORE_QUEUE_HPP
#define VULKAN_RAII_CORE_QUEUE_HPP

#include <volk.h>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

enum class QueueType {
    GRAPHICS,
    PRESENT,
    COMPUTE,
    TRANSFER
};

class Queue {
public:
    // Constructor that wraps a VkQueue retrieved from a device
    Queue(VkQueue queue, uint32_t family_index, QueueType type);

    // Default constructor
    Queue() = default;

    // Move constructor and assignment
    Queue(Queue&& other) noexcept;
    Queue& operator=(Queue&& other) noexcept;

    // Copy constructor and assignment (copy allowed for Queue)
    Queue(const Queue&) = default;
    Queue& operator=(const Queue&) = default;

    [[nodiscard]] VkQueue GetHandle() const { return queue_; }
    
    // Implicit conversion to VkQueue
    operator VkQueue() const { return queue_; }

    // Check if the queue is valid
    [[nodiscard]] bool IsValid() const { return queue_ != VK_NULL_HANDLE; }

    // Get queue family index
    [[nodiscard]] uint32_t GetFamilyIndex() const { return familyIndex_; }

    // Get queue type
    [[nodiscard]] QueueType GetType() const { return type_; }

    // Submit command buffers to the queue
    VkResult Submit(const std::vector<VkCommandBuffer>& command_buffers,
                   const std::vector<VkSemaphore>& wait_semaphores = {},
                   const std::vector<VkPipelineStageFlags>& wait_stages = {},
                   const std::vector<VkSemaphore>& signal_semaphores = {},
                   VkFence fence = VK_NULL_HANDLE) const;

    // Submit a single command buffer
    VkResult Submit(VkCommandBuffer command_buffer,
                   VkSemaphore wait_semaphore = VK_NULL_HANDLE,
                   VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VkSemaphore signal_semaphore = VK_NULL_HANDLE,
                   VkFence fence = VK_NULL_HANDLE) const;

    // Present swap chain images (only valid for present queues)
    [[nodiscard]] VkResult Present(const std::vector<VkSwapchainKHR>& swap_chains,
                    const std::vector<uint32_t>& image_indices,
                    const std::vector<VkSemaphore>& wait_semaphores = {}) const;

    // Present single swap chain image
    VkResult Present(VkSwapchainKHR swap_chain,
                    uint32_t image_index,
                    VkSemaphore wait_semaphore = VK_NULL_HANDLE) const;

    // Wait for queue to be idle
    [[nodiscard]] VkResult WaitIdle() const;

    // Bind sparse buffer memory
    VkResult BindSparse(const std::vector<VkBindSparseInfo>& bind_info,
                       VkFence fence = VK_NULL_HANDLE) const;

    // Get queue capabilities based on family properties
    static VkQueueFlags GetQueueCapabilities(const Device& device, uint32_t queue_family_index);

    // Check if queue supports specific operations
    [[nodiscard]] bool SupportsGraphics() const;
    [[nodiscard]] bool SupportsCompute() const;
    [[nodiscard]] bool SupportsTransfer() const;

private:
    VkQueue queue_{VK_NULL_HANDLE};
    uint32_t familyIndex_{0};
    QueueType type_{QueueType::GRAPHICS};
    VkQueueFlags capabilities_{0}; // Cached queue capabilities

    // Helper methods
    [[nodiscard]] VkSubmitInfo CreateSubmitInfo(const std::vector<VkCommandBuffer>& command_buffers,
                                 const std::vector<VkSemaphore>& wait_semaphores,
                                 const std::vector<VkPipelineStageFlags>& wait_stages,
                                 const std::vector<VkSemaphore>& signal_semaphores) const;

    [[nodiscard]] VkPresentInfoKHR CreatePresentInfo(const std::vector<VkSwapchainKHR>& swap_chains,
                                      const std::vector<uint32_t>& image_indices,
                                      const std::vector<VkSemaphore>& wait_semaphores) const;
};

// Queue manager class to handle multiple queues
class QueueManager {
public:
    explicit QueueManager(const Device& device);

    // Get queues by type
    [[nodiscard]] const Queue& GetGraphicsQueue() const { return graphicsQueue_; }
    [[nodiscard]] const Queue& GetPresentQueue() const { return presentQueue_; }
    [[nodiscard]] const Queue& GetComputeQueue() const { return computeQueue_; }
    [[nodiscard]] const Queue& GetTransferQueue() const { return transferQueue_; }

    // Check if specific queue types are available
    [[nodiscard]] bool HasGraphicsQueue() const { return graphicsQueue_.IsValid(); }
    [[nodiscard]] bool HasPresentQueue() const { return presentQueue_.IsValid(); }
    [[nodiscard]] bool HasComputeQueue() const { return computeQueue_.IsValid(); }
    [[nodiscard]] bool HasTransferQueue() const { return transferQueue_.IsValid(); }

private:
    Queue graphicsQueue_;
    Queue presentQueue_;
    Queue computeQueue_;
    Queue transferQueue_;

    void InitializeQueues(const Device& device);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_CORE_QUEUE_HPP