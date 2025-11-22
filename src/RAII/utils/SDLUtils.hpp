#ifndef VULKAN_RAII_UTILS_SDL_UTILS_HPP
#define VULKAN_RAII_UTILS_SDL_UTILS_HPP

#include <volk.h>
#include <string>
#include <vector>

// Forward declare SDL types to avoid requiring SDL headers in all files
struct SDL_Window;

namespace VulkanEngine::RAII::Utils {

// SDL-specific Vulkan utilities
class SDLUtils {
public:
    // Initialize SDL for Vulkan usage (initializes SDL if needed and loads the Vulkan loader)
    static bool InitializeSdlForVulkan();

    // Cleanup SDL resources related to Vulkan usage
    static void QuitSdl();

    // Get required Vulkan instance extensions for SDL
    static std::vector<const char*> GetRequiredInstanceExtensions();

    // Check if Vulkan is supported by SDL
    static bool IsVulkanSupported();

    // Get Vulkan loader function (for volk initialization)
    static void* GetVulkanLoader();

    // Window management
    static SDL_Window* CreateVulkanWindow(const char* title,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            uint32_t flags = 0);

    // Get window size (logical size and drawable pixel size)
    static void GetWindowSize(SDL_Window* window, int* width, int* height);
    static void GetDrawableSize(SDL_Window* window, int* width, int* height);

    // Handle window events that affect Vulkan rendering
    static bool HandleWindowEvent(const void* event); // SDL_Event* but void* to avoid including SDL

    // Check if window was resized/minimized/restored
    static bool WasWindowResized(const void* event);
    static bool WasWindowMinimized(const void* event);
    static bool WasWindowRestored(const void* event);

    // Get window from Vulkan surface (not yet implemented)
    static SDL_Window* GetWindowFromSurface(VkSurfaceKHR surface);

    // Utility to check window state
    static bool IsWindowMinimized(SDL_Window* window);
    static bool IsWindowMaximized(SDL_Window* window);
    static bool IsWindowFullscreen(SDL_Window* window);

    // Convert SDL key/mouse events to engine-specific format (if needed)
    struct KeyEvent {
        int scancode;
        int keycode;
        bool pressed;
        bool repeat;
    };

    struct MouseEvent {
        int x;
        int y;
        int button;
        bool pressed;
        int clicks;
    };

    static bool GetKeyEvent(const void* sdl_event, KeyEvent& key_event);
    static bool GetMouseEvent(const void* sdl_event, MouseEvent& mouse_event);

    // High DPI support
    static float GetDisplayScale(SDL_Window* window);
    static void GetDisplayDpi(SDL_Window* window, float* ddpi, float* hdpi, float* vdpi);

    // Monitor/display information
    static int GetDisplayCount();
    static void GetDisplayBounds(int display_index, int* x, int* y, int* w, int* h);
    static void GetDisplayUsableBounds(int display_index, int* x, int* y, int* w, int* h);

    // Performance timing
    static uint64_t GetPerformanceCounter();
    static uint64_t GetPerformanceFrequency();
    static double GetElapsedTime(uint64_t start, uint64_t end);

    // Clipboard support
    static bool HasClipboardText();
    static std::string GetClipboardText();
    static void SetClipboardText(const std::string& text);

    // Error handling
    static std::string GetSdlError();
    static void ClearSdlError();

    // Get SDL version string
    // If linked is true, returns the version of the linked SDL library; otherwise, returns the compiled version.
    static std::string GetSdlVersionString(bool linked = true);
private:
    static bool s_initialized;
    static bool s_vulkan_supported;
};

// RAII wrapper for SDL initialization
class SDLContext {
public:
    explicit SDLContext(uint32_t flags = 0); // SDL_INIT_VIDEO by default
    ~SDLContext();

    // Move constructor and assignment
    SDLContext(SDLContext&& other) noexcept;
    SDLContext& operator=(SDLContext&& other) noexcept;

    // Delete copy constructor and assignment
    SDLContext(const SDLContext&) = delete;
    SDLContext& operator=(const SDLContext&) = delete;

    [[nodiscard]] bool IsValid() const { return initialized_; }

private:
    bool initialized_{false};
};

// RAII wrapper for SDL window
class SDLWindow {
public:
    SDLWindow(const char* title, int x, int y, int width, int height, uint32_t flags = 0);
    ~SDLWindow();

    // Move constructor and assignment
    SDLWindow(SDLWindow&& other) noexcept;
    SDLWindow& operator=(SDLWindow&& other) noexcept;

    // Delete copy constructor and assignment
    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

    [[nodiscard]] SDL_Window* GetHandle() const { return window_; }
    operator SDL_Window*() const { return window_; }

    [[nodiscard]] bool IsValid() const { return window_ != nullptr; }

    void GetSize(int* width, int* height) const;
    void GetDrawableSize(int* width, int* height) const;
    void SetTitle(const char* title);

private:
    SDL_Window* window_{nullptr};
};

} // namespace VulkanEngine::RAII::Utils

#endif // VULKAN_RAII_UTILS_SDL_UTILS_HPP
