#ifndef VULKAN_RAII_SDL2_APPLICATION_HPP
#define VULKAN_RAII_SDL2_APPLICATION_HPP

#include <volk.h>
#include <string>
#include <memory>
#include <functional>
#include <cstdint>
#include <queue>

// Forward declare SDL types
struct SDL_Window;
union SDL_Event;


namespace VulkanEngine::RAII {

// Forward declarations
class Instance;
class PhysicalDevice;
class Device;
class Surface;
class Swapchain;
class RenderPass;
class Renderer;

namespace Utils {
class SDLContext;
class SDLWindow;
}

// Configuration structure for SDL Vulkan application
struct SDLApplicationConfig {
    std::string windowTitle_ = "Vulkan Engine Application";
    int windowWidth_ = 1280;
    int windowHeight_ = 720;
    int windowX_ = -1; // SDL_WINDOWPOS_CENTERED
    int windowY_ = -1; // SDL_WINDOWPOS_CENTERED
    uint32_t windowFlags_ = 0; // Defaults to a resizable Vulkan window when left at 0
    
    bool enableValidation_ = true;
    bool enableVSync_ = true;
    uint32_t maxFramesInFlight_ = 3;
    
    // Optional application callbacks
    std::function<void(double delta_time)> updateCallback_;
    std::function<void()> renderCallback_;
    std::function<void(const SDL_Event&)> eventCallback_;
    std::function<void()> initCallback_;
    std::function<void()> cleanupCallback_;
    std::function<void(int width, int height)> resizeCallback_;

    uint16_t frame_time_sample_count_ = 120; // number of frames to average for frame time calculation
};

// High-level SDL + Vulkan application class
class SDLApplication {
public:
    explicit SDLApplication(SDLApplicationConfig config = {});
    ~SDLApplication();

    // Delete copy constructor and assignment
    SDLApplication(const SDLApplication&) = delete;
    SDLApplication& operator=(const SDLApplication&) = delete;

    // Move constructor and assignment
    SDLApplication(SDLApplication&& other) noexcept;
    SDLApplication& operator=(SDLApplication&& other) noexcept;

    // Initialize the application
    bool initialize();

    // Run the application main loop
    void run();

    // Shutdown the application
    void shutdown();

    // Check if application is running
    [[nodiscard]] bool is_running() const { return running_; }

    // Request application exit
    void request_exit() { running_ = false; }

    // Access to underlying Vulkan objects
    [[nodiscard]] Instance* get_instance() const { return instance_.get(); }
    [[nodiscard]] PhysicalDevice* get_physical_device() const { return physicalDevice_.get(); }
    [[nodiscard]] Device* get_device() const { return device_.get(); }
    [[nodiscard]] Surface* get_surface() const { return surface_.get(); }
    [[nodiscard]] Swapchain* get_swapchain() const { return swapchain_.get(); }
    [[nodiscard]] RenderPass* get_render_pass() const { return renderPass_.get(); }
    [[nodiscard]] Renderer* get_renderer() const { return renderer_.get(); }

    // Read-only access to configuration (for derived classes/utilities)
    [[nodiscard]] const SDLApplicationConfig& get_config() const { return config_; }

    // Access to SDL objects
    [[nodiscard]] SDL_Window* get_window() const;

    // Window management
    void get_window_size(int* width, int* height) const;
    void get_drawable_size(int* width, int* height) const;
    void set_window_title(const char* title);

    // Frame timing
    [[nodiscard]] double get_delta_time() const { return deltaTime_; }
    [[nodiscard]] double get_last_fps() const { return 1.0 / deltaTime_; } // return the fps extrapolated from a single frame via delta time.
    [[nodiscard]] uint64_t get_frame_count() const { return frameCount_; }
    [[nodiscard]] inline double get_average_frame_time() const { return current_frame_sum_ / static_cast<double>(frame_times_.size()); }; // average frame time over last 120 frames in seconds

    // Update configuration (some changes require restart)
    void update_config(const SDLApplicationConfig& new_config);

protected:
    // Virtual methods for customization
    virtual bool on_initialize() { return true; }
    virtual void on_update(double delta_time) {}
    virtual void on_render() {}
    virtual void on_event(const SDL_Event& event) {}
    virtual void on_resize(int width, int height) {}
    virtual void on_shutdown() {}

    // Helper methods
    virtual bool create_vulkan_objects();
    virtual void create_render_pass();
    virtual void handle_window_resize();

private:
    SDLApplicationConfig config_;
    bool initialized_{false};
    bool running_{false};
    
    // Timing
    uint64_t lastTime_{0};
    double deltaTime_{0.0};
    uint64_t frameCount_{0};

    // a queue to hold the last 120 frame times for averaging
    std::queue<double> frame_times_;
    double current_frame_sum_{0.0};
    
    // SDL objects
    std::unique_ptr<Utils::SDLContext> sdlContext_;
    std::unique_ptr<Utils::SDLWindow> window_;
    
    // Vulkan objects
    std::unique_ptr<Instance> instance_;
    std::unique_ptr<PhysicalDevice> physicalDevice_;
    std::unique_ptr<Device> device_;
    std::unique_ptr<Surface> surface_;
    std::unique_ptr<Swapchain> swapchain_;
    std::unique_ptr<RenderPass> renderPass_;
    std::unique_ptr<Renderer> renderer_;
    
    // Internal methods
    void process_events();
    void update_timing();
    // bool recreate_swapchain();
    void cleanup();
    void shutdown_internal(bool call_callbacks);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_SDL2_APPLICATION_HPP