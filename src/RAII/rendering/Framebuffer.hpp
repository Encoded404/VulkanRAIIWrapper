#ifndef VULKAN_RAII_RENDERING_FRAMEBUFFER_HPP
#define VULKAN_RAII_RENDERING_FRAMEBUFFER_HPP

#include <volk.h>
#include <vector>

namespace VulkanEngine::RAII {

class Device; // Forward declaration
class RenderPass; // Forward declaration

class Framebuffer {
public:
    // Constructor that creates a framebuffer for the given render pass and attachments
    Framebuffer(const Device& device,
                const RenderPass& render_pass,
                const std::vector<VkImageView>& attachments,
                uint32_t width,
                uint32_t height,
                uint32_t layers = 1);

    // Destructor
    ~Framebuffer();

    // Move constructor and assignment
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkFramebuffer by only allowing moving.
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    [[nodiscard]] VkFramebuffer get_handle() const { return framebuffer_; }
    
    // Implicit conversion to VkFramebuffer
    operator VkFramebuffer() const { return framebuffer_; }

    // Check if the framebuffer is valid
    [[nodiscard]] bool is_valid() const { return framebuffer_ != VK_NULL_HANDLE; }

    // Get framebuffer dimensions
    [[nodiscard]] uint32_t get_width() const { return width_; }
    [[nodiscard]] uint32_t get_height() const { return height_; }
    [[nodiscard]] uint32_t get_layers() const { return layers_; }

    // Get attachment count
    [[nodiscard]] uint32_t get_attachment_count() const { return static_cast<uint32_t>(attachments_.size()); }

    // Get attachments
    [[nodiscard]] const std::vector<VkImageView>& get_attachments() const { return attachments_; }

private:
    VkFramebuffer framebuffer_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    VkRenderPass renderPass_{VK_NULL_HANDLE}; // Reference to render pass

    std::vector<VkImageView> attachments_;
    uint32_t width_{0};
    uint32_t height_{0};
    uint32_t layers_{1};

    // Helper methods
    void create_framebuffer();
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_FRAMEBUFFER_HPP