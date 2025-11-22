#include "PipelineLayout.hpp"

#include "../core/Device.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>


namespace VulkanEngine::RAII {

PipelineLayout::PipelineLayout(const Device& device,
                               const std::vector<VkDescriptorSetLayout>& set_layouts,
                               const std::vector<VkPushConstantRange>& push_constant_ranges)
    : device_(device.GetHandle()),
      setLayouts_(set_layouts),
      pushConstantRanges_(push_constant_ranges) {
    if (device.GetHandle() == VK_NULL_HANDLE) {
        throw std::invalid_argument("PipelineLayout requires a valid device");
    }
    CreatePipelineLayout();
}

PipelineLayout::~PipelineLayout() {
    Cleanup();
}

PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
    : pipelineLayout_(other.pipelineLayout_),
      device_(other.device_),
      setLayouts_(std::move(other.setLayouts_)),
      pushConstantRanges_(std::move(other.pushConstantRanges_)) {
    other.pipelineLayout_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other) noexcept {
    if (this != &other) {
        Cleanup();
        pipelineLayout_ = other.pipelineLayout_;
        device_ = other.device_;
        setLayouts_ = std::move(other.setLayouts_);
        pushConstantRanges_ = std::move(other.pushConstantRanges_);

        other.pipelineLayout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

PipelineLayout PipelineLayout::CreateSingleSet(const Device& device, VkDescriptorSetLayout set_layout) {
    return PipelineLayout(device, {set_layout}, {});
}

PipelineLayout PipelineLayout::CreatePushConstantsOnly(const Device& device,
                                                        const std::vector<VkPushConstantRange>& push_constant_ranges) {
    return PipelineLayout(device, {}, push_constant_ranges);
}

PipelineLayout PipelineLayout::CreateEmpty(const Device& device) {
    return PipelineLayout(device, {}, {});
}

void PipelineLayout::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo layout_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layout_info.setLayoutCount = static_cast<uint32_t>(setLayouts_.size());
    layout_info.pSetLayouts = setLayouts_.empty() ? nullptr : setLayouts_.data();
    layout_info.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges_.size());
    layout_info.pPushConstantRanges = pushConstantRanges_.empty() ? nullptr : pushConstantRanges_.data();

    if (vkCreatePipelineLayout(device_, &layout_info, nullptr, &pipelineLayout_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }
}

void PipelineLayout::Cleanup() {
    if (pipelineLayout_ != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
        pipelineLayout_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

