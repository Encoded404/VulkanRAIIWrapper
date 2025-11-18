#ifndef VULKAN_RAII_EXAMPLES_SDL2_EXAMPLE_HPP
#define VULKAN_RAII_EXAMPLES_SDL2_EXAMPLE_HPP

// Example showing how to use the SDL2 + Vulkan RAII integration

#include "../SDL2Application.hpp"
#include <iostream>

namespace VulkanEngine {
namespace RAII {
namespace Examples {

// Simple example application using SDL2 + Vulkan
class SimpleSDL2Example : public SDL2Application {
public:
    SimpleSDL2Example() : SDL2Application(createConfig()) {}

private:
    static SDL2ApplicationConfig createConfig() {
        SDL2ApplicationConfig config;
        config.windowTitle = "SDL2 + Vulkan RAII Example";
        config.windowWidth = 1024;
        config.windowHeight = 768;
        config.enableValidation = true;
        config.enableVSync = true;
        
        return config;
    }

    bool onInitialize() override {
        std::cout << "Initializing SDL2 + Vulkan application...\n";
        
        // Your initialization code here
        // You can access Vulkan objects through getDevice(), getSwapchain(), etc.
        
        return true;
    }

    void onUpdate(double deltaTime) override {
        // Your update logic here
        // deltaTime is in seconds
        
        static double accumulatedTime = 0.0;
        accumulatedTime += deltaTime;
        
        // Example: print FPS every second
        if (accumulatedTime >= 1.0) {
            std::cout << "FPS: " << getFPS() << std::endl;
            accumulatedTime = 0.0;
        }
    }

    void onRender() override {
        // Your rendering code here
        // Use getRenderer() to access the renderer
        
        auto* renderer = getRenderer();
        if (renderer && renderer->beginFrame()) {
            // Record rendering commands here
            auto& commandBuffer = renderer->getCurrentCommandBuffer();
            
            // Example: begin render pass, draw, end render pass
            
            renderer->endFrame();
        }
    }

    void onEvent(const SDL_Event& event) override {
        // Handle SDL2 events here
        // Common events: SDL_QUIT, SDL_KEYDOWN, SDL_KEYUP, SDL_MOUSEBUTTONDOWN, etc.
        
        switch (event.type) {
            case 0x100: // SDL_QUIT
                std::cout << "Quit requested\n";
                requestExit();
                break;
                
            case 0x300: // SDL_KEYDOWN
                // Handle key press
                break;
                
            case 0x401: // SDL_MOUSEBUTTONDOWN
                // Handle mouse click
                break;
        }
    }

    void onResize(int width, int height) override {
        std::cout << "Window resized to " << width << "x" << height << std::endl;
        // Swapchain recreation is handled automatically
    }

    void onShutdown() override {
        std::cout << "Shutting down SDL2 + Vulkan application...\n";
        // Your cleanup code here
    }
};

// Function to run the example
inline int runSimpleExample() {
    try {
        SimpleSDL2Example app;
        
        if (!app.initialize()) {
            std::cerr << "Failed to initialize application\n";
            return -1;
        }
        
        app.run();
        app.shutdown();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
}

} // namespace Examples
} // namespace RAII
} // namespace VulkanEngine

/*
// Usage in main.cpp:

#include "VulkanRAII.hpp"

int main() {
    return VulkanEngine::RAII::Examples::runSimpleExample();
}

// Or create your own derived class:

class MyApp : public VulkanEngine::RAII::SDL2Application {
    // Override virtual methods as needed
};

int main() {
    MyApp app;
    if (app.initialize()) {
        app.run();
    }
    return 0;
}
*/

#endif // VULKAN_RAII_EXAMPLES_SDL2_EXAMPLE_HPP