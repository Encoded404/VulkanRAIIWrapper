#ifndef VULKAN_RAII_PRESENTATION_SURFACE_HPP
#define VULKAN_RAII_PRESENTATION_SURFACE_HPP

#include <volk.h>
#include <vector>

// Forward declare SDL types to avoid including SDL2 in header
struct SDL_Window;


namespace VulkanEngine::RAII {

class Instance; // Forward declaration

class Surface {
public:
    // Primary constructor for SDL2 integration
    Surface(const Instance& instance, SDL_Window* window);

    // Constructor that wraps an existing VkSurfaceKHR
    Surface(VkSurfaceKHR surface, const Instance& instance);

    // Generic constructor for custom surface creation (advanced use)
    Surface(const Instance& instance, void* window_handle, void* platform_handle = nullptr);

    // Destructor
    ~Surface();

    // Move constructor and assignment
    Surface(Surface&& other) noexcept;
    Surface& operator=(Surface&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkSurfaceKHR by only allowing moving.
    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;

    [[nodiscard]] VkSurfaceKHR get_handle() const { return surface_; }
    
    // Implicit conversion to VkSurfaceKHR
    operator VkSurfaceKHR() const { return surface_; }

    // Check if the surface is valid
    [[nodiscard]] bool is_valid() const { return surface_ != VK_NULL_HANDLE; }

    // Get surface capabilities for a physical device
    VkSurfaceCapabilitiesKHR get_capabilities(VkPhysicalDevice physical_device) const;

    // Get surface formats for a physical device
    std::vector<VkSurfaceFormatKHR> get_formats(VkPhysicalDevice physical_device) const;

    // Get surface present modes for a physical device
    std::vector<VkPresentModeKHR> get_present_modes(VkPhysicalDevice physical_device) const;

    // Check if physical device supports presentation to this surface
    bool is_supported(VkPhysicalDevice physical_device, uint32_t queue_family_index) const;

    // Get drawable size from SDL window (useful for high DPI displays)
    void get_drawable_size(SDL_Window* window, int* width, int* height) const;

    // SDL2-specific utility functions
    static std::vector<const char*> get_required_instance_extensions(SDL_Window* window = nullptr);
    static bool is_vulkan_supported();

private:
    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    VkInstance instance_{VK_NULL_HANDLE}; // Reference to instance for cleanup
    SDL_Window* window_{nullptr}; // Reference to SDL window (optional)

    // Helper methods
    void create_surface_from_sdl(SDL_Window* window);
    void create_surface(void* window_handle, void* platform_handle);
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_PRESENTATION_SURFACE_HPP