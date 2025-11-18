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


namespace VulkanEngine::RAII {

Renderer::Renderer(const Device& device,
                                     Swapchain& swapchain,
                                     const RenderPass& render_pass,
                   uint32_t max_frames_in_flight)
    : device_(&device),
            swapchain_(&swapchain),
      renderPass_(&render_pass),
      maxFramesInFlight_(std::max(1u, max_frames_in_flight)) {
    create_command_objects();
    create_sync_objects(swapchain.get_image_count());
    create_framebuffers();
}

Renderer::~Renderer() {
    cleanup();
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
        cleanup();
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

bool Renderer::begin_frame() {
    if (frameInProgress_) {
        return false;
    }

    if (!device_ || !swapchain_) {
        return false;
    }

    VulkanEngine::RAII::Fence& fence = *inFlightFences_[currentFrame_];
    fence.wait();
    
    VkResult result = swapchain_->acquire_next_image(std::numeric_limits<uint64_t>::max(),
                                                    *imageAvailableSemaphores_[currentFrame_],
                                                    VK_NULL_HANDLE,
                                                    imageIndex_);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || swapchain_->needs_recreate())
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
    fence.reset();
    
    auto& command_buffer = *commandBuffers_[currentFrame_];
    command_buffer.reset();
    command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    frameInProgress_ = true;
    return true;
}

bool Renderer::end_frame() {
    if (!frameInProgress_ || !device_ || !swapchain_) {
        return false;
    }

    auto& command_buffer = *commandBuffers_[currentFrame_];
    command_buffer.end();

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
    VkCommandBuffer raw_command_buffer = command_buffer.get_handle();
    submit_info.pCommandBuffers = &raw_command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    VkQueue graphics_queue = device_->get_graphics_queue();
    if (vkQueueSubmit(graphics_queue,
                      1,
                      &submit_info,
                      *inFlightFences_[currentFrame_]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    // Present waits on the semaphore signaled by the submit above (per-image indexing)
    std::vector<VkSemaphore> wait_sem{*renderFinishedSemaphores_[imageIndex_]};
    VkQueue present_queue = device_->get_present_queue();
    VkResult present_result = swapchain_->present(wait_sem, imageIndex_, present_queue);

    // std::cout << "present_result: " << present_result << " needSwapchainRecreation_: " << needsSwapchainRecreation_ << " swapchain_->needs_recreate(): " << swapchain_->needs_recreate() << '\n';
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || needsSwapchainRecreation_ || swapchain_->needs_recreate()) {
        // the semaphores are still valid, no need to recreate them
        if(totalFrameCount_ - lastRecreateTime_ > 5)
        {
            lastRecreateTime_ = totalFrameCount_;
            recreate(false);
            needsSwapchainRecreation_ = false;
        }
    } else if (present_result == VK_SUBOPTIMAL_KHR) {
        // the semaphores are still valid, no need to recreate them
        std::cout << "swapchain is suboptimal but still valid\n";
        if(totalFrameCount_ - lastRecreateTime_ > 5)
        {
            lastRecreateTime_ = totalFrameCount_;
            recreate(false);
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

CommandBuffer& Renderer::get_current_command_buffer() {
    if (!frameInProgress_) {
        throw std::runtime_error("No frame in progress");
    }
    return *commandBuffers_[currentFrame_];
}

void Renderer::wait_idle() {
    if (device_) {
        vkDeviceWaitIdle(device_->get_handle());
    }
}

void Renderer::recreate(bool recreate_semaphores)
{
    wait_idle();
    // recreate swapchain
    swapchain_->recreate();
    // recreate semaphore sync objects if requested
    if (recreate_semaphores) {
        recreate_semaphore_sync_objects(swapchain_->get_image_count());
    }
    // create new framebuffers
    create_framebuffers();
}

void Renderer::recreate_semaphore_sync_objects(uint32_t num_of_swapchain_images) {
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

void Renderer::create_sync_objects(uint32_t num_of_swapchain_images) {
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

void Renderer::create_command_objects() {
    if (!device_) {
        throw std::runtime_error("Renderer requires a valid device");
    }

    auto indices = device_->get_queue_family_indices();
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

void Renderer::create_framebuffers()
{
    framebuffers_.clear();

    if (!swapchain_ || !renderPass_) {
        return;
    }

    const auto& image_views = swapchain_->get_image_views();
    VkExtent2D extent = swapchain_->get_extent();

    framebuffers_.reserve(image_views.size());
    for (auto view : image_views) {
        std::vector<VkImageView> attachments{view};
        framebuffers_.push_back(std::make_unique<Framebuffer>(*device_,
                                                               *renderPass_,
                                                               attachments,
                                                               extent.width,
                                                               extent.height));
    }
}

void Renderer::cleanup() {
    if (device_) {
        vkDeviceWaitIdle(device_->get_handle());
    }

    framebuffers_.clear();
    commandBuffers_.clear();
    commandPools_.clear();
    imageAvailableSemaphores_.clear();
    renderFinishedSemaphores_.clear();
    inFlightFences_.clear();

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

