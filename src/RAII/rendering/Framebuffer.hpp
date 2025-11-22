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

    [[nodiscard]] VkFramebuffer GetHandle() const { return framebuffer_; }
    
    // Implicit conversion to VkFramebuffer
    operator VkFramebuffer() const { return framebuffer_; }

    // Check if the framebuffer is valid
    [[nodiscard]] bool IsValid() const { return framebuffer_ != VK_NULL_HANDLE; }

    // Get framebuffer dimensions
    [[nodiscard]] uint32_t GetWidth() const { return width_; }
    [[nodiscard]] uint32_t GetHeight() const { return height_; }
    [[nodiscard]] uint32_t GetLayers() const { return layers_; }

    // Get attachment count
    [[nodiscard]] uint32_t GetAttachmentCount() const { return static_cast<uint32_t>(attachments_.size()); }

    // Get attachments
    [[nodiscard]] const std::vector<VkImageView>& GetAttachments() const { return attachments_; }

private:
    VkFramebuffer framebuffer_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    VkRenderPass renderPass_{VK_NULL_HANDLE}; // Reference to render pass

    std::vector<VkImageView> attachments_;
    uint32_t width_{0};
    uint32_t height_{0};
    uint32_t layers_{1};

    // Helper methods
    void CreateFramebuffer();
    void Cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_FRAMEBUFFER_HPP