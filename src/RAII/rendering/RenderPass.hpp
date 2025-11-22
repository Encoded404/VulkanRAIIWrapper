#ifndef VULKAN_RAII_RENDERING_RENDER_PASS_HPP
#define VULKAN_RAII_RENDERING_RENDER_PASS_HPP

#include <volk.h>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

struct AttachmentDescription {
    VkFormat format;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentDescriptionFlags flags = 0;
};

struct SubpassDescription {
    VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    std::vector<VkAttachmentReference> inputAttachments;
    std::vector<VkAttachmentReference> colorAttachments;
    std::vector<VkAttachmentReference> resolveAttachments;
    VkAttachmentReference depthStencilAttachment{VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED};
    std::vector<uint32_t> preserveAttachments;
    VkSubpassDescriptionFlags flags = 0;
};

struct SubpassDependency {
    uint32_t srcSubpass = VK_SUBPASS_EXTERNAL;
    uint32_t dstSubpass = 0;
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkAccessFlags srcAccessMask = 0;
    VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkDependencyFlags dependencyFlags = 0;
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

    [[nodiscard]] VkRenderPass GetHandle() const { return renderPass_; }
    
    // Implicit conversion to VkRenderPass
    operator VkRenderPass() const { return renderPass_; }

    // Check if the render pass is valid
    [[nodiscard]] bool IsValid() const { return renderPass_ != VK_NULL_HANDLE; }

    // Get attachment count
    [[nodiscard]] uint32_t GetAttachmentCount() const { return static_cast<uint32_t>(attachments_.size()); }

    // Get subpass count
    [[nodiscard]] uint32_t GetSubpassCount() const { return static_cast<uint32_t>(subpasses_.size()); }

    // Get attachment descriptions
    [[nodiscard]] const std::vector<AttachmentDescription>& GetAttachments() const { return attachments_; }

    // Get subpass descriptions
    [[nodiscard]] const std::vector<SubpassDescription>& GetSubpasses() const { return subpasses_; }

    // Get dependencies
    [[nodiscard]] const std::vector<SubpassDependency>& GetDependencies() const { return dependencies_; }

    // Helper to create simple color attachment reference
    static VkAttachmentReference CreateColorAttachmentRef(uint32_t attachment,
                                                         VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Helper to create depth attachment reference
    static VkAttachmentReference CreateDepthAttachmentRef(uint32_t attachment,
                                                         VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

private:
    VkRenderPass renderPass_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device

    std::vector<AttachmentDescription> attachments_;
    std::vector<SubpassDescription> subpasses_;
    std::vector<SubpassDependency> dependencies_;

    // Helper methods
    void CreateRenderPass();
    void CreateSimpleRenderPass(VkFormat color_format, VkFormat depth_format, 
                               VkSampleCountFlagBits samples,
                               VkAttachmentLoadOp color_load_op, VkAttachmentLoadOp depth_load_op);
    void Cleanup();
    
    // Conversion helpers
    [[nodiscard]] std::vector<VkAttachmentDescription> ConvertAttachments() const;
    [[nodiscard]] std::vector<VkSubpassDescription> ConvertSubpasses() const;
    [[nodiscard]] std::vector<VkSubpassDependency> ConvertDependencies() const;
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_RENDER_PASS_HPP