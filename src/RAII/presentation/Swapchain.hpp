#ifndef VULKAN_RAII_PRESENTATION_SWAPCHAIN_HPP
#define VULKAN_RAII_PRESENTATION_SWAPCHAIN_HPP

#include <volk.h>
#include <vector>

// Forward declare SDL types
struct SDL_Window;


namespace VulkanEngine::RAII {

class Device; // Forward declaration
class Surface; // Forward declaration

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain {
public:
    // SDL2-specific constructor that automatically gets window size
    Swapchain(const Device& device, 
              const Surface& surface,
              SDL_Window* window,
              VkPresentModeKHR preferred_present_mode = VK_PRESENT_MODE_MAILBOX_KHR,
              VkSurfaceFormatKHR preferred_format = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});

    // Constructor that creates a swapchain for the given device and surface
    Swapchain(const Device& device, 
              const Surface& surface,
              uint32_t width, 
              uint32_t height,
              VkPresentModeKHR preferred_present_mode = VK_PRESENT_MODE_MAILBOX_KHR,
              VkSurfaceFormatKHR preferred_format = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});

    // Destructor
    ~Swapchain();

    // Move constructor and assignment
    Swapchain(Swapchain&& other) noexcept;
    Swapchain& operator=(Swapchain&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkSwapchainKHR by only allowing moving.
    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    [[nodiscard]] VkSwapchainKHR GetHandle() const { return swapchain_; }
    
    // Implicit conversion to VkSwapchainKHR
    operator VkSwapchainKHR() const { return swapchain_; }

    // Check if the swapchain is valid
    [[nodiscard]] bool IsValid() const { return swapchain_ != VK_NULL_HANDLE; }

    // Get swapchain images
    [[nodiscard]] std::vector<VkImage> GetImages() const;

    // Get swapchain image views
    [[nodiscard]] const std::vector<VkImageView>& GetImageViews() const { return imageViews_; }

    // Get swapchain format
    [[nodiscard]] VkFormat GetImageFormat() const { return imageFormat_; }

    // Get swapchain extent
    [[nodiscard]] VkExtent2D GetExtent() const { return extent_; }

    // Get image count
    [[nodiscard]] uint32_t GetImageCount() const { return static_cast<uint32_t>(imageViews_.size()); }

    // Acquire next image
    VkResult AcquireNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t& image_index) const;

    // Present image
    VkResult Present(const std::vector<VkSemaphore>& wait_semaphores, uint32_t image_index, VkQueue present_queue) const;

    // SDL2-specific recreate method that gets size from window
    void Recreate(SDL_Window* window = nullptr);

    void Recreate(uint32_t width, uint32_t height);

    // Check if recreate is needed (e.g., window was resized)
    [[nodiscard]] bool NeedsRecreate() const { return needsRecreate_; }

    // Mark swapchain as needing recreation (call this on window events)
    void MarkForRecreation() { needsRecreate_ = true; }

private:
    VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    VkSurfaceKHR surface_{VK_NULL_HANDLE}; // Reference to surface
    const Device* deviceRef_{nullptr};
    SDL_Window* window_{nullptr}; // Reference to SDL window (if created with SDL constructor)
    
    std::vector<VkImageView> imageViews_;
    VkFormat imageFormat_{VK_FORMAT_UNDEFINED};
    VkExtent2D extent_{0, 0};
    VkPresentModeKHR presentMode_{VK_PRESENT_MODE_FIFO_KHR};
    uint32_t minImageCount_{0};
    VkSurfaceFormatKHR surfaceFormat_{VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    bool needsRecreate_{false};

    // Helper methods
    void CreateSwapchain(uint32_t width, uint32_t height);
    void CreateImageViews();
    void Cleanup();
    
    SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physical_device) const;
    [[nodiscard]] VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) const;
    [[nodiscard]] VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) const;
    [[nodiscard]] VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) const;
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_PRESENTATION_SWAPCHAIN_HPP