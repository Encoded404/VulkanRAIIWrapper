#ifndef VULKAN_RAII_RENDERING_RENDER_PASS_HPP
#define VULKAN_RAII_RENDERING_RENDER_PASS_HPP

#include <volk.h>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

struct AttachmentDescription {
    VkFormat format_;
    VkSampleCountFlagBits samples_ = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp loadOp_ = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp storeOp_ = VK_ATTACHMENT_STORE_OP_STORE;
    VkAttachmentLoadOp stencilLoadOp_ = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp stencilStoreOp_ = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkImageLayout initialLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout finalLayout_ = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentDescriptionFlags flags_ = 0;
};

struct SubpassDescription {
    VkPipelineBindPoint pipelineBindPoint_ = VK_PIPELINE_BIND_POINT_GRAPHICS;
    std::vector<VkAttachmentReference> inputAttachments_;
    std::vector<VkAttachmentReference> colorAttachments_;
    std::vector<VkAttachmentReference> resolveAttachments_;
    VkAttachmentReference depthStencilAttachment_{VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED};
    std::vector<uint32_t> preserveAttachments_;
    VkSubpassDescriptionFlags flags_ = 0;
};

struct SubpassDependency {
    uint32_t srcSubpass_ = VK_SUBPASS_EXTERNAL;
    uint32_t dstSubpass_ = 0;
    VkPipelineStageFlags srcStageMask_ = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkPipelineStageFlags dstStageMask_ = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkAccessFlags srcAccessMask_ = 0;
    VkAccessFlags dstAccessMask_ = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkDependencyFlags dependencyFlags_ = 0;
};

class RenderPass {
public:
    // Constructor that creates a render pass with detailed configuration
    RenderPass(const Device& device,
               const std::vector<AttachmentDescription>& attachments,
               const std::vector<SubpassDescription>& subpasses,
               const std::vector<SubpassDependency>& dependencies = {});

    // Constructor for simple single-subpass render pass
    RenderPass(const Device& device,
               VkFormat color_format,
               VkFormat depth_format = VK_FORMAT_UNDEFINED,
               VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
               VkAttachmentLoadOp color_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
               VkAttachmentLoadOp depth_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR);

    // Destructor
    ~RenderPass();

    // Move constructor and assignment
    RenderPass(RenderPass&& other) noexcept;
    RenderPass& operator=(RenderPass&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkRenderPass by only allowing moving.
    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;

    [[nodiscard]] VkRenderPass get_handle() const { return renderPass_; }
    
    // Implicit conversion to VkRenderPass
    operator VkRenderPass() const { return renderPass_; }

    // Check if the render pass is valid
    [[nodiscard]] bool is_valid() const { return renderPass_ != VK_NULL_HANDLE; }

    // Get attachment count
    [[nodiscard]] uint32_t get_attachment_count() const { return static_cast<uint32_t>(attachments_.size()); }

    // Get subpass count
    [[nodiscard]] uint32_t get_subpass_count() const { return static_cast<uint32_t>(subpasses_.size()); }

    // Get attachment descriptions
    [[nodiscard]] const std::vector<AttachmentDescription>& get_attachments() const { return attachments_; }

    // Get subpass descriptions
    [[nodiscard]] const std::vector<SubpassDescription>& get_subpasses() const { return subpasses_; }

    // Get dependencies
    [[nodiscard]] const std::vector<SubpassDependency>& get_dependencies() const { return dependencies_; }

    // Helper to create simple color attachment reference
    static VkAttachmentReference create_color_attachment_ref(uint32_t attachment,
                                                         VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Helper to create depth attachment reference
    static VkAttachmentReference create_depth_attachment_ref(uint32_t attachment,
                                                         VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

private:
    VkRenderPass renderPass_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device

    std::vector<AttachmentDescription> attachments_;
    std::vector<SubpassDescription> subpasses_;
    std::vector<SubpassDependency> dependencies_;

    // Helper methods
    void create_render_pass();
    void create_simple_render_pass(VkFormat color_format, VkFormat depth_format, 
                               VkSampleCountFlagBits samples,
                               VkAttachmentLoadOp color_load_op, VkAttachmentLoadOp depth_load_op);
    void cleanup();
    
    // Conversion helpers
    [[nodiscard]] std::vector<VkAttachmentDescription> convert_attachments() const;
    [[nodiscard]] std::vector<VkSubpassDescription> convert_subpasses() const;
    [[nodiscard]] std::vector<VkSubpassDependency> convert_dependencies() const;
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_RENDER_PASS_HPP