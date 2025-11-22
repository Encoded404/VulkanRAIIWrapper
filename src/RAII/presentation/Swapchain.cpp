#include "Swapchain.hpp"

#include "../core/Device.hpp"
#include "../core/PhysicalDevice.hpp"
#include "Surface.hpp"
#include "types/QueueFamilyIndices.hpp"

#include <SDL3/SDL_video.h>
// #include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>
// #include <iostream>


namespace VulkanEngine::RAII {

Swapchain::Swapchain(const Device& device,
                     const Surface& surface,
                     SDL_Window* window,
                     VkPresentModeKHR preferred_present_mode,
                     VkSurfaceFormatKHR preferred_format)
    : device_(device.GetHandle()),
      surface_(surface.GetHandle()),
    deviceRef_(&device),
      window_(window),
      presentMode_(preferred_present_mode),
      surfaceFormat_(preferred_format) {
    int width = 0;
    int height = 0;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    if (width <= 0 || height <= 0) {
        width = 1;
        height = 1;
    }
    CreateSwapchain(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    CreateImageViews();
}

Swapchain::Swapchain(const Device& device,
                     const Surface& surface,
                     uint32_t width,
                     uint32_t height,
                     VkPresentModeKHR preferred_present_mode,
                     VkSurfaceFormatKHR preferred_format)
    : device_(device.GetHandle()),
      surface_(surface.GetHandle()),
    deviceRef_(&device),
      presentMode_(preferred_present_mode),
      surfaceFormat_(preferred_format) {
    CreateSwapchain(width, height);
    CreateImageViews();
}

Swapchain::~Swapchain() {
    Cleanup();
}

Swapchain::Swapchain(Swapchain&& other) noexcept
    : swapchain_(other.swapchain_),
      device_(other.device_),
      surface_(other.surface_),
    deviceRef_(other.deviceRef_),
      window_(other.window_),
      imageViews_(std::move(other.imageViews_)),
      imageFormat_(other.imageFormat_),
      extent_(other.extent_),
      presentMode_(other.presentMode_),
      surfaceFormat_(other.surfaceFormat_),
      needsRecreate_(other.needsRecreate_) {
    other.swapchain_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.surface_ = VK_NULL_HANDLE;
    other.deviceRef_ = nullptr;
    other.window_ = nullptr;
    other.imageFormat_ = VK_FORMAT_UNDEFINED;
    other.extent_ = {0, 0};
    other.needsRecreate_ = false;
}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept {
    if (this != &other) {
        Cleanup();
        swapchain_ = other.swapchain_;
        device_ = other.device_;
        surface_ = other.surface_;
    deviceRef_ = other.deviceRef_;
        window_ = other.window_;
        imageViews_ = std::move(other.imageViews_);
        imageFormat_ = other.imageFormat_;
        extent_ = other.extent_;
        presentMode_ = other.presentMode_;
        surfaceFormat_ = other.surfaceFormat_;
        needsRecreate_ = other.needsRecreate_;

        other.swapchain_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.surface_ = VK_NULL_HANDLE;
    other.deviceRef_ = nullptr;
        other.window_ = nullptr;
        other.imageFormat_ = VK_FORMAT_UNDEFINED;
        other.extent_ = {0, 0};
        other.needsRecreate_ = false;
    }
    return *this;
}

std::vector<VkImage> Swapchain::GetImages() const {
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    if (image_count > 0) {
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, images.data());
    }
    return images;
}

VkResult Swapchain::AcquireNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t& image_index) const {
    return vkAcquireNextImageKHR(device_, swapchain_, timeout, semaphore, fence, &image_index);
}

VkResult Swapchain::Present(const std::vector<VkSemaphore>& wait_semaphores, uint32_t image_index, VkQueue present_queue) const {
    VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
    present_info.pWaitSemaphores = wait_semaphores.empty() ? nullptr : wait_semaphores.data();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain_;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    return vkQueuePresentKHR(present_queue, &present_info);
}

