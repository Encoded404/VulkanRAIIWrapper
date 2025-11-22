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
#include "utils/CapabilityUtils.hpp"

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
#include <sstream>
#include <string_view>
#include <algorithm>
#include <string>
#include <cstddef> // for std::size_t

namespace VulkanEngine::RAII {

namespace {
constexpr uint32_t DEFAULT_APPLICATION_VERSION = VK_MAKE_VERSION(1, 0, 0);

std::vector<Utils::NamedCapabilityRequest> GatherValidationLayers(bool enable) {
    if (!enable) {
        return {};
    }
    return {{Constants::KHRONOS_VALIDATION_LAYER, Utils::CapabilityRequirement::OPTIONAL}};
}

bool HasRequiredRequest(const std::vector<Utils::NamedCapabilityRequest>& requests) {
    return std::ranges::any_of(requests, [](const Utils::NamedCapabilityRequest& request) {
        return request.requirement == Utils::CapabilityRequirement::REQUIRED;
    });
}

std::string JoinNames(const std::vector<std::string>& names) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << names[i];
    }
    return oss.str();
}

void LogOptionalCapabilities(const std::vector<std::string>& names, std::string_view label) {
    if (names.empty()) {
        return;
    }
    std::cerr << "Optional " << label << " not available: " << JoinNames(names) << '\n' << std::flush;
}

std::vector<const char*> ToCStrVector(const std::vector<std::string>& names) {
    std::vector<const char*> result;
    result.reserve(names.size());
    for (const std::string& name : names) {
        result.push_back(name.c_str());
    }
    return result;
}

VkPresentModeKHR ChoosePresentMode(bool enable_v_sync) {
    return enable_v_sync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
}

uint32_t EnsureWindowFlags(uint32_t flags) {
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
    ShutdownInternal(false);
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
        Shutdown();

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

bool SDLApplication::Initialize() {
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

        if (!CreateVulkanObjects()) {
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

        initialized_ = true;
        running_ = true;

        std::flush(std::cout);

        return true;
    } catch (const std::exception& ex) {
        std::cerr << "SDLApplication initialization failed: " << ex.what() << '\n' << std::flush;
        Cleanup();
        return false;
    }
}

void SDLApplication::Run() {
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

        /* if (renderer_ && renderer_->needs_swapchain_recreation()) {
            if (!recreate_swapchain()) {
                continue;
            }
        } */

        if (config_.updateCallback) {
            config_.updateCallback(deltaTime_);
        }
        //std::cout << "current frame time sum: " << current_frame_sum_ << '\n' << std::flush;
        OnUpdate(deltaTime_);

        if (renderer_) {
            if (!renderer_->BeginFrame())
            {
                continue;
            }

            if (config_.renderCallback) {
                config_.renderCallback();
            }

            OnRender();

            renderer_->EndFrame();
        } else {
            if (config_.renderCallback) {
                config_.renderCallback();
            }
            OnRender();
        }

        ++frameCount_;

        frame_timer.Stop();
        double frame_time = frame_timer.ElapsedSeconds();
        // Reset and start the frame timer at the beginning of each frame. we do it here instead of the start to capture the time taken by event processing and looping as well.
        // Note: reset() clears accumulated time but does not start the timer.
        frame_timer.Reset();
        frame_timer.Start();

        frame_times_.push(frame_time);
        current_frame_sum_ += frame_time;
        if ((uint16_t)frame_times_.size() > config_.frame_time_sample_count) {
            current_frame_sum_ -= frame_times_.front();
            frame_times_.pop();
        }
    }
}

void SDLApplication::Shutdown() {
    ShutdownInternal(true);
}

void SDLApplication::ShutdownInternal(bool call_callbacks) {
    if (!initialized_) {
        return;
    }

    running_ = false;

    if (call_callbacks) {
        // Only invoke overridable hooks when the most-derived type is still alive.
        OnShutdown();
        if (config_.cleanupCallback) {
            config_.cleanupCallback();
        }
    }

    Cleanup();
}

SDL_Window* SDLApplication::GetWindow() const {
    return window_ ? static_cast<SDL_Window*>(*window_) : nullptr;
}

void SDLApplication::GetWindowSize(int* width, int* height) const {
    if (window_) {
        window_->GetSize(width, height);
    }
}

void SDLApplication::GetDrawableSize(int* width, int* height) const {
    if (window_) {
        window_->GetDrawableSize(width, height);
    }
}

void SDLApplication::SetWindowTitle(const char* title) {
    if (window_) {
        window_->SetTitle(title);
    }
}

void SDLApplication::UpdateConfig(const SDLApplicationConfig& new_config) {
    bool title_changed = new_config.windowTitle != config_.windowTitle;
    bool vsync_changed = new_config.enableVSync != config_.enableVSync;
    config_ = new_config;

    if (title_changed && window_) {
        window_->SetTitle(config_.windowTitle.c_str());
    }

    if (vsync_changed && swapchain_) {
        swapchain_->MarkForRecreation();
        if (renderer_) {
            renderer_->MarkSwapchainForRecreation();
        }
    }
}

bool SDLApplication::CreateVulkanObjects() {
    SDL_Window* window = GetWindow();
    if (!window) {
        throw std::runtime_error("Window must be created before Vulkan objects");
    }

    std::vector<Utils::NamedCapabilityRequest> instance_extension_requests;
    std::vector<const char*> sdl_extensions = Utils::SDLUtils::GetRequiredInstanceExtensions();
    instance_extension_requests.reserve(sdl_extensions.size() + config_.instanceExtensions.size() + 1);
    for (const char* extension : sdl_extensions) {
        if (extension) {
            instance_extension_requests.push_back({extension, Utils::CapabilityRequirement::REQUIRED});
        }
    }
    instance_extension_requests.insert(instance_extension_requests.end(),
                                       config_.instanceExtensions.begin(),
                                       config_.instanceExtensions.end());

    std::vector<Utils::NamedCapabilityRequest> validation_requests = GatherValidationLayers(config_.enableValidation);
    validation_requests.insert(validation_requests.end(),
                               config_.validationLayers.begin(),
                               config_.validationLayers.end());

    if (!validation_requests.empty()) {
        const bool require_debug_utils = HasRequiredRequest(validation_requests);
        instance_extension_requests.push_back({Constants::DEBUG_UTILS_EXTENSION,
                                               require_debug_utils ? Utils::CapabilityRequirement::REQUIRED
                                                                   : Utils::CapabilityRequirement::OPTIONAL});
    }

    std::vector<std::string> available_instance_extensions = Utils::EnumerateInstanceExtensionNames();
    Utils::NamedCapabilityResolution instance_resolution =
        Utils::ResolveNamedCapabilities(instance_extension_requests, available_instance_extensions);
    if (!instance_resolution.missingRequired.empty()) {
        std::ostringstream oss;
        oss << "Missing required instance extensions: " << JoinNames(instance_resolution.missingRequired);
        throw std::runtime_error(oss.str());
    }
    LogOptionalCapabilities(instance_resolution.missingOptional, "instance extensions");

    std::vector<std::string> available_layers = Utils::EnumerateInstanceLayerNames();
    Utils::NamedCapabilityResolution layer_resolution =
        Utils::ResolveNamedCapabilities(validation_requests, available_layers);
    if (!layer_resolution.missingRequired.empty()) {
        std::ostringstream oss;
        oss << "Missing required validation layers: " << JoinNames(layer_resolution.missingRequired);
        throw std::runtime_error(oss.str());
    }
    LogOptionalCapabilities(layer_resolution.missingOptional, "validation layers");

    std::vector<const char*> enabled_instance_extension_names = ToCStrVector(instance_resolution.enabled);
    std::vector<const char*> enabled_validation_layer_names = ToCStrVector(layer_resolution.enabled);

    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pApplicationName = config_.windowTitle.c_str();
    app_info.applicationVersion = DEFAULT_APPLICATION_VERSION;

    instance_ = std::make_unique<Instance>(app_info.pApplicationName,
                                            app_info.applicationVersion,
                                            enabled_instance_extension_names,
                                            enabled_validation_layer_names);

    surface_ = std::make_unique<Surface>(*instance_, window);

    physicalDevice_ = std::make_unique<PhysicalDevice>(*instance_, surface_->GetHandle());

    std::vector<Utils::NamedCapabilityRequest> device_extension_requests;
    device_extension_requests.reserve(1 + config_.deviceExtensions.size());
    device_extension_requests.push_back({Constants::SWAPCHAIN_EXTENSION, Utils::CapabilityRequirement::REQUIRED});
    device_extension_requests.insert(device_extension_requests.end(),
                                     config_.deviceExtensions.begin(),
                                     config_.deviceExtensions.end());
    std::vector<VkExtensionProperties> device_extension_properties = physicalDevice_->GetAvailableExtensions();
    std::vector<std::string> available_device_extensions;
    available_device_extensions.reserve(device_extension_properties.size());
    for (const VkExtensionProperties& prop : device_extension_properties) {
        available_device_extensions.emplace_back(prop.extensionName);
    }

    Utils::NamedCapabilityResolution device_extension_resolution =
        Utils::ResolveNamedCapabilities(device_extension_requests, available_device_extensions);
    if (!device_extension_resolution.missingRequired.empty()) {
        std::ostringstream oss;
        oss << "Missing required device extensions: " << JoinNames(device_extension_resolution.missingRequired);
        throw std::runtime_error(oss.str());
    }
    LogOptionalCapabilities(device_extension_resolution.missingOptional, "device extensions");

    VkPhysicalDeviceFeatures supported_features = physicalDevice_->GetFeatures();
    Utils::FeatureResolution feature_resolution = Utils::ResolveDeviceFeatures(
        supported_features,
        config_.requiredDeviceFeatures,
        config_.optionalDeviceFeatures);
    if (!feature_resolution.missingRequired.empty()) {
        std::ostringstream oss;
        oss << "Missing required device features: " << JoinNames(feature_resolution.missingRequired);
        throw std::runtime_error(oss.str());
    }
    LogOptionalCapabilities(feature_resolution.missingOptional, "device features");

    VkPhysicalDeviceFeatures enabled_features = feature_resolution.enabled;
    std::vector<const char*> enabled_device_extension_names = ToCStrVector(device_extension_resolution.enabled);

    device_ = std::make_unique<Device>(*physicalDevice_,
                                       enabled_device_extension_names,
                                       enabled_features,
                                       enabled_validation_layer_names);

    VkPresentModeKHR present_mode = ChoosePresentMode(config_.enableVSync);
    VkSurfaceFormatKHR preferred_format{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    swapchain_ = std::make_unique<Swapchain>(*device_, *surface_, window, present_mode, preferred_format);

    CreateRenderPass();

    renderer_ = std::make_unique<Renderer>(*device_, *swapchain_, *renderPass_, config_.maxFramesInFlight);

    return true;
}

void SDLApplication::CreateRenderPass() {
    if (!device_ || !swapchain_) {
        throw std::runtime_error("Renderer dependencies not initialised");
    }

    VkFormat color_format = swapchain_->GetImageFormat();
    renderPass_ = std::make_unique<RenderPass>(*device_, color_format);
}

void SDLApplication::SetRenderPass(std::unique_ptr<RenderPass> render_pass) {
    renderPass_ = std::move(render_pass);
}

void SDLApplication::HandleWindowResize() {
    if (swapchain_) {
        swapchain_->MarkForRecreation();
    }
}

void SDLApplication::ProcessEvents() {
    SDL_Event event{};
    Utils::Timer event_timer;
    event_timer.Reset();
    while (SDL_PollEvent(&event)) {
        event_timer.Stop();
        double event_processing_time_us = event_timer.ElapsedMicroseconds();
        if(event_processing_time_us > 500.0)
        {
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
    }
}

void SDLApplication::UpdateTiming() {
    uint64_t current_time = Utils::SDLUtils::GetPerformanceCounter();
    uint64_t frequency = Utils::SDLUtils::GetPerformanceFrequency();
    // Avoid repeating the same branch body: prefer a single assignment for the
    // default case and compute elapsed time only when we have a valid
    // previous timestamp and a non-zero frequency.
    if (lastTime_ != 0 && frequency > 0) {
        deltaTime_ = Utils::SDLUtils::GetElapsedTime(lastTime_, current_time);
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

void SDLApplication::Cleanup() {
    if (device_) {
        try
        {
            device_->WaitIdle();
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

    Utils::SDLUtils::QuitSdl();
    sdlContext_.reset();

    initialized_ = false;
}

} // namespace VulkanEngine::RAII

