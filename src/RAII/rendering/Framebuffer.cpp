#include "Framebuffer.hpp"

#include "RenderPass.hpp"
#include "../core/Device.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>


namespace VulkanEngine::RAII {

Framebuffer::Framebuffer(const Device& device,
                         const RenderPass& render_pass,
                         const std::vector<VkImageView>& attachments,
                         uint32_t width,
                         uint32_t height,
                         uint32_t layers)
    : device_(device.GetHandle()),
      renderPass_(render_pass.GetHandle()),
      attachments_(attachments),
      width_(width),
      height_(height),
      layers_(layers) {
    if (device == VK_NULL_HANDLE) {
        throw std::invalid_argument("Framebuffer requires a valid device");
    }
    if (renderPass_ == VK_NULL_HANDLE) {
        throw std::invalid_argument("Framebuffer requires a valid render pass");
    }
    if (width_ == 0 || height_ == 0) {
        throw std::invalid_argument("Framebuffer dimensions must be greater than zero");
    }

    CreateFramebuffer();
}

Framebuffer::~Framebuffer() {
    Cleanup();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : framebuffer_(other.framebuffer_),
      device_(other.device_),
      renderPass_(other.renderPass_),
      attachments_(std::move(other.attachments_)),
      width_(other.width_),
      height_(other.height_),
      layers_(other.layers_) {
    other.framebuffer_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.renderPass_ = VK_NULL_HANDLE;
    other.width_ = 0;
    other.height_ = 0;
    other.layers_ = 0;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        Cleanup();
        framebuffer_ = other.framebuffer_;
        device_ = other.device_;
        renderPass_ = other.renderPass_;
        attachments_ = std::move(other.attachments_);
        width_ = other.width_;
        height_ = other.height_;
        layers_ = other.layers_;

        other.framebuffer_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.renderPass_ = VK_NULL_HANDLE;
        other.width_ = 0;
        other.height_ = 0;
        other.layers_ = 0;
    }
    return *this;
}

void Framebuffer::CreateFramebuffer() {
    VkFramebufferCreateInfo framebuffer_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebuffer_info.renderPass = renderPass_;
    framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments_.size());
    framebuffer_info.pAttachments = attachments_.empty() ? nullptr : attachments_.data();
    framebuffer_info.width = width_;
    framebuffer_info.height = height_;
    framebuffer_info.layers = layers_;

    if (vkCreateFramebuffer(device_, &framebuffer_info, nullptr, &framebuffer_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer");
    }
}

void Framebuffer::Cleanup() {
    if (framebuffer_ != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(device_, framebuffer_, nullptr);
        framebuffer_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
    renderPass_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

