# Main RAII library
find_package(SDL3 CONFIG REQUIRED COMPONENTS SDL3)

add_library(VulkanRAIIWrapper STATIC
    # core
    core/DebugMessenger.cpp
    core/Instance.cpp
    core/PhysicalDevice.cpp
    core/Queue.cpp
    core/Device.cpp

    # application
    SDL2Application.cpp

    # presentation
    presentation/Surface.cpp
    presentation/Swapchain.cpp

    # rendering
    rendering/CommandPool.cpp
    rendering/CommandBuffer.cpp
    rendering/Framebuffer.cpp
    rendering/RenderPass.cpp
    rendering/Pipeline.cpp
    rendering/PipelineStructs.cpp
    rendering/Renderer.cpp

    # resources
    resources/Buffer.cpp
    resources/Image.cpp
    resources/Sampler.cpp
    resources/VmaAllocator.cpp
    resources/DescriptorPool.cpp
    resources/DescriptorSetLayout.cpp
    resources/PipelineLayout.cpp
    resources/Shader.cpp

    # synchronization
    sync/Semaphore.cpp
    sync/Fence.cpp
    sync/Event.cpp

    # types
    types/QueueFamilyIndices.cpp

    # utilities
    utils/Constants.cpp
    utils/DebugUtils.cpp
    utils/FormatUtils.cpp
    utils/ImageUtils.cpp
    utils/MemoryUtils.cpp
    utils/PipelineUtils.cpp
    utils/SDLUtils.cpp
    utils/VulkanUtils.cpp
    utils/Timer.cpp

    # library glue
    Library_impl.cpp
)

# Backwards compatibility alias until downstreams migrate
add_library(VulkanEngineLib ALIAS VulkanRAIIWrapper)

# Set C++ standard using target_compile_features
target_compile_features(VulkanRAIIWrapper PUBLIC cxx_std_20)

# Compiler warnings and options
target_compile_options(VulkanRAIIWrapper PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>
)

# Link Vulkan, Volk, and VMA via vcpkg
target_link_libraries(VulkanRAIIWrapper PUBLIC 
    Vulkan::Vulkan
    volk::volk
    GPUOpen::VulkanMemoryAllocator
    SDL3::SDL3
)

# Include directories
target_include_directories(VulkanRAIIWrapper PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
)