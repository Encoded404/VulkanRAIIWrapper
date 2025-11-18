#include "SDL2Application.hpp"

#include "core/instance.hpp"
#include "core/PhysicalDevice.hpp"
#include "core/Device.hpp"
#include "presentation/Surface.hpp"
#include "presentation/Swapchain.hpp"
#include "rendering/RenderPass.hpp"
#include "rendering/Renderer.hpp"
#include "utils/SDLUtils.hpp"
#include "utils/Constants.hpp"
#include "utils/Timer.hpp"

// #include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_hints.h>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#include <queue>

 

namespace VulkanEngine::RAII {

namespace {
constexpr uint32_t DEFAULT_APPLICATION_VERSION = VK_MAKE_VERSION(1, 0, 0);

std::vector<const char*> gather_validation_layers(bool enable) {
    if (!enable) {
        return {};
    }
    return {Constants::KHRONOS_VALIDATION_LAYER};
}

VkPresentModeKHR choose_present_mode(bool enable_v_sync) {
    return enable_v_sync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
}

uint32_t ensure_window_flags(uint32_t flags) {
    constexpr uint32_t DEFAULT_FLAGS = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    if (flags == 0) {
        return DEFAULT_FLAGS;
    }
    return (flags | SDL_WINDOW_VULKAN);
}

} // namespace

SDLApplication::SDLApplication(SDLApplicationConfig config)
    : config_(std::move(config)) {}

SDLApplication::~SDLApplication() {
    shutdown_internal(false);
}

SDLApplication::SDLApplication(SDLApplication&& other) noexcept
    : config_(std::move(other.config_)),
      initialized_(other.initialized_),
      running_(other.running_),
      lastTime_(other.lastTime_),
      deltaTime_(other.deltaTime_),
      frameCount_(other.frameCount_),
      sdlContext_(std::move(other.sdlContext_)),
      window_(std::move(other.window_)),
      instance_(std::move(other.instance_)),
      physicalDevice_(std::move(other.physicalDevice_)),
      device_(std::move(other.device_)),
      surface_(std::move(other.surface_)),
      swapchain_(std::move(other.swapchain_)),
      renderPass_(std::move(other.renderPass_)),
      renderer_(std::move(other.renderer_)) {
    other.initialized_ = false;
    other.running_ = false;
    other.lastTime_ = 0;
    other.deltaTime_ = 0.0;
    other.frameCount_ = 0;
}

SDLApplication& SDLApplication::operator=(SDLApplication&& other) noexcept {
    if (this != &other) {
        shutdown();

        config_ = std::move(other.config_);
        initialized_ = other.initialized_;
        running_ = other.running_;
        lastTime_ = other.lastTime_;
        deltaTime_ = other.deltaTime_;
        frameCount_ = other.frameCount_;

        sdlContext_ = std::move(other.sdlContext_);
        window_ = std::move(other.window_);
        instance_ = std::move(other.instance_);
        physicalDevice_ = std::move(other.physicalDevice_);
        device_ = std::move(other.device_);
        surface_ = std::move(other.surface_);
        swapchain_ = std::move(other.swapchain_);
        renderPass_ = std::move(other.renderPass_);
        renderer_ = std::move(other.renderer_);

        other.initialized_ = false;
        other.running_ = false;
        other.lastTime_ = 0;
        other.deltaTime_ = 0.0;
        other.frameCount_ = 0;
    }
    return *this;
}

bool SDLApplication::initialize() {
    if (initialized_) {
        return true;
    }

    try {
        if (!Utils::SDLUtils::initialize_sdl_for_vulkan()) {
            std::cerr << "Failed to initialize SDL for Vulkan" << '\n' << std::flush;
            return false;
        }

        std::cout << "linked SDL Version: " << Utils::SDLUtils::get_sdl_version_string(true) << '\n';
        std::cout << "compiled SDL Version: " << Utils::SDLUtils::get_sdl_version_string(false) << '\n';

        sdlContext_ = std::make_unique<Utils::SDLContext>(SDL_INIT_VIDEO);
        if (!sdlContext_ || !sdlContext_->is_valid()) {
            std::cerr << "Failed to initialize SDL context" << '\n';
            return false;
        }

        // Ensure Wayland mode scaling behaves as desired; must be set before creating the window.
        // This is a no-op on non-Wayland platforms.
        bool result = SDL_SetHint(SDL_HINT_VIDEO_WAYLAND_MODE_SCALING, "none");
        if(result == false) {
            std::cerr << "Failed to set SDL hint for Wayland mode scaling" << '\n' << SDL_GetError() << '\n';
        }
        else
        {
            std::cout << "Successfully set SDL hint for Wayland mode scaling" << '\n' << std::flush;
        }

        uint32_t flags = ensure_window_flags(config_.windowFlags_);
        window_ = std::make_unique<Utils::SDLWindow>(config_.windowTitle_.c_str(),
                                                     config_.windowX_,
                                                     config_.windowY_,
                                                     config_.windowWidth_,
                                                     config_.windowHeight_,
                                                     flags);
        if (!window_ || !window_->is_valid()) {
            std::cerr << "Failed to create SDL window" << '\n' << std::flush;
            return false;
        }

        if (!create_vulkan_objects()) {
            return false;
        }

        if (!on_initialize()) {
            return false;
        }

        if (config_.initCallback_) {
            config_.initCallback_();
        }

        lastTime_ = Utils::SDLUtils::get_performance_counter();
        deltaTime_ = 0.0;
        frameCount_ = 0;

        initialized_ = true;
        running_ = true;

        std::flush(std::cout);

        return true;
    } catch (const std::exception& ex) {
        std::cerr << "SDLApplication initialization failed: " << ex.what() << '\n' << std::flush;
        cleanup();
        return false;
    }
}

void SDLApplication::run() {
    if (!initialized_ && !initialize()) {
        return;
    }

    Utils::Timer frame_timer;
    while (running_) {

        process_events();
        if (!running_) {
            break;
        }

        update_timing();

        /* if (renderer_ && renderer_->needs_swapchain_recreation()) {
            if (!recreate_swapchain()) {
                continue;
            }
        } */

        if (config_.updateCallback_) {
            config_.updateCallback_(deltaTime_);
        }
        //std::cout << "current frame time sum: " << current_frame_sum_ << '\n' << std::flush;
        on_update(deltaTime_);

        if (renderer_) {
            if (!renderer_->begin_frame())
            {
                continue;
            }

            if (config_.renderCallback_) {
                config_.renderCallback_();
            }

            on_render();

            renderer_->end_frame();
        } else {
            if (config_.renderCallback_) {
                config_.renderCallback_();
            }
            on_render();
        }

        ++frameCount_;

        frame_timer.stop();
        double frame_time = frame_timer.elapsed_seconds();
        // Reset and start the frame timer at the beginning of each frame. we do it here instead of the start to capture the time taken by event processing and looping as well.
        // Note: reset() clears accumulated time but does not start the timer.
        frame_timer.reset();
        frame_timer.start();

        frame_times_.push(frame_time);
        current_frame_sum_ += frame_time;
        if ((uint16_t)frame_times_.size() > config_.frame_time_sample_count_) {
            current_frame_sum_ -= frame_times_.front();
            frame_times_.pop();
        }
    }
}

void SDLApplication::shutdown() {
    shutdown_internal(true);
}

void SDLApplication::shutdown_internal(bool call_callbacks) {
    if (!initialized_) {
        return;
    }

    running_ = false;

    if (call_callbacks) {
        // Only invoke overridable hooks when the most-derived type is still alive.
        on_shutdown();
        if (config_.cleanupCallback_) {
            config_.cleanupCallback_();
        }
    }

    cleanup();
}

SDL_Window* SDLApplication::get_window() const {
    return window_ ? static_cast<SDL_Window*>(*window_) : nullptr;
}

void SDLApplication::get_window_size(int* width, int* height) const {
    if (window_) {
        window_->get_size(width, height);
    }
}

void SDLApplication::get_drawable_size(int* width, int* height) const {
    if (window_) {
        window_->get_drawable_size(width, height);
    }
}

void SDLApplication::set_window_title(const char* title) {
    if (window_) {
        window_->set_title(title);
    }
}

void SDLApplication::update_config(const SDLApplicationConfig& new_config) {
    bool title_changed = new_config.windowTitle_ != config_.windowTitle_;
    bool vsync_changed = new_config.enableVSync_ != config_.enableVSync_;
    config_ = new_config;

    if (title_changed && window_) {
        window_->set_title(config_.windowTitle_.c_str());
    }

    if (vsync_changed && swapchain_) {
        swapchain_->mark_for_recreation();
        if (renderer_) {
            renderer_->mark_swapchain_for_recreation();
        }
    }
}

bool SDLApplication::create_vulkan_objects() {
    SDL_Window* window = get_window();
    if (!window) {
        throw std::runtime_error("Window must be created before Vulkan objects");
    }

    std::vector<const char*> required_extensions = Utils::SDLUtils::get_required_instance_extensions();
    auto validation_layers = gather_validation_layers(config_.enableValidation_);

    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pApplicationName = config_.windowTitle_.c_str();
    app_info.applicationVersion = DEFAULT_APPLICATION_VERSION;

    instance_ = std::make_unique<Instance>(app_info.pApplicationName,
                                            app_info.applicationVersion,
                                            required_extensions,
                                            validation_layers);

    surface_ = std::make_unique<Surface>(*instance_, window);

    physicalDevice_ = std::make_unique<PhysicalDevice>(*instance_, surface_->get_handle());

    std::vector<const char*> device_extensions = {Constants::SWAPCHAIN_EXTENSION};
    VkPhysicalDeviceFeatures features{};

    device_ = std::make_unique<Device>(*physicalDevice_, device_extensions, features, validation_layers);

    VkPresentModeKHR present_mode = choose_present_mode(config_.enableVSync_);
    VkSurfaceFormatKHR preferred_format{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    swapchain_ = std::make_unique<Swapchain>(*device_, *surface_, window, present_mode, preferred_format);

    create_render_pass();

    renderer_ = std::make_unique<Renderer>(*device_, *swapchain_, *renderPass_, config_.maxFramesInFlight_);

    return true;
}

void SDLApplication::create_render_pass() {
    if (!device_ || !swapchain_) {
        throw std::runtime_error("Renderer dependencies not initialised");
    }

    VkFormat color_format = swapchain_->get_image_format();
    renderPass_ = std::make_unique<RenderPass>(*device_, color_format);
}

void SDLApplication::handle_window_resize() {
    if (swapchain_) {
        swapchain_->mark_for_recreation();
    }
}

void SDLApplication::process_events() {
    SDL_Event event{};
    Utils::Timer event_timer;
    event_timer.reset();
    while (SDL_PollEvent(&event)) {
        event_timer.stop();
        double event_processing_time_us = event_timer.elapsed_microseconds();
        if(event_processing_time_us > 500.0)
        {
            std::cout << "Event processing took a long time: " << event_processing_time_us << " microseconds" << '\n' << std::flush;
        }
        if (config_.eventCallback_) {
            config_.eventCallback_(event);
        }
        on_event(event);

        switch (event.type) {
            case SDL_EVENT_QUIT:
                running_ = false;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (event.window.windowID == SDL_GetWindowID(get_window())) {
                    running_ = false;
                }
                break;
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                handle_window_resize();
                if (config_.resizeCallback_) {
                    config_.resizeCallback_(event.window.data1, event.window.data2);
                }
                on_resize(event.window.data1, event.window.data2);
                break;
            default:
                break;
        }

        if (!running_) {
            break;
        }
    }
}

void SDLApplication::update_timing() {
    uint64_t current_time = Utils::SDLUtils::get_performance_counter();
    uint64_t frequency = Utils::SDLUtils::get_performance_frequency();
    // Avoid repeating the same branch body: prefer a single assignment for the
    // default case and compute elapsed time only when we have a valid
    // previous timestamp and a non-zero frequency.
    if (lastTime_ != 0 && frequency > 0) {
        deltaTime_ = Utils::SDLUtils::get_elapsed_time(lastTime_, current_time);
    } else {
        deltaTime_ = 0.0;
    }
    lastTime_ = current_time;
}

/* bool SDLApplication::recreate_swapchain() {
    if (!swapchain_ || !renderer_ || !device_) {
        return false;
    }

    SDL_Window* window = get_window();
    int width = 0;
    int height = 0;
    if (!SDL_GetWindowSizeInPixels(window, &width, &height)) {
        std::cerr << "Failed to query window pixel size: " << SDL_GetError() << '\n' << std::flush;
        return false;
    }

    if (width == 0 || height == 0) {
        return false;
    }

    device_->wait_idle();

    try {
        swapchain_->recreate(window);
        create_render_pass();
        renderer_->recreate(*swapchain_);
        if (config_.resizeCallback_) {
            config_.resizeCallback_(width, height);
        }
        on_resize(width, height);
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to recreate swapchain: " << ex.what() << '\n' << std::flush;
        return false;
    }
} */

void SDLApplication::cleanup() {
    if (device_) {
        try
        {
            device_->wait_idle();
        } catch (...) {
            // print the error as a warning
            std::cerr << "Warning: Failed to wait for device idle" << '\n' << std::flush;
            // Ignore errors during shutdown
        }
    }

    renderer_.reset();
    renderPass_.reset();
    swapchain_.reset();
    surface_.reset();
    device_.reset();
    physicalDevice_.reset();
    instance_.reset();

    window_.reset();

    Utils::SDLUtils::quit_sdl();
    sdlContext_.reset();

    initialized_ = false;
}

} // namespace VulkanEngine::RAII

