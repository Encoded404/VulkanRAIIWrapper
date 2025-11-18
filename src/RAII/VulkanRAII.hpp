#ifndef VULKAN_RAII_HPP
#define VULKAN_RAII_HPP

// Main RAII header that includes all Vulkan RAII wrappers
// This provides a convenient single include for the entire RAII library

// Core Vulkan objects
#include "core/instance.hpp"
#include "core/PhysicalDevice.hpp"
#include "core/Device.hpp"
#include "core/Queue.hpp"
#include "core/DebugMessenger.hpp"

// Presentation
#include "presentation/Surface.hpp"
#include "presentation/Swapchain.hpp"

// Rendering
#include "rendering/CommandPool.hpp"
#include "rendering/CommandBuffer.hpp"
#include "rendering/RenderPass.hpp"
#include "rendering/Framebuffer.hpp"
#include "rendering/PipelineStructs.hpp"
#include "rendering/Pipeline.hpp"
#include "rendering/Renderer.hpp"

// Resources
#include "resources/Buffer.hpp"
#include "resources/Image.hpp"
#include "resources/Shader.hpp"
#include "resources/Sampler.hpp"
#include "resources/VmaAllocator.hpp"
#include "resources/DescriptorSetLayout.hpp"
#include "resources/DescriptorPool.hpp"
#include "resources/PipelineLayout.hpp"

// Synchronization
#include "sync/Semaphore.hpp"
#include "sync/Fence.hpp"
#include "sync/Event.hpp"

// Types
#include "types/QueueFamilyIndices.hpp"

// Utilities
#include "utils/VulkanUtils.hpp"

// SDL2 Integration
#include "SDL2Application.hpp"

namespace VulkanEngine {
namespace RAII {

// Convenience namespace alias
namespace VE = VulkanEngine;
namespace VER = VulkanEngine::RAII;

} // namespace RAII
} // namespace VulkanEngine

#endif // VULKAN_RAII_HPP