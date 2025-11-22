#include "Renderer.hpp"

#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Framebuffer.hpp"
#include "RenderPass.hpp"
#include "../core/Device.hpp"
#include "../presentation/Swapchain.hpp"
#include "../sync/Semaphore.hpp"
#include "../sync/Fence.hpp"
// #include "../utils/SDLUtils.hpp"


#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#include <limits>
#include <iostream>
#include <cstddef> // for std::size_t

namespace VulkanEngine::RAII {

Renderer::Renderer(const Device& device,
                                     Swapchain& swapchain,
                                     const RenderPass& render_pass,
                   uint32_t max_frames_in_flight)
    : device_(&device),
            swapchain_(&swapchain),
      renderPass_(&render_pass),
      maxFramesInFlight_(std::max(1u, max_frames_in_flight)) {
    CreateCommandObjects();
    CreateSyncObjects(swapchain.GetImageCount());
    CreateFramebuffers();
}

Renderer::~Renderer() {
    Cleanup();
}

Renderer::Renderer(Renderer&& other) noexcept
    : device_(other.device_),
      swapchain_(other.swapchain_),
      renderPass_(other.renderPass_),
      maxFramesInFlight_(other.maxFramesInFlight_),
      currentFrame_(other.currentFrame_),
      imageIndex_(other.imageIndex_),
      frameInProgress_(other.frameInProgress_),
      needsSwapchainRecreation_(other.needsSwapchainRecreation_),
      commandPools_(std::move(other.commandPools_)),
      commandBuffers_(std::move(other.commandBuffers_)),
      imageAvailableSemaphores_(std::move(other.imageAvailableSemaphores_)),
      renderFinishedSemaphores_(std::move(other.renderFinishedSemaphores_)),
      inFlightFences_(std::move(other.inFlightFences_)),
      framebuffers_(std::move(other.framebuffers_)) {
    other.device_ = nullptr;
    other.swapchain_ = nullptr;
    other.renderPass_ = nullptr;
    other.maxFramesInFlight_ = 0;
    other.currentFrame_ = 0;
    other.imageIndex_ = 0;
    other.frameInProgress_ = false;
    other.needsSwapchainRecreation_ = false;
}

Renderer& Renderer::operator=(Renderer&& other) noexcept {
    if (this != &other) {
        Cleanup();
        device_ = other.device_;
        swapchain_ = other.swapchain_;
        renderPass_ = other.renderPass_;
        maxFramesInFlight_ = other.maxFramesInFlight_;
        currentFrame_ = other.currentFrame_;
        imageIndex_ = other.imageIndex_;
        frameInProgress_ = other.frameInProgress_;
        needsSwapchainRecreation_ = other.needsSwapchainRecreation_;
        commandPools_ = std::move(other.commandPools_);
        commandBuffers_ = std::move(other.commandBuffers_);
        imageAvailableSemaphores_ = std::move(other.imageAvailableSemaphores_);
        renderFinishedSemaphores_ = std::move(other.renderFinishedSemaphores_);
        inFlightFences_ = std::move(other.inFlightFences_);
        framebuffers_ = std::move(other.framebuffers_);

        other.device_ = nullptr;
        other.swapchain_ = nullptr;
        other.renderPass_ = nullptr;
        other.maxFramesInFlight_ = 0;
        other.currentFrame_ = 0;
        other.imageIndex_ = 0;
        other.frameInProgress_ = false;
        other.needsSwapchainRecreation_ = false;
    }
    return *this;
}

bool Renderer::BeginFrame() {
    if (frameInProgress_) {
        return false;
    }

    if (!device_ || !swapchain_) {
        return false;
    }

    VulkanEngine::RAII::Fence& fence = *inFlightFences_[currentFrame_];
    fence.Wait();
    
    VkResult result = swapchain_->AcquireNextImage(std::numeric_limits<uint64_t>::max(),
                                                    *imageAvailableSemaphores_[currentFrame_],
                                                    VK_NULL_HANDLE,
                                                    imageIndex_);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || swapchain_->NeedsRecreate())
    {
        //lastRecreateTime_ = totalFrameCount_;
        //recreate();
        needsSwapchainRecreation_ = true;
        //return false;
    }
    
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    // only when we know we are going to do work, reset the fence
    fence.Reset();
    
