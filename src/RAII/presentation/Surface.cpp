#include "Surface.hpp"

#include "../core/instance.hpp"
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_video.h>
#include <cstdint>
#include <stdexcept>
#include <vector>


namespace VulkanEngine::RAII {

Surface::Surface(const Instance& instance, SDL_Window* window)
    : instance_(instance.get_handle()), window_(window) {
    if (!window) {
        throw std::invalid_argument("SDL_Window pointer must not be null");
    }
    create_surface_from_sdl(window);
}

Surface::Surface(VkSurfaceKHR surface, const Instance& instance)
    : surface_(surface), instance_(instance.get_handle()) {}

Surface::Surface(const Instance& instance, void* window_handle, void* platform_handle)
    : instance_(instance.get_handle()) {
    create_surface(window_handle, platform_handle);
}

Surface::~Surface() {
    cleanup();
}

Surface::Surface(Surface&& other) noexcept
    : surface_(other.surface_),
      instance_(other.instance_),
      window_(other.window_) {
    other.surface_ = VK_NULL_HANDLE;
    other.instance_ = VK_NULL_HANDLE;
    other.window_ = nullptr;
}

Surface& Surface::operator=(Surface&& other) noexcept {
    if (this != &other) {
        cleanup();
        surface_ = other.surface_;
        instance_ = other.instance_;
        window_ = other.window_;
        other.surface_ = VK_NULL_HANDLE;
        other.instance_ = VK_NULL_HANDLE;
        other.window_ = nullptr;
    }
    return *this;
}

VkSurfaceCapabilitiesKHR Surface::get_capabilities(VkPhysicalDevice physical_device) const {
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface_, &capabilities);
    return capabilities;
}

std::vector<VkSurfaceFormatKHR> Surface::get_formats(VkPhysicalDevice physical_device) const {
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_, &format_count, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(format_count);
    if (format_count > 0) {
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_, &format_count, formats.data());
    }
    return formats;
}

std::vector<VkPresentModeKHR> Surface::get_present_modes(VkPhysicalDevice physical_device) const {
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface_, &present_mode_count, nullptr);

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    if (present_mode_count > 0) {
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface_, &present_mode_count, present_modes.data());
    }
    return present_modes;
}

bool Surface::is_supported(VkPhysicalDevice physical_device, uint32_t queue_family_index) const {
    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface_, &supported);
    return supported == VK_TRUE;
}

void Surface::get_drawable_size(SDL_Window* window, int* width, int* height) const {
    SDL_GetWindowSizeInPixels(window ? window : window_, width, height);
}

std::vector<const char*> Surface::get_required_instance_extensions(SDL_Window* /*window*/) {
    unsigned int count = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);
    if (!extensions || count == 0) {
        throw std::runtime_error("Failed to query SDL Vulkan instance extensions");
    }

    return std::vector<const char*>(extensions, extensions + count);
}

bool Surface::is_vulkan_supported() {
    return SDL_Vulkan_LoadLibrary(nullptr) == 0;
}

void Surface::create_surface_from_sdl(SDL_Window* window) {
    if (!SDL_Vulkan_CreateSurface(window, reinterpret_cast<VkInstance>(instance_), nullptr, &surface_)) {
        throw std::runtime_error("Failed to create Vulkan surface from SDL window");
    }
}

void Surface::create_surface(void* /*windowHandle*/, void* /*platformHandle*/) {
    throw std::runtime_error("Generic surface creation is not implemented for this platform");
}

void Surface::cleanup() {
    if (surface_ != VK_NULL_HANDLE && instance_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }
}

} // namespace VulkanEngine::RAII

