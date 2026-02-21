#include "SDL3Application.hpp"

#include "utils/SDLUtils.hpp"
#include "utils/Timer.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <iostream>
#include <stdexcept>
#include <utility>

namespace VulkanEngine::RAII {

namespace {
uint32_t EnsureWindowFlags(uint32_t flags) {
    constexpr uint32_t DEFAULT_FLAGS = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    if (flags == 0) {
        return DEFAULT_FLAGS;
    }
    return (flags | SDL_WINDOW_VULKAN);
}
} // namespace

SDL3Application::SDL3Application(SDL3ApplicationConfig config)
    : config_(std::move(config)) {}

SDL3Application::~SDL3Application() {
    ShutdownInternal(false);
}

SDL3Application::SDL3Application(SDL3Application&& other) noexcept
    : config_(std::move(other.config_)),
      initialized_(other.initialized_),
      running_(other.running_),
      lastTime_(other.lastTime_),
      deltaTime_(other.deltaTime_),
      frameCount_(other.frameCount_),
      frame_times_(std::move(other.frame_times_)),
      current_frame_sum_(other.current_frame_sum_),
      sdlContext_(std::move(other.sdlContext_)),
      window_(std::move(other.window_)) {
    other.initialized_ = false;
    other.running_ = false;
    other.lastTime_ = 0;
    other.deltaTime_ = 0.0;
    other.frameCount_ = 0;
    other.current_frame_sum_ = 0.0;
}

SDL3Application& SDL3Application::operator=(SDL3Application&& other) noexcept {
    if (this != &other) {
        Shutdown();

        config_ = std::move(other.config_);
        initialized_ = other.initialized_;
        running_ = other.running_;
        lastTime_ = other.lastTime_;
        deltaTime_ = other.deltaTime_;
        frameCount_ = other.frameCount_;
        frame_times_ = std::move(other.frame_times_);
        current_frame_sum_ = other.current_frame_sum_;

        sdlContext_ = std::move(other.sdlContext_);
        window_ = std::move(other.window_);

        other.initialized_ = false;
        other.running_ = false;
        other.lastTime_ = 0;
        other.deltaTime_ = 0.0;
        other.frameCount_ = 0;
        other.current_frame_sum_ = 0.0;
    }
    return *this;
}

bool SDL3Application::Initialize() {
    if (initialized_) {
        return true;
    }

    try {
        if (!Utils::SDLUtils::InitializeSdlForVulkan()) {
            std::cerr << "Failed to initialize SDL for Vulkan" << '\n' << std::flush;
            return false;
        }

        std::cout << "linked SDL Version: " << Utils::SDLUtils::GetSdlVersionString(true) << '\n';
        std::cout << "compiled SDL Version: " << Utils::SDLUtils::GetSdlVersionString(false) << '\n';

        sdlContext_ = std::make_unique<Utils::SDLContext>(SDL_INIT_VIDEO);
        if (!sdlContext_ || !sdlContext_->IsValid()) {
            std::cerr << "Failed to initialize SDL context" << '\n' << std::flush;
            return false;
        }

        bool hint_result = SDL_SetHint(SDL_HINT_VIDEO_WAYLAND_MODE_SCALING, "none");
        if (!hint_result) {
            std::cerr << "Failed to set SDL hint for Wayland mode scaling" << '\n' << SDL_GetError() << '\n';
        } else {
            std::cout << "Successfully set SDL hint for Wayland mode scaling" << '\n' << std::flush;
        }

        uint32_t flags = EnsureWindowFlags(config_.windowFlags);
        window_ = std::make_unique<Utils::SDLWindow>(config_.windowTitle.c_str(),
                                                     config_.windowX,
                                                     config_.windowY,
                                                     config_.windowWidth,
                                                     config_.windowHeight,
                                                     flags);
        if (!window_ || !window_->IsValid()) {
            std::cerr << "Failed to create SDL window" << '\n' << std::flush;
            return false;
        }

        if (!OnInitialize()) {
            return false;
        }

        if (config_.initCallback) {
            config_.initCallback();
        }

        lastTime_ = Utils::SDLUtils::GetPerformanceCounter();
        deltaTime_ = 0.0;
        frameCount_ = 0;
        current_frame_sum_ = 0.0;
        while (!frame_times_.empty()) {
            frame_times_.pop();
        }

        initialized_ = true;
        running_ = true;

        std::flush(std::cout);

        return true;
    } catch (const std::exception& ex) {
        std::cerr << "SDL3Application initialization failed: " << ex.what() << '\n' << std::flush;
        Cleanup();
        return false;
    }
}

void SDL3Application::Run() {
    if (!initialized_ && !Initialize()) {
        return;
    }

    Utils::Timer frame_timer;
    while (running_) {
        ProcessEvents();
        if (!running_) {
            break;
        }

        UpdateTiming();

        if (config_.updateCallback) {
            config_.updateCallback(deltaTime_);
        }
        OnUpdate(deltaTime_);

        if (config_.renderCallback) {
            config_.renderCallback();
        }
        OnRender();

        ++frameCount_;

        frame_timer.Stop();
        double frame_time = frame_timer.ElapsedSeconds();
        frame_timer.Reset();
        frame_timer.Start();

        frame_times_.push(frame_time);
        current_frame_sum_ += frame_time;
        if (frame_times_.size() > config_.frame_time_sample_count) {
            current_frame_sum_ -= frame_times_.front();
            frame_times_.pop();
        }
    }
}

void SDL3Application::Shutdown() {
    ShutdownInternal(true);
}

void SDL3Application::ShutdownInternal(bool call_callbacks) {
    if (!initialized_) {
        return;
    }

    running_ = false;

    if (call_callbacks) {
        OnShutdown();
        if (config_.cleanupCallback) {
            config_.cleanupCallback();
        }
    }

    Cleanup();
}

::SDL_Window* SDL3Application::GetWindow() const {
    return window_ ? window_->GetHandle() : nullptr;
}

void SDL3Application::GetWindowSize(int* width, int* height) const {
    if (window_) {
        window_->GetSize(width, height);
    }
}

void SDL3Application::GetDrawableSize(int* width, int* height) const {
    if (window_) {
        window_->GetDrawableSize(width, height);
    }
}

void SDL3Application::SetWindowTitle(const char* title) {
    if (window_) {
        window_->SetTitle(title);
    }
}

void SDL3Application::UpdateConfig(const SDL3ApplicationConfig& new_config) {
    bool title_changed = new_config.windowTitle != config_.windowTitle;
    config_ = new_config;

    if (title_changed && window_) {
        window_->SetTitle(config_.windowTitle.c_str());
    }
}

void SDL3Application::ProcessEvents() {
    ::SDL_Event event{};
    Utils::Timer event_timer;
    event_timer.Reset();
    while (SDL_PollEvent(&event)) {
        event_timer.Stop();
        double event_processing_time_us = event_timer.ElapsedMicroseconds();
        if (event_processing_time_us > 500.0) {
            std::cout << "Event processing took a long time: " << event_processing_time_us << " microseconds" << '\n' << std::flush;
        }
        if (config_.eventCallback) {
            config_.eventCallback(event);
        }
        OnEvent(event);

        switch (event.type) {
            case SDL_EVENT_QUIT:
                running_ = false;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (event.window.windowID == SDL_GetWindowID(GetWindow())) {
                    running_ = false;
                }
                break;
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                HandleWindowResize();
                if (config_.resizeCallback) {
                    config_.resizeCallback(event.window.data1, event.window.data2);
                }
                OnResize(event.window.data1, event.window.data2);
                break;
            default:
                break;
        }

        if (!running_) {
            break;
        }

        event_timer.Reset();
    }
}

void SDL3Application::UpdateTiming() {
    uint64_t current_time = Utils::SDLUtils::GetPerformanceCounter();
    uint64_t frequency = Utils::SDLUtils::GetPerformanceFrequency();
    if (lastTime_ != 0 && frequency > 0) {
        deltaTime_ = Utils::SDLUtils::GetElapsedTime(lastTime_, current_time);
    } else {
        deltaTime_ = 0.0;
    }
    lastTime_ = current_time;
}

void SDL3Application::Cleanup() {
    window_.reset();

    Utils::SDLUtils::QuitSdl();
    sdlContext_.reset();

    initialized_ = false;
}

} // namespace VulkanEngine::RAII
