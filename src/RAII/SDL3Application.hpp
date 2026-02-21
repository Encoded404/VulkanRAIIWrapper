#ifndef VULKAN_RAII_SDL3_APPLICATION_HPP
#define VULKAN_RAII_SDL3_APPLICATION_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <string>

namespace VulkanEngine::RAII {

namespace Utils {
    class SDLContext;
    class SDLWindow;
}

struct SDL_Window;
union SDL_Event;

struct SDL3ApplicationConfig {
    std::string windowTitle = "Vulkan Engine Application";
    int windowWidth = 1280;
    int windowHeight = 720;
    int windowX = -1;
    int windowY = -1;
    uint32_t windowFlags = 0;

    std::function<void(double)> updateCallback;
    std::function<void()> renderCallback;
    std::function<void(const ::SDL_Event&)> eventCallback;
    std::function<void()> initCallback;
    std::function<void()> cleanupCallback;
    std::function<void(int width, int height)> resizeCallback;

    uint16_t frame_time_sample_count = 120;
};

class SDL3Application {
public:
    explicit SDL3Application(SDL3ApplicationConfig config = {});
    ~SDL3Application();

    SDL3Application(const SDL3Application&) = delete;
    SDL3Application& operator=(const SDL3Application&) = delete;

    SDL3Application(SDL3Application&& other) noexcept;
    SDL3Application& operator=(SDL3Application&& other) noexcept;

    bool Initialize();
    void Run();
    void Shutdown();

    [[nodiscard]] bool IsRunning() const { return running_; }
    void RequestExit() { running_ = false; }

    [[nodiscard]] ::SDL_Window* GetWindow() const;

    void GetWindowSize(int* width, int* height) const;
    void GetDrawableSize(int* width, int* height) const;
    void SetWindowTitle(const char* title);

    [[nodiscard]] double GetDeltaTime() const { return deltaTime_; }
    [[nodiscard]] double GetLastFps() const { return deltaTime_ > 0.0 ? 1.0 / deltaTime_ : 0.0; }
    [[nodiscard]] uint64_t GetFrameCount() const { return frameCount_; }
    [[nodiscard]] double GetAverageFrameTime() const {
        return frame_times_.empty() ? 0.0 : current_frame_sum_ / static_cast<double>(frame_times_.size());
    }

    void UpdateConfig(const SDL3ApplicationConfig& new_config);

protected:
    virtual bool OnInitialize() { return true; }
    virtual void OnUpdate(double delta_time) {}
    virtual void OnRender() {}
    virtual void OnEvent(const ::SDL_Event& event) {}
    virtual void OnResize(int width, int height) {}
    virtual void OnShutdown() {}

    virtual void HandleWindowResize();

private:
    void ProcessEvents();
    void UpdateTiming();
    void Cleanup();
    void ShutdownInternal(bool call_callbacks);

    SDL3ApplicationConfig config_;
    bool initialized_{false};
    bool running_{false};

    uint64_t lastTime_{0};
    double deltaTime_{0.0};
    uint64_t frameCount_{0};

    std::queue<double> frame_times_;
    double current_frame_sum_{0.0};

    std::unique_ptr<Utils::SDLContext> sdlContext_;
    std::unique_ptr<Utils::SDLWindow> window_;
};

} // namespace VulkanEngine::RAII

#endif // VULKAN_RAII_SDL3_APPLICATION_HPP
