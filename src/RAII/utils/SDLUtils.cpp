#include "SDLUtils.hpp"

#include "Constants.hpp"

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_version.h>

#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

namespace VulkanEngine::RAII::Utils {

namespace {

// SDL3 surfaces displays via opaque IDs; translate legacy index lookups so
// existing code can keep addressing monitors by index.
SDL_DisplayID GetDisplayIdFromIndex(int display_index) {
    int count = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&count);
    if (!displays || display_index < 0 || display_index >= count) {
        if (displays) {
            SDL_free(displays);
        }
        return 0;
    }

    SDL_DisplayID id = displays[display_index];
    SDL_free(displays);
    return id;
}

} // namespace

bool SDLUtils::s_initialized = false;
bool SDLUtils::s_vulkan_supported = false;

bool SDLUtils::InitializeSdlForVulkan() {
    if (s_initialized) {
        return s_vulkan_supported;
    }

    if (!SDL_Init(static_cast<SDL_InitFlags>(SDL_INIT_VIDEO))) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n' << std::flush;
        return false;
    }

    if (!SDL_Vulkan_LoadLibrary(nullptr)) {
        std::cerr << "Failed to load Vulkan: " << SDL_GetError() << '\n' << std::flush;
        SDL_Quit();
        return false;
    }

    s_initialized = true;
    s_vulkan_supported = true;
    return true;
}

void SDLUtils::QuitSdl() {
    if (!s_initialized) {
        return;
    }

    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
    s_initialized = false;
    s_vulkan_supported = false;
}

std::vector<const char*> SDLUtils::GetRequiredInstanceExtensions() {
    if (!IsVulkanSupported()) {
        throw std::runtime_error("SDL Vulkan support has not been initialised");
    }

    Uint32 count = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);
    if (!extensions || count == 0) {
        throw std::runtime_error("Failed to query SDL Vulkan instance extensions");
    }

    return std::vector<const char*>(extensions, extensions + count);
}

bool SDLUtils::IsVulkanSupported() {
    return s_vulkan_supported || InitializeSdlForVulkan();
}

void* SDLUtils::GetVulkanLoader() {
    if (!IsVulkanSupported()) {
        return nullptr;
    }
    return reinterpret_cast<void*>(SDL_Vulkan_GetVkGetInstanceProcAddr());
}

SDL_Window* SDLUtils::CreateVulkanWindow(const char* title,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            uint32_t flags) {
    if (!InitializeSdlForVulkan()) {
        return nullptr;
    }

    SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(flags | SDL_WINDOW_VULKAN);
    SDL_Window* window = SDL_CreateWindow(title ? title : "", width, height, window_flags);
    if (!window) {
        return nullptr;
    }

    const int posX = (x < 0) ? static_cast<int>(SDL_WINDOWPOS_CENTERED) : x;
    const int posY = (y < 0) ? static_cast<int>(SDL_WINDOWPOS_CENTERED) : y;
    SDL_SetWindowPosition(window, posX, posY);
    return window;
}

void SDLUtils::GetWindowSize(SDL_Window* window, int* width, int* height) {
    if (!window) {
        return;
    }
    SDL_GetWindowSize(window, width, height);
}

void SDLUtils::GetDrawableSize(SDL_Window* window, int* width, int* height) {
    if (!window) {
        return;
    }
    SDL_GetWindowSizeInPixels(window, width, height);
}

bool SDLUtils::HandleWindowEvent(const void* event) {
    if (!event) {
        return false;
    }

    const SDL_Event* sdl_event = static_cast<const SDL_Event*>(event);
    switch (sdl_event->type) {
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            return true;
        default:
            return false;
    }
}

bool SDLUtils::WasWindowResized(const void* event) {
    const SDL_Event* sdl_event = static_cast<const SDL_Event*>(event);
    return sdl_event &&
           (sdl_event->type == SDL_EVENT_WINDOW_RESIZED ||
            sdl_event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);
}