    auto& command_buffer = *commandBuffers_[currentFrame_];
    command_buffer.Reset();
    command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    frameInProgress_ = true;
    return true;
}

bool Renderer::EndFrame() {
    if (!frameInProgress_ || !device_ || !swapchain_) {
        return false;
    }

    auto& command_buffer = *commandBuffers_[currentFrame_];
    command_buffer.End();

    // Wait on the same semaphore we used for acquire in begin_frame (per-frame indexing)
    VkSemaphore wait_semaphores[] = {*imageAvailableSemaphores_[currentFrame_]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // Signal the per-image render-finished semaphore; present will wait on this
    VkSemaphore signal_semaphores[] = {*renderFinishedSemaphores_[imageIndex_]};

    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    VkCommandBuffer raw_command_buffer = command_buffer.GetHandle();
    submit_info.pCommandBuffers = &raw_command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    VkQueue graphics_queue = device_->GetGraphicsQueue();
    if (vkQueueSubmit(graphics_queue,
                      1,
                      &submit_info,
                      *inFlightFences_[currentFrame_]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    // Present waits on the semaphore signaled by the submit above (per-image indexing)
    std::vector<VkSemaphore> wait_sem{*renderFinishedSemaphores_[imageIndex_]};
    VkQueue present_queue = device_->GetPresentQueue();
    VkResult present_result = swapchain_->Present(wait_sem, imageIndex_, present_queue);

    // std::cout << "present_result: " << present_result << " needSwapchainRecreation_: " << needsSwapchainRecreation_ << " swapchain_->needs_recreate(): " << swapchain_->NeedsRecreate() << '\n';
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || needsSwapchainRecreation_ || swapchain_->NeedsRecreate()) {
        // the semaphores are still valid, no need to recreate them
        if(totalFrameCount_ - lastRecreateTime_ > 5)
        {
            lastRecreateTime_ = totalFrameCount_;
            Recreate(false);
            needsSwapchainRecreation_ = false;
        }
    } else if (present_result == VK_SUBOPTIMAL_KHR) {
        // the semaphores are still valid, no need to recreate them
        std::cout << "swapchain is suboptimal but still valid\n";
        if(totalFrameCount_ - lastRecreateTime_ > 5)
        {
            lastRecreateTime_ = totalFrameCount_;
            Recreate(false);
            needsSwapchainRecreation_ = false;
        }
    } else if (present_result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swapchain image");
    }

    frameInProgress_ = false;
    currentFrame_ = (currentFrame_ + 1) % maxFramesInFlight_;
    totalFrameCount_++;
    return !needsSwapchainRecreation_;
}

CommandBuffer& Renderer::GetCurrentCommandBuffer() {
    if (!frameInProgress_) {
        throw std::runtime_error("No frame in progress");
    }
    return *commandBuffers_[currentFrame_];
}

void Renderer::WaitIdle() {
    if (device_) {
        vkDeviceWaitIdle(device_->GetHandle());
    }
}

void Renderer::SetExternalAttachments(std::vector<std::vector<VkImageView>> attachments) {
    extraAttachments_ = std::move(attachments);
}

void Renderer::ClearExternalAttachments() {
    extraAttachments_.clear();
}

void Renderer::RebuildFramebuffers() {
    WaitIdle();
    CreateFramebuffers();
}

void Renderer::Recreate(bool recreate_semaphores)
{
    WaitIdle();
    // recreate swapchain
    swapchain_->Recreate();
    // recreate semaphore sync objects if requested
    if (recreate_semaphores) {
        RecreateSemaphoreSyncObjects(swapchain_->GetImageCount());
    }
    // create new framebuffers
    CreateFramebuffers();
}

void Renderer::RecreateSemaphoreSyncObjects(uint32_t num_of_swapchain_images) {
    if (!device_ || !swapchain_) {
        std::cerr << "Renderer::recreate_semaphore_sync_objects called without valid device or swapchain" << '\n' << std::flush;
        return;
    }

    imageAvailableSemaphores_.clear();
    renderFinishedSemaphores_.clear();

    imageAvailableSemaphores_.reserve(maxFramesInFlight_);
    renderFinishedSemaphores_.reserve(num_of_swapchain_images);

    for (uint32_t i = 0; i < num_of_swapchain_images; ++i) {
        renderFinishedSemaphores_.push_back(std::make_unique<Semaphore>(*device_));
        if (i < maxFramesInFlight_) {
            imageAvailableSemaphores_.push_back(std::make_unique<Semaphore>(*device_));
        }
    }
}

void Renderer::CreateSyncObjects(uint32_t num_of_swapchain_images) {
    imageAvailableSemaphores_.clear();
    renderFinishedSemaphores_.clear();
    inFlightFences_.clear();

    imageAvailableSemaphores_.reserve(maxFramesInFlight_);
    renderFinishedSemaphores_.reserve(num_of_swapchain_images);
    inFlightFences_.reserve(maxFramesInFlight_);

    for (uint32_t i = 0; i < num_of_swapchain_images; ++i) {
        renderFinishedSemaphores_.push_back(std::make_unique<Semaphore>(*device_));
        if (i < maxFramesInFlight_) {
            imageAvailableSemaphores_.push_back(std::make_unique<Semaphore>(*device_));
            inFlightFences_.push_back(std::make_unique<Fence>(*device_, VK_FENCE_CREATE_SIGNALED_BIT));
        }
    }
}

void Renderer::CreateCommandObjects() {
    if (!device_) {
        throw std::runtime_error("Renderer requires a valid device");
    }

    auto indices = device_->GetQueueFamilyIndices();
    if (!indices.graphicsFamily_.has_value()) {
        throw std::runtime_error("Renderer requires graphics queue family");
    }

    commandPools_.clear();
    commandBuffers_.clear();

    commandPools_.reserve(maxFramesInFlight_);
    commandBuffers_.reserve(maxFramesInFlight_);

    for (uint32_t i = 0; i < maxFramesInFlight_; ++i) {
        std::unique_ptr<VulkanEngine::RAII::CommandPool> pool = std::make_unique<CommandPool>(*device_, indices.graphicsFamily_.value());
        std::unique_ptr<VulkanEngine::RAII::CommandBuffer> buffer = std::make_unique<CommandBuffer>(*pool);
        commandPools_.push_back(std::move(pool));
        commandBuffers_.push_back(std::move(buffer));
    }
}

void Renderer::CreateFramebuffers()
{
    framebuffers_.clear();

    if (!swapchain_ || !renderPass_) {
        return;
    }

    const auto& image_views = swapchain_->GetImageViews();
    VkExtent2D extent = swapchain_->GetExtent();
    bool include_extra = !extraAttachments_.empty() && extraAttachments_.size() == image_views.size();
    if (!extraAttachments_.empty() && extraAttachments_.size() != image_views.size()) {
        std::cerr << "Renderer::CreateFramebuffers: ignoring extra attachments due to size mismatch" << '\n';
        include_extra = false;
    }

    framebuffers_.reserve(image_views.size());
    for (std::size_t i = 0; i < image_views.size(); ++i) {
        std::vector<VkImageView> attachments{image_views[i]};
        if (include_extra) {
            const std::vector<VkImageView>& extras = extraAttachments_[i];
            attachments.insert(attachments.end(), extras.begin(), extras.end());
        }
        framebuffers_.push_back(std::make_unique<Framebuffer>(*device_,
                                                               *renderPass_,
                                                               attachments,
                                                               extent.width,
                                                               extent.height));
    }
}

void Renderer::Cleanup() {
    if (device_) {
        vkDeviceWaitIdle(device_->GetHandle());
    }

    framebuffers_.clear();
    commandBuffers_.clear();
    commandPools_.clear();
    imageAvailableSemaphores_.clear();
    renderFinishedSemaphores_.clear();
    inFlightFences_.clear();
    extraAttachments_.clear();

    device_ = nullptr;
    swapchain_ = nullptr;
    renderPass_ = nullptr;
    maxFramesInFlight_ = 0;
    currentFrame_ = 0;
    imageIndex_ = 0;
    frameInProgress_ = false;
    needsSwapchainRecreation_ = false;
}

} // namespace VulkanEngine::RAII