void Swapchain::Recreate(SDL_Window* window) {
    int width = 0;
    int height = 0;
    if(window_ == nullptr && window == nullptr) {
        throw std::runtime_error("Swapchain::recreate called with no valid SDL_Window");
    }
    SDL_GetWindowSizeInPixels(window ? window : window_, &width, &height);
    if (width <= 0 || height <= 0) {
        return;
    }
    Recreate(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    
    if(window)
    {
        window_ = window;
    }
}

void Swapchain::Recreate(uint32_t width, uint32_t height) {
    Cleanup();
    CreateSwapchain(width, height);
    CreateImageViews();
    needsRecreate_ = false;
}

SwapchainSupportDetails Swapchain::QuerySwapchainSupport(VkPhysicalDevice physical_device) const {
    SwapchainSupportDetails details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface_, &details.capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_, &format_count, nullptr);
    if (format_count > 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_, &format_count, details.formats.data());
    }

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface_, &present_mode_count, nullptr);
    if (present_mode_count > 0) {
        details.presentModes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface_, &present_mode_count, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) const {
    for (const auto& available_format : available_formats) {
        if (available_format.format == surfaceFormat_.format &&
            available_format.colorSpace == surfaceFormat_.colorSpace) {
            return available_format;
        }
    }
    return available_formats.empty() ? VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
                                    : available_formats.front();
}

VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) const {
    for (auto mode : available_present_modes) {
        if (mode == presentMode_) {
            return mode;
        }
    }
    for (auto mode : available_present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    VkExtent2D actual_extent{width, height};

    actual_extent.width = std::max(capabilities.minImageExtent.width,
                                  std::min(capabilities.maxImageExtent.width, actual_extent.width));
    actual_extent.height = std::max(capabilities.minImageExtent.height,
                                   std::min(capabilities.maxImageExtent.height, actual_extent.height));

    return actual_extent;
}

void Swapchain::CreateSwapchain(uint32_t width, uint32_t height) {
    if (!deviceRef_) {
        throw std::runtime_error("Swapchain requires valid device reference");
    }

    VkPhysicalDevice physical_device = deviceRef_->GetPhysicalDevice().GetHandle();
    SwapchainSupportDetails support = QuerySwapchainSupport(physical_device);

    VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(support.formats);
    VkPresentModeKHR present_mode = ChooseSwapPresentMode(support.presentModes);
    VkExtent2D extent = ChooseSwapExtent(support.capabilities, width, height);

    uint32_t image_count;
    if(minImageCount_ != 0)
    {
        image_count = minImageCount_;
    }
    else
    {
        image_count = std::max(support.capabilities.minImageCount, 3u);
    }

    if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
        image_count = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    create_info.surface = surface_;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = deviceRef_->GetQueueFamilyIndices();
    uint32_t queue_family_indices[] = {
        indices.graphicsFamily_.value_or(0),
        indices.presentFamily_.value_or(indices.graphicsFamily_.value_or(0))
    };

    if (indices.graphicsFamily_ != indices.presentFamily_) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = swapchain_;

    VkSwapchainKHR new_swapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &new_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain");
    }

    if (new_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
    }
    swapchain_ = new_swapchain;
    imageFormat_ = surface_format.format;
    extent_ = extent;
    presentMode_ = present_mode;
    surfaceFormat_ = surface_format;
}

void Swapchain::CreateImageViews() {
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    if (image_count > 0) {
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, images.data());
    }

    imageViews_.resize(image_count);
    for (uint32_t i = 0; i < image_count; ++i) {
        VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_info.image = images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = imageFormat_;
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device_, &view_info, nullptr, &imageViews_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain image views");
        }
    }
}

void Swapchain::Cleanup() {
    for (VkImageView view : imageViews_) {
        if (view != VK_NULL_HANDLE) {
            vkDestroyImageView(device_, view, nullptr);
        }
    }
    imageViews_.clear();

    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }
}

} // namespace VulkanEngine::RAII

