#ifndef VULKAN_RAII_UTILS_CONSTANTS_HPP
#define VULKAN_RAII_UTILS_CONSTANTS_HPP

#include <cstdint>
#include <volk.h>
#include <array>

namespace VulkanEngine::RAII::Constants {

// Common validation layers
constexpr const char* KHRONOS_VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";

// Common device extensions
constexpr const char* SWAPCHAIN_EXTENSION = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
constexpr const char* MAINTENANCE1_EXTENSION = VK_KHR_MAINTENANCE1_EXTENSION_NAME;
constexpr const char* DEDICATED_ALLOCATION_EXTENSION = VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME;
constexpr const char* GET_MEMORY_REQUIREMENTS2_EXTENSION = VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME;

// Common instance extensions
constexpr const char* DEBUG_UTILS_EXTENSION = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
constexpr const char* SURFACE_EXTENSION = VK_KHR_SURFACE_EXTENSION_NAME;

// Common queue priorities
constexpr float QUEUE_PRIORITY_HIGH = 1.0f;
constexpr float QUEUE_PRIORITY_MEDIUM = 0.5f;
constexpr float QUEUE_PRIORITY_LOW = 0.1f;

// Common timeout values (in nanoseconds)
constexpr uint64_t TIMEOUT_1_SECOND = 1000000000ULL;
constexpr uint64_t TIMEOUT_100_MS = 100000000ULL;
constexpr uint64_t TIMEOUT_10_MS = 10000000ULL;
constexpr uint64_t TIMEOUT_1_MS = 1000000ULL;

// Common memory alignment values
constexpr VkDeviceSize MEMORY_ALIGNMENT_4 = 4;
constexpr VkDeviceSize MEMORY_ALIGNMENT_16 = 16;
constexpr VkDeviceSize MEMORY_ALIGNMENT_256 = 256;

// Common buffer sizes
constexpr VkDeviceSize BUFFER_SIZE_1KB = (VkDeviceSize)1024;
constexpr VkDeviceSize BUFFER_SIZE_1MB = (VkDeviceSize)1024 * 1024;
constexpr VkDeviceSize BUFFER_SIZE_16MB = (VkDeviceSize)16 * 1024 * 1024;
constexpr VkDeviceSize BUFFER_SIZE_64MB = (VkDeviceSize)64 * 1024 * 1024;
constexpr VkDeviceSize BUFFER_SIZE_256MB = (VkDeviceSize)256 * 1024 * 1024;

// Common clear values
constexpr VkClearColorValue CLEAR_COLOR_BLACK = {{0.0f, 0.0f, 0.0f, 1.0f}};
constexpr VkClearColorValue CLEAR_COLOR_WHITE = {{1.0f, 1.0f, 1.0f, 1.0f}};
constexpr VkClearColorValue CLEAR_COLOR_RED = {{1.0f, 0.0f, 0.0f, 1.0f}};
constexpr VkClearColorValue CLEAR_COLOR_GREEN = {{0.0f, 1.0f, 0.0f, 1.0f}};
constexpr VkClearColorValue CLEAR_COLOR_BLUE = {{0.0f, 0.0f, 1.0f, 1.0f}};
constexpr VkClearColorValue CLEAR_COLOR_CORNFLOWER_BLUE = {{0.392f, 0.584f, 0.929f, 1.0f}};

constexpr VkClearDepthStencilValue CLEAR_DEPTH_STENCIL_1_0 = {1.0f, 0};
constexpr VkClearDepthStencilValue CLEAR_DEPTH_STENCIL_0_0 = {0.0f, 0};

// Common image formats
constexpr VkFormat FORMAT_COLOR_SRGB = VK_FORMAT_B8G8R8A8_SRGB;
constexpr VkFormat FORMAT_COLOR_UNORM = VK_FORMAT_B8G8R8A8_UNORM;
constexpr VkFormat FORMAT_DEPTH_32F = VK_FORMAT_D32_SFLOAT;
constexpr VkFormat FORMAT_DEPTH_24_STENCIL_8 = VK_FORMAT_D24_UNORM_S8_UINT;
constexpr VkFormat FORMAT_DEPTH_32F_STENCIL_8 = VK_FORMAT_D32_SFLOAT_S8_UINT;

// Common present modes
constexpr VkPresentModeKHR PRESENT_MODE_IMMEDIATE = VK_PRESENT_MODE_IMMEDIATE_KHR;
constexpr VkPresentModeKHR PRESENT_MODE_MAILBOX = VK_PRESENT_MODE_MAILBOX_KHR;
constexpr VkPresentModeKHR PRESENT_MODE_FIFO = VK_PRESENT_MODE_FIFO_KHR;
constexpr VkPresentModeKHR PRESENT_MODE_FIFO_RELAXED = VK_PRESENT_MODE_FIFO_RELAXED_KHR;

// Common color spaces
constexpr VkColorSpaceKHR COLOR_SPACE_SRGB = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// Application info constants
constexpr uint32_t ENGINE_VERSION = VK_MAKE_VERSION(1, 0, 0);
constexpr uint32_t VULKAN_API_VERSION = VK_API_VERSION_1_0;
constexpr const char* ENGINE_NAME = "VulkanEngine";

// Common validation layer array
const std::array<const char*, 1> VALIDATION_LAYERS = {
    KHRONOS_VALIDATION_LAYER
};

// Common device extensions array
const std::array<const char*, 1> DEVICE_EXTENSIONS = {
    SWAPCHAIN_EXTENSION
};

// Note: Do not hardcode platform-specific surface extensions here.
// Use runtime queries (e.g., SDLUtils::get_required_instance_extensions or
// Surface::get_required_instance_extensions) to gather the correct set.

// Note: When using SDL3, use SDLUtils::get_required_instance_extensions() instead,
// as SDL2 will automatically provide the correct platform-specific surface extensions.

// Commonly used vertex input binding
constexpr VkVertexInputBindingDescription VERTEX_BINDING_DESCRIPTION = {
    0,                                // binding
    0,                                // stride (to be filled)
    VK_VERTEX_INPUT_RATE_VERTEX       // inputRate
};

// Maximum number of common objects
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
constexpr uint32_t MAX_DESCRIPTOR_SETS = 1000;
constexpr uint32_t MAX_UNIFORM_BUFFERS = 100;
constexpr uint32_t MAX_STORAGE_BUFFERS = 100;
constexpr uint32_t MAX_SAMPLERS = 100;
constexpr uint32_t MAX_IMAGES = 1000;

// SDL-specific constants used across the engine
namespace SDL {
    constexpr int DEFAULT_WINDOW_WIDTH = 1280;
    constexpr int DEFAULT_WINDOW_HEIGHT = 720;
    constexpr float DEFAULT_DPI = 96.0f;
    constexpr float HIGH_DPI_THRESHOLD = 144.0f; // 1.5x scaling
    constexpr uint64_t NANOSECONDS_PER_SECOND = 1000000000ULL;
}

} // namespace VulkanEngine::RAII::Constants



#endif // VULKAN_RAII_UTILS_CONSTANTS_HPP