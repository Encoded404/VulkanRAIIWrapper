#ifndef VULKAN_RAII_SDL2_APPLICATION_HPP
#define VULKAN_RAII_SDL2_APPLICATION_HPP

#include <volk.h>
#include <string>
#include <memory>
#include <functional>
#include <cstdint>
#include <queue>

#include "utils/CapabilityUtils.hpp"

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
    std::string windowTitle = "Vulkan Engine Application";
    int windowWidth = 1280;
    int windowHeight = 720;
    int windowX = -1; // SDL_WINDOWPOS_CENTERED
    int windowY = -1; // SDL_WINDOWPOS_CENTERED
    uint32_t windowFlags = 0; // Defaults to a resizable Vulkan window when left at 0
    
    bool enableValidation = true;
    bool enableVSync = true;
    uint32_t maxFramesInFlight = 3;

    std::vector<Utils::NamedCapabilityRequest> validationLayers = {}; // Custom validation layers (if empty, default ones will be used)
    std::vector<Utils::NamedCapabilityRequest> instanceExtensions = {}; // Additional instance extensions to enable
    std::vector<Utils::NamedCapabilityRequest> deviceExtensions = {}; // Additional device extensions to enable
    VkPhysicalDeviceFeatures requiredDeviceFeatures = {}; // Required physical device features
    VkPhysicalDeviceFeatures optionalDeviceFeatures = {}; // Optional physical device features
    
    // Optional application callbacks
    std::function<void(double delta_time)> updateCallback;
    std::function<void()> renderCallback;
    std::function<void(const SDL_Event&)> eventCallback;
    std::function<void()> initCallback;
    std::function<void()> cleanupCallback;
    std::function<void(int width, int height)> resizeCallback;

    uint16_t frame_time_sample_count = 120; // number of frames to average for frame time calculation
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
    bool Initialize();

    // Run the application main loop
    void Run();

    // Shutdown the application
    void Shutdown();

    // Check if application is running
    [[nodiscard]] bool IsRunning() const { return running_; }

    // Request application exit
    void RequestExit() { running_ = false; }

    // Access to underlying Vulkan objects
    [[nodiscard]] Instance* GetInstance() const { return instance_.get(); }
    [[nodiscard]] PhysicalDevice* GetPhysicalDevice() const { return physicalDevice_.get(); }
    [[nodiscard]] Device* GetDevice() const { return device_.get(); }
    [[nodiscard]] Surface* GetSurface() const { return surface_.get(); }
    [[nodiscard]] Swapchain* GetSwapchain() const { return swapchain_.get(); }
    [[nodiscard]] RenderPass* GetRenderPass() const { return renderPass_.get(); }
    [[nodiscard]] Renderer* GetRenderer() const { return renderer_.get(); }

    // Read-only access to configuration (for derived classes/utilities)
    [[nodiscard]] const SDLApplicationConfig& GetConfig() const { return config_; }

    // Access to SDL objects
    [[nodiscard]] SDL_Window* GetWindow() const;

    // Window management
    void GetWindowSize(int* width, int* height) const;
    void GetDrawableSize(int* width, int* height) const;
    void SetWindowTitle(const char* title);

    // Frame timing
    [[nodiscard]] double GetDeltaTime() const { return deltaTime_; }
    [[nodiscard]] double GetLastFps() const { return 1.0 / deltaTime_; } // return the fps extrapolated from a single frame via delta time.
    [[nodiscard]] uint64_t GetFrameCount() const { return frameCount_; }
    [[nodiscard]] inline double GetAverageFrameTime() const { return current_frame_sum_ / static_cast<double>(frame_times_.size()); }; // average frame time over last 120 frames in seconds

    // Update configuration (some changes require restart)
    void UpdateConfig(const SDLApplicationConfig& new_config);

protected:
    // Virtual methods for customization
    virtual bool OnInitialize() { return true; }
    virtual void OnUpdate(double delta_time) {}
    virtual void OnRender() {}
    virtual void OnEvent(const SDL_Event& event) {}
    virtual void OnResize(int width, int height) {}
    virtual void OnShutdown() {}

    // Helper methods
    virtual bool CreateVulkanObjects();
    virtual void CreateRenderPass();
    void SetRenderPass(std::unique_ptr<RenderPass> render_pass);
    virtual void HandleWindowResize();

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
    void ProcessEvents();
    void UpdateTiming();
    // bool recreate_swapchain();
    void Cleanup();
    void ShutdownInternal(bool call_callbacks);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_SDL2_APPLICATION_HPP