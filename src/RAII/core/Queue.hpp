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

    [[nodiscard]] VkQueue get_handle() const { return queue_; }
    
    // Implicit conversion to VkQueue
    operator VkQueue() const { return queue_; }

    // Check if the queue is valid
    [[nodiscard]] bool is_valid() const { return queue_ != VK_NULL_HANDLE; }

    // Get queue family index
    [[nodiscard]] uint32_t get_family_index() const { return familyIndex_; }

    // Get queue type
    [[nodiscard]] QueueType get_type() const { return type_; }

    // Submit command buffers to the queue
    VkResult submit(const std::vector<VkCommandBuffer>& command_buffers,
                   const std::vector<VkSemaphore>& wait_semaphores = {},
                   const std::vector<VkPipelineStageFlags>& wait_stages = {},
                   const std::vector<VkSemaphore>& signal_semaphores = {},
                   VkFence fence = VK_NULL_HANDLE) const;

    // Submit a single command buffer
    VkResult submit(VkCommandBuffer command_buffer,
                   VkSemaphore wait_semaphore = VK_NULL_HANDLE,
                   VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                   VkSemaphore signal_semaphore = VK_NULL_HANDLE,
                   VkFence fence = VK_NULL_HANDLE) const;

    // Present swap chain images (only valid for present queues)
    [[nodiscard]] VkResult present(const std::vector<VkSwapchainKHR>& swap_chains,
                    const std::vector<uint32_t>& image_indices,
                    const std::vector<VkSemaphore>& wait_semaphores = {}) const;

    // Present single swap chain image
    VkResult present(VkSwapchainKHR swap_chain,
                    uint32_t image_index,
                    VkSemaphore wait_semaphore = VK_NULL_HANDLE) const;

    // Wait for queue to be idle
    [[nodiscard]] VkResult wait_idle() const;

    // Bind sparse buffer memory
    VkResult bind_sparse(const std::vector<VkBindSparseInfo>& bind_info,
                       VkFence fence = VK_NULL_HANDLE) const;

    // Get queue capabilities based on family properties
    static VkQueueFlags get_queue_capabilities(const Device& device, uint32_t queue_family_index);

    // Check if queue supports specific operations
    [[nodiscard]] bool supports_graphics() const;
    [[nodiscard]] bool supports_compute() const;
    [[nodiscard]] bool supports_transfer() const;

private:
    VkQueue queue_{VK_NULL_HANDLE};
    uint32_t familyIndex_{0};
    QueueType type_{QueueType::GRAPHICS};
    VkQueueFlags capabilities_{0}; // Cached queue capabilities

    // Helper methods
    [[nodiscard]] VkSubmitInfo create_submit_info(const std::vector<VkCommandBuffer>& command_buffers,
                                 const std::vector<VkSemaphore>& wait_semaphores,
                                 const std::vector<VkPipelineStageFlags>& wait_stages,
                                 const std::vector<VkSemaphore>& signal_semaphores) const;

    [[nodiscard]] VkPresentInfoKHR create_present_info(const std::vector<VkSwapchainKHR>& swap_chains,
                                      const std::vector<uint32_t>& image_indices,
                                      const std::vector<VkSemaphore>& wait_semaphores) const;
};

// Queue manager class to handle multiple queues
class QueueManager {
public:
    explicit QueueManager(const Device& device);

    // Get queues by type
    [[nodiscard]] const Queue& get_graphics_queue() const { return graphicsQueue_; }
    [[nodiscard]] const Queue& get_present_queue() const { return presentQueue_; }
    [[nodiscard]] const Queue& get_compute_queue() const { return computeQueue_; }
    [[nodiscard]] const Queue& get_transfer_queue() const { return transferQueue_; }

    // Check if specific queue types are available
    [[nodiscard]] bool has_graphics_queue() const { return graphicsQueue_.is_valid(); }
    [[nodiscard]] bool has_present_queue() const { return presentQueue_.is_valid(); }
    [[nodiscard]] bool has_compute_queue() const { return computeQueue_.is_valid(); }
    [[nodiscard]] bool has_transfer_queue() const { return transferQueue_.is_valid(); }

private:
    Queue graphicsQueue_;
    Queue presentQueue_;
    Queue computeQueue_;
    Queue transferQueue_;

    void initialize_queues(const Device& device);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_CORE_QUEUE_HPP