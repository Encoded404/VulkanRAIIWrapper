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
    bool begin_frame();

    // End frame rendering and present
    bool end_frame();

    // Get current command buffer for recording
    CommandBuffer& get_current_command_buffer();

    // Get current frame index
    [[nodiscard]] uint32_t get_current_frame_index() const { return currentFrame_; }

    // Get current image index
    [[nodiscard]] uint32_t get_current_image_index() const { return imageIndex_; }

    // Check if frame is in progress
    [[nodiscard]] bool is_frame_in_progress() const { return frameInProgress_; }

    // Wait for all frames to complete
    void wait_idle();

    // Recreate resources (for window resize)
    void recreate(bool recreate_semaphores = true);

    // Get max frames in flight
    [[nodiscard]] uint32_t get_max_frames_in_flight() const { return maxFramesInFlight_; }

    // SDL2-specific methods
    
    // Check if renderer needs to handle window resize
    [[nodiscard]] bool needs_swapchain_recreation() const { return needsSwapchainRecreation_; }
    
    // Mark that swapchain recreation is needed (e.g., after window resize)
    void mark_swapchain_for_recreation() { needsSwapchainRecreation_ = true; }
    
    // Clear swapchain recreation flag
    void clear_swapchain_recreation_flag() { needsSwapchainRecreation_ = false; }

    std::vector<std::unique_ptr<Framebuffer>>& get_framebuffers() { return framebuffers_; }

    [[nodiscard]] uint64_t get_total_frame_count() const { return totalFrameCount_; }

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

    // Helper methods
    void create_sync_objects(uint32_t num_of_swapchain_images);
    void recreate_semaphore_sync_objects(uint32_t num_of_swapchain_images);
    void create_command_objects();
    void create_framebuffers();
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_RENDERER_HPP