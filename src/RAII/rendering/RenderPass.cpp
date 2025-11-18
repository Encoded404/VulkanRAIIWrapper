
#include "RenderPass.hpp"

#include "../core/Device.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>


namespace VulkanEngine::RAII {

RenderPass::RenderPass(const Device& device,
                       const std::vector<AttachmentDescription>& attachments,
                       const std::vector<SubpassDescription>& subpasses,
                       const std::vector<SubpassDependency>& dependencies)
    : device_(device.get_handle()),
      attachments_(attachments),
      subpasses_(subpasses),
      dependencies_(dependencies) {
    if (device == VK_NULL_HANDLE) {
        throw std::invalid_argument("RenderPass requires a valid device");
    }

    if (subpasses.empty()) {
        throw std::invalid_argument("RenderPass requires at least one subpass");
    }

    create_render_pass();
}

RenderPass::RenderPass(const Device& device,
                       VkFormat color_format,
                       VkFormat depth_format,
                       VkSampleCountFlagBits samples,
                       VkAttachmentLoadOp color_load_op,
                       VkAttachmentLoadOp depth_load_op)
    : device_(device.get_handle()) {
    if (device == VK_NULL_HANDLE) {
        throw std::invalid_argument("RenderPass requires a valid device");
    }

    create_simple_render_pass(color_format, depth_format, samples, color_load_op, depth_load_op);
    create_render_pass();
}

RenderPass::~RenderPass() {
    cleanup();
}

RenderPass::RenderPass(RenderPass&& other) noexcept
    : renderPass_(other.renderPass_),
      device_(other.device_),
      attachments_(std::move(other.attachments_)),
      subpasses_(std::move(other.subpasses_)),
      dependencies_(std::move(other.dependencies_)) {
    other.renderPass_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

RenderPass& RenderPass::operator=(RenderPass&& other) noexcept {
    if (this != &other) {
        cleanup();
        renderPass_ = other.renderPass_;
        device_ = other.device_;
        attachments_ = std::move(other.attachments_);
        subpasses_ = std::move(other.subpasses_);
        dependencies_ = std::move(other.dependencies_);

        other.renderPass_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

VkAttachmentReference RenderPass::create_color_attachment_ref(uint32_t attachment, VkImageLayout layout) {
    return {attachment, layout};
}

VkAttachmentReference RenderPass::create_depth_attachment_ref(uint32_t attachment, VkImageLayout layout) {
    return {attachment, layout};
}

void RenderPass::create_render_pass() {
    std::vector<VkAttachmentDescription> vk_attachments = convert_attachments();

    std::vector<std::vector<VkAttachmentReference>> input_attachment_refs(subpasses_.size());
    std::vector<std::vector<VkAttachmentReference>> color_attachment_refs(subpasses_.size());
    std::vector<std::vector<VkAttachmentReference>> resolve_attachment_refs(subpasses_.size());
    std::vector<std::vector<uint32_t>> preserve_attachment_refs(subpasses_.size());
    std::vector<VkAttachmentReference> depth_attachment_refs(subpasses_.size());

    std::vector<VkSubpassDescription> vk_subpasses(subpasses_.size());
    for (size_t i = 0; i < vk_subpasses.size(); ++i) {
        const auto& subpass = subpasses_[i];
        auto& vk_subpass = vk_subpasses[i];
        vk_subpass = {};
        vk_subpass.pipelineBindPoint = subpass.pipelineBindPoint_;
        vk_subpass.flags = subpass.flags_;

        input_attachment_refs[i] = subpass.inputAttachments_;
        color_attachment_refs[i] = subpass.colorAttachments_;
        resolve_attachment_refs[i] = subpass.resolveAttachments_;
        preserve_attachment_refs[i] = subpass.preserveAttachments_;
        depth_attachment_refs[i] = subpass.depthStencilAttachment_;

        vk_subpass.inputAttachmentCount = static_cast<uint32_t>(input_attachment_refs[i].size());
        vk_subpass.pInputAttachments = input_attachment_refs[i].empty() ? nullptr : input_attachment_refs[i].data();

        vk_subpass.colorAttachmentCount = static_cast<uint32_t>(color_attachment_refs[i].size());
        vk_subpass.pColorAttachments = color_attachment_refs[i].empty() ? nullptr : color_attachment_refs[i].data();

        vk_subpass.pResolveAttachments = resolve_attachment_refs[i].empty() ? nullptr : resolve_attachment_refs[i].data();

        vk_subpass.preserveAttachmentCount = static_cast<uint32_t>(preserve_attachment_refs[i].size());
        vk_subpass.pPreserveAttachments = preserve_attachment_refs[i].empty() ? nullptr : preserve_attachment_refs[i].data();

        if (depth_attachment_refs[i].attachment != VK_ATTACHMENT_UNUSED) {
            vk_subpass.pDepthStencilAttachment = &depth_attachment_refs[i];
        } else {
            vk_subpass.pDepthStencilAttachment = nullptr;
        }
    }

    std::vector<VkSubpassDependency> vk_dependencies = convert_dependencies();

    VkRenderPassCreateInfo render_pass_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_info.attachmentCount = static_cast<uint32_t>(vk_attachments.size());
    render_pass_info.pAttachments = vk_attachments.empty() ? nullptr : vk_attachments.data();
    render_pass_info.subpassCount = static_cast<uint32_t>(vk_subpasses.size());
    render_pass_info.pSubpasses = vk_subpasses.data();
    render_pass_info.dependencyCount = static_cast<uint32_t>(vk_dependencies.size());
    render_pass_info.pDependencies = vk_dependencies.empty() ? nullptr : vk_dependencies.data();

    if (vkCreateRenderPass(device_, &render_pass_info, nullptr, &renderPass_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }
}

void RenderPass::create_simple_render_pass(VkFormat color_format,
                                        VkFormat depth_format,
                                        VkSampleCountFlagBits samples,
                                        VkAttachmentLoadOp color_load_op,
                                        VkAttachmentLoadOp depth_load_op) {
    attachments_.clear();
    subpasses_.clear();
    dependencies_.clear();

    AttachmentDescription color_attachment{};
    color_attachment.format_ = color_format;
    color_attachment.samples_ = samples;
    color_attachment.loadOp_ = color_load_op;
    color_attachment.storeOp_ = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.initialLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout_ = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments_.push_back(color_attachment);

    bool has_depth = depth_format != VK_FORMAT_UNDEFINED;
    uint32_t depth_index = 0;
    if (has_depth) {
        depth_index = static_cast<uint32_t>(attachments_.size());
        AttachmentDescription depth_attachment{};
        depth_attachment.format_ = depth_format;
        depth_attachment.samples_ = samples;
        depth_attachment.loadOp_ = depth_load_op;
        depth_attachment.storeOp_ = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp_ = depth_load_op;
        depth_attachment.stencilStoreOp_ = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout_ = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments_.push_back(depth_attachment);
    }

    SubpassDescription subpass{};
    subpass.pipelineBindPoint_ = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachments_.push_back({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    if (has_depth) {
        subpass.depthStencilAttachment_ = {depth_index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    }
    subpasses_.push_back(subpass);

    SubpassDependency dependency{};
    dependency.srcSubpass_ = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass_ = 0;
    dependency.srcStageMask_ = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask_ = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask_ = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (has_depth) {
        dependency.srcStageMask_ |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask_ |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask_ |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    dependencies_.push_back(dependency);
}

void RenderPass::cleanup() {
    if (renderPass_ != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device_, renderPass_, nullptr);
        renderPass_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

std::vector<VkAttachmentDescription> RenderPass::convert_attachments() const {
    std::vector<VkAttachmentDescription> attachments;
    attachments.reserve(attachments_.size());
    for (const auto& attachment : attachments_) {
        VkAttachmentDescription desc{};
        desc.flags = attachment.flags_;
        desc.format = attachment.format_;
        desc.samples = attachment.samples_;
        desc.loadOp = attachment.loadOp_;
        desc.storeOp = attachment.storeOp_;
        desc.stencilLoadOp = attachment.stencilLoadOp_;
        desc.stencilStoreOp = attachment.stencilStoreOp_;
        desc.initialLayout = attachment.initialLayout_;
        desc.finalLayout = attachment.finalLayout_;
        attachments.push_back(desc);
    }
    return attachments;
}

std::vector<VkSubpassDependency> RenderPass::convert_dependencies() const {
    std::vector<VkSubpassDependency> dependencies;
    dependencies.reserve(dependencies_.size());
    for (const auto& dependency : dependencies_) {
        VkSubpassDependency dep{};
        dep.srcSubpass = dependency.srcSubpass_;
        dep.dstSubpass = dependency.dstSubpass_;
        dep.srcStageMask = dependency.srcStageMask_;
        dep.dstStageMask = dependency.dstStageMask_;
        dep.srcAccessMask = dependency.srcAccessMask_;
        dep.dstAccessMask = dependency.dstAccessMask_;
        dep.dependencyFlags = dependency.dependencyFlags_;
        dependencies.push_back(dep);
    }
    return dependencies;
}

} // namespace VulkanEngine::RAII

