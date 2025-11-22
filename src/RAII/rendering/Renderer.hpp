#ifndef VULKAN_RAII_RENDERING_RENDERER_HPP
#define VULKAN_RAII_RENDERING_RENDERER_HPP

#include <volk.h>
#include <vector>
#include <memory>

// Forward declare SDL types
struct SDL_Window;


namespace VulkanEngine::RAII {

class Device; // Forward declaration
class Swapchain; // Forward declaration
class RenderPass; // Forward declaration
class Framebuffer; // Forward declaration
class CommandPool; // Forward declaration
class CommandBuffer; // Forward declaration
class Semaphore; // Forward declaration
class Fence; // Forward declaration

class Renderer {
public:
    // Constructor
    Renderer(const Device& device,
             Swapchain& swapchain,
             const RenderPass& render_pass,
             uint32_t max_frames_in_flight = 2);

    // Destructor
    ~Renderer();

    // Move constructor and assignment
    Renderer(Renderer&& other) noexcept;
    Renderer& operator=(Renderer&& other) noexcept;

    // Delete copy constructor and assignment
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Begin frame rendering
    bool BeginFrame();

    // End frame rendering and present
    bool EndFrame();

    // Get current command buffer for recording
    CommandBuffer& GetCurrentCommandBuffer();

    // Get current frame index
    [[nodiscard]] uint32_t GetCurrentFrameIndex() const { return currentFrame_; }

    // Get current image index
    [[nodiscard]] uint32_t GetCurrentImageIndex() const { return imageIndex_; }

    // Check if frame is in progress
    [[nodiscard]] bool IsFrameInProgress() const { return frameInProgress_; }

    // Wait for all frames to complete
    void WaitIdle();

    // Supply additional image views (e.g., depth) to attach alongside the swapchain image per framebuffer.
    void SetExternalAttachments(std::vector<std::vector<VkImageView>> attachments);
    void ClearExternalAttachments();

    // Rebuild framebuffers to incorporate current attachments and swapchain state.
    void RebuildFramebuffers();

    // Recreate resources (for window resize)
    void Recreate(bool recreate_semaphores = true);

    // Get max frames in flight
    [[nodiscard]] uint32_t GetMaxFramesInFlight() const { return maxFramesInFlight_; }

    // SDL2-specific methods
    
    // Check if renderer needs to handle window resize
    [[nodiscard]] bool NeedsSwapchainRecreation() const { return needsSwapchainRecreation_; }
    
    // Mark that swapchain recreation is needed (e.g., after window resize)
    void MarkSwapchainForRecreation() { needsSwapchainRecreation_ = true; }
    
    // Clear swapchain recreation flag
    void ClearSwapchainRecreationFlag() { needsSwapchainRecreation_ = false; }

    std::vector<std::unique_ptr<Framebuffer>>& GetFramebuffers() { return framebuffers_; }

    [[nodiscard]] uint64_t GetTotalFrameCount() const { return totalFrameCount_; }

private:
    const Device* device_{nullptr};
    Swapchain* swapchain_{nullptr};
    const RenderPass* renderPass_{nullptr};

    uint32_t maxFramesInFlight_{2};
    uint32_t currentFrame_{0};
    uint32_t imageIndex_{0};
    bool frameInProgress_{false};
    bool needsSwapchainRecreation_{false};

    uint64_t totalFrameCount_{0};
    uint64_t lastRecreateTime_{0};

    // Per-frame resources
    std::vector<std::unique_ptr<CommandPool>> commandPools_;
    std::vector<std::unique_ptr<CommandBuffer>> commandBuffers_;
    std::vector<std::unique_ptr<Semaphore>> imageAvailableSemaphores_;
    std::vector<std::unique_ptr<Semaphore>> renderFinishedSemaphores_;
    std::vector<std::unique_ptr<Fence>> inFlightFences_;

    // Framebuffers for each swapchain image
    std::vector<std::unique_ptr<Framebuffer>> framebuffers_;
    std::vector<std::vector<VkImageView>> extraAttachments_;

    // Helper methods
    void CreateSyncObjects(uint32_t num_of_swapchain_images);
    void RecreateSemaphoreSyncObjects(uint32_t num_of_swapchain_images);
    void CreateCommandObjects();
    void CreateFramebuffers();
    void Cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_RENDERER_HPP