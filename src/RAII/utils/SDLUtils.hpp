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
    static bool initialize_sdl_for_vulkan();

    // Cleanup SDL resources related to Vulkan usage
    static void quit_sdl();

    // Get required Vulkan instance extensions for SDL
    static std::vector<const char*> get_required_instance_extensions();

    // Check if Vulkan is supported by SDL
    static bool is_vulkan_supported();

    // Get Vulkan loader function (for volk initialization)
    static void* get_vulkan_loader();

    // Window management
    static SDL_Window* create_vulkan_window(const char* title,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            uint32_t flags = 0);

    // Get window size (logical size and drawable pixel size)
    static void get_window_size(SDL_Window* window, int* width, int* height);
    static void get_drawable_size(SDL_Window* window, int* width, int* height);

    // Handle window events that affect Vulkan rendering
    static bool handle_window_event(const void* event); // SDL_Event* but void* to avoid including SDL

    // Check if window was resized/minimized/restored
    static bool was_window_resized(const void* event);
    static bool was_window_minimized(const void* event);
    static bool was_window_restored(const void* event);

    // Get window from Vulkan surface (not yet implemented)
    static SDL_Window* get_window_from_surface(VkSurfaceKHR surface);

    // Utility to check window state
    static bool is_window_minimized(SDL_Window* window);
    static bool is_window_maximized(SDL_Window* window);
    static bool is_window_fullscreen(SDL_Window* window);

    // Convert SDL key/mouse events to engine-specific format (if needed)
    struct KeyEvent {
        int scancode_;
        int keycode_;
        bool pressed_;
        bool repeat_;
    };

    struct MouseEvent {
        int x_;
        int y_;
        int button_;
        bool pressed_;
        int clicks_;
    };

    static bool get_key_event(const void* sdl_event, KeyEvent& key_event);
    static bool get_mouse_event(const void* sdl_event, MouseEvent& mouse_event);

    // High DPI support
    static float get_display_scale(SDL_Window* window);
    static void get_display_dpi(SDL_Window* window, float* ddpi, float* hdpi, float* vdpi);

    // Monitor/display information
    static int get_display_count();
    static void get_display_bounds(int display_index, int* x, int* y, int* w, int* h);
    static void get_display_usable_bounds(int display_index, int* x, int* y, int* w, int* h);

    // Performance timing
    static uint64_t get_performance_counter();
    static uint64_t get_performance_frequency();
    static double get_elapsed_time(uint64_t start, uint64_t end);

    // Clipboard support
    static bool has_clipboard_text();
    static std::string get_clipboard_text();
    static void set_clipboard_text(const std::string& text);

    // Error handling
    static std::string get_sdl_error();
    static void clear_sdl_error();

    // Get SDL version string
    // If linked is true, returns the version of the linked SDL library; otherwise, returns the compiled version.
    static std::string get_sdl_version_string(bool linked = true);
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

    [[nodiscard]] bool is_valid() const { return initialized_; }

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

    [[nodiscard]] SDL_Window* get_handle() const { return window_; }
    operator SDL_Window*() const { return window_; }

    [[nodiscard]] bool is_valid() const { return window_ != nullptr; }

    void get_size(int* width, int* height) const;
    void get_drawable_size(int* width, int* height) const;
    void set_title(const char* title);

private:
    SDL_Window* window_{nullptr};
};

} // namespace VulkanEngine::RAII::Utils

#endif // VULKAN_RAII_UTILS_SDL_UTILS_HPP