bool SDLUtils::WasWindowMinimized(const void* event) {
    const SDL_Event* sdl_event = static_cast<const SDL_Event*>(event);
    return sdl_event && sdl_event->type == SDL_EVENT_WINDOW_MINIMIZED;
}

bool SDLUtils::WasWindowRestored(const void* event) {
    const SDL_Event* sdl_event = static_cast<const SDL_Event*>(event);
    return sdl_event &&
           (sdl_event->type == SDL_EVENT_WINDOW_RESTORED || sdl_event->type == SDL_EVENT_WINDOW_MAXIMIZED);
}

SDL_Window* SDLUtils::GetWindowFromSurface(VkSurfaceKHR /*surface*/) {
    return nullptr;
}

bool SDLUtils::IsWindowMinimized(SDL_Window* window) {
    return window && (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) != 0;
}

bool SDLUtils::IsWindowMaximized(SDL_Window* window) {
    return window && (SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) != 0;
}

bool SDLUtils::IsWindowFullscreen(SDL_Window* window) {
    return window && (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
}

bool SDLUtils::GetKeyEvent(const void* sdl_event, KeyEvent& key_event) {
    const SDL_Event* event = static_cast<const SDL_Event*>(sdl_event);
    if (!event || (event->type != SDL_EVENT_KEY_DOWN && event->type != SDL_EVENT_KEY_UP)) {
        return false;
    }

    key_event.scancode = event->key.scancode;
    key_event.keycode = static_cast<int>(event->key.key);
    key_event.pressed = event->key.down;
    key_event.repeat = event->key.repeat;
    return true;
}

bool SDLUtils::GetMouseEvent(const void* sdl_event, MouseEvent& mouse_event) {
    const SDL_Event* event = static_cast<const SDL_Event*>(sdl_event);
    if (!event || (event->type != SDL_EVENT_MOUSE_BUTTON_DOWN && event->type != SDL_EVENT_MOUSE_BUTTON_UP)) {
        return false;
    }

    mouse_event.x = static_cast<int>(event->button.x);
    mouse_event.y = static_cast<int>(event->button.y);
    mouse_event.button = event->button.button;
    mouse_event.pressed = event->button.down;
    mouse_event.clicks = event->button.clicks;
    return true;
}

float SDLUtils::GetDisplayScale(SDL_Window* window) {
    if (!window) {
        return 1.0f;
    }

    float scale = SDL_GetWindowDisplayScale(window);
    if (scale <= 0.0f) {
        scale = 1.0f;
    }
    return scale;
}

void SDLUtils::GetDisplayDpi(SDL_Window* window, float* ddpi, float* hdpi, float* vdpi) {
    const float scale = GetDisplayScale(window);
    const float dpi = Constants::SDL::DEFAULT_DPI * scale;
    if (ddpi) *ddpi = dpi;
    if (hdpi) *hdpi = dpi;
    if (vdpi) *vdpi = dpi;
}

int SDLUtils::GetDisplayCount() {
    int count = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&count);
    if (displays) {
        SDL_free(displays);
    }
    return count;
}

void SDLUtils::GetDisplayBounds(int display_index, int* x, int* y, int* w, int* h) {
    SDL_DisplayID display_id = GetDisplayIdFromIndex(display_index);
    if (display_id == 0) {
        return;
    }

    SDL_Rect rect{};
    if (SDL_GetDisplayBounds(display_id, &rect)) {
        if (x) *x = rect.x;
        if (y) *y = rect.y;
        if (w) *w = rect.w;
        if (h) *h = rect.h;
    }
}

void SDLUtils::GetDisplayUsableBounds(int display_index, int* x, int* y, int* w, int* h) {
    SDL_DisplayID display_id = GetDisplayIdFromIndex(display_index);
    if (display_id == 0) {
        return;
    }

    SDL_Rect rect{};
    if (SDL_GetDisplayUsableBounds(display_id, &rect)) {
        if (x) *x = rect.x;
        if (y) *y = rect.y;
        if (w) *w = rect.w;
        if (h) *h = rect.h;
    }
}

uint64_t SDLUtils::GetPerformanceCounter() {
    return SDL_GetPerformanceCounter();
}

uint64_t SDLUtils::GetPerformanceFrequency() {
    return SDL_GetPerformanceFrequency();
}

double SDLUtils::GetElapsedTime(uint64_t start, uint64_t end) {
    uint64_t freq = GetPerformanceFrequency();
    if (freq == 0) {
        return 0.0;
    }
    return static_cast<double>(end - start) / static_cast<double>(freq);
}

bool SDLUtils::HasClipboardText() {
    return SDL_HasClipboardText();
}

std::string SDLUtils::GetClipboardText() {
    char* text = SDL_GetClipboardText();
    std::string result = text ? text : "";
    SDL_free(text);
    return result;
}

void SDLUtils::SetClipboardText(const std::string& text) {
    SDL_SetClipboardText(text.c_str());
}

std::string SDLUtils::GetSdlError() {
    const char* error = SDL_GetError();
    return error ? error : "";
}

void SDLUtils::ClearSdlError() {
    SDL_ClearError();
}

std::string SDLUtils::GetSdlVersionString(bool /*linked*/) {
    const int version = SDL_GetVersion(); // linked sdl version
    return std::to_string(SDL_VERSIONNUM_MAJOR(version)) + "." +
           std::to_string(SDL_VERSIONNUM_MINOR(version)) + "." +
           std::to_string(SDL_VERSIONNUM_MICRO(version) );
}

SDLContext::SDLContext(uint32_t flags) {
    SDL_InitFlags init_flags = flags == 0 ? SDL_INIT_VIDEO : static_cast<SDL_InitFlags>(flags);
    initialized_ = SDL_Init(init_flags);
    if (!initialized_) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n' << std::flush;
    }
}

SDLContext::~SDLContext() {
    if (initialized_) {
        SDL_Quit();
        initialized_ = false;
    }
}

SDLContext::SDLContext(SDLContext&& other) noexcept
    : initialized_(other.initialized_) {
    other.initialized_ = false;
}

SDLContext& SDLContext::operator=(SDLContext&& other) noexcept {
    if (this != &other) {
        if (initialized_) {
            SDL_Quit();
        }
        initialized_ = other.initialized_;
        other.initialized_ = false;
    }
    return *this;
}

SDLWindow::SDLWindow(const char* title,
                     int x,
                     int y,
                     int width,
                     int height,
                     uint32_t flags) {
    SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(flags);
    window_ = SDL_CreateWindow(title ? title : "", width, height, window_flags);
    if (!window_) {
        return;
    }

    const int posX = (x < 0) ? static_cast<int>(SDL_WINDOWPOS_CENTERED) : x;
    const int posY = (y < 0) ? static_cast<int>(SDL_WINDOWPOS_CENTERED) : y;
    SDL_SetWindowPosition(window_, posX, posY);
}

SDLWindow::~SDLWindow() {
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

SDLWindow::SDLWindow(SDLWindow&& other) noexcept
    : window_(other.window_) {
    other.window_ = nullptr;
}

SDLWindow& SDLWindow::operator=(SDLWindow&& other) noexcept {
    if (this != &other) {
        if (window_) {
            SDL_DestroyWindow(window_);
        }
        window_ = other.window_;
        other.window_ = nullptr;
    }
    return *this;
}

void SDLWindow::GetSize(int* width, int* height) const {
    if (!window_) {
        return;
    }
    SDL_GetWindowSize(window_, width, height);
}

void SDLWindow::GetDrawableSize(int* width, int* height) const {
    if (!window_) {
        return;
    }
    SDL_GetWindowSizeInPixels(window_, width, height);
}

void SDLWindow::SetTitle(const char* title) {
    if (!window_) {
        return;
    }
    SDL_SetWindowTitle(window_, title ? title : "");
}

} // namespace VulkanEngine::RAII::Utils
