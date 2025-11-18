#include "DescriptorSetLayout.hpp"

#include "../core/Device.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>


namespace VulkanEngine::RAII {

DescriptorSetLayout::DescriptorSetLayout(const Device& device,
                                         const std::vector<DescriptorSetLayoutBinding>& bindings,
                                         VkDescriptorSetLayoutCreateFlags flags)
    : device_(device.get_handle()),
      bindings_(convert_bindings(bindings)) {
    if (device == VK_NULL_HANDLE) {
        throw std::invalid_argument("DescriptorSetLayout requires a valid device");
    }
    create_descriptor_set_layout(flags);
}

DescriptorSetLayout::DescriptorSetLayout(const Device& device,
                                         const std::vector<VkDescriptorSetLayoutBinding>& bindings,
                                         VkDescriptorSetLayoutCreateFlags flags)
    : device_(device.get_handle()),
      bindings_(bindings) {
    if (device == VK_NULL_HANDLE) {
        throw std::invalid_argument("DescriptorSetLayout requires a valid device");
    }
    create_descriptor_set_layout(flags);
}

DescriptorSetLayout::~DescriptorSetLayout() {
    cleanup();
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
    : descriptorSetLayout_(other.descriptorSetLayout_),
      device_(other.device_),
      bindings_(std::move(other.bindings_)) {
    other.descriptorSetLayout_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept {
    if (this != &other) {
        cleanup();
        descriptorSetLayout_ = other.descriptorSetLayout_;
        device_ = other.device_;
        bindings_ = std::move(other.bindings_);

        other.descriptorSetLayout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

bool DescriptorSetLayout::has_binding(uint32_t binding) const {
    return get_binding(binding) != nullptr;
}

const VkDescriptorSetLayoutBinding* DescriptorSetLayout::get_binding(uint32_t binding) const {
    for (const auto& layout_binding : bindings_) {
        if (layout_binding.binding == binding) {
            return &layout_binding;
        }
    }
    return nullptr;
}

VkDescriptorType DescriptorSetLayout::get_descriptor_type(uint32_t binding) const {
    auto binding_ptr = get_binding(binding);
    if (!binding_ptr) {
        throw std::out_of_range("Descriptor set layout binding not found");
    }
    return binding_ptr->descriptorType;
}

uint32_t DescriptorSetLayout::get_descriptor_count(uint32_t binding) const {
    auto binding_ptr = get_binding(binding);
    if (!binding_ptr) {
        throw std::out_of_range("Descriptor set layout binding not found");
    }
    return binding_ptr->descriptorCount;
}

DescriptorSetLayout DescriptorSetLayout::create_single_binding(const Device& device,
                                                             uint32_t binding,
                                                             VkDescriptorType descriptor_type,
                                                             VkShaderStageFlags stage_flags,
                                                             uint32_t descriptor_count) {
    DescriptorSetLayoutBinding layout_binding{binding, descriptor_type, descriptor_count, stage_flags, nullptr};
    return DescriptorSetLayout(device, std::vector<DescriptorSetLayoutBinding>{layout_binding});
}

void DescriptorSetLayout::create_descriptor_set_layout(VkDescriptorSetLayoutCreateFlags flags) {
    VkDescriptorSetLayoutCreateInfo layout_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layout_info.bindingCount = static_cast<uint32_t>(bindings_.size());
    layout_info.pBindings = bindings_.empty() ? nullptr : bindings_.data();
    layout_info.flags = flags;

    if (vkCreateDescriptorSetLayout(device_, &layout_info, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }
}

void DescriptorSetLayout::cleanup() {
    if (descriptorSetLayout_ != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device_, descriptorSetLayout_, nullptr);
        descriptorSetLayout_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayout::convert_bindings(const std::vector<DescriptorSetLayoutBinding>& bindings) {
    std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
    vk_bindings.reserve(bindings.size());
    for (const auto& binding : bindings) {
        VkDescriptorSetLayoutBinding vk_binding{};
        vk_binding.binding = binding.binding_;
        vk_binding.descriptorType = binding.descriptorType_;
        vk_binding.descriptorCount = binding.descriptorCount_;
        vk_binding.stageFlags = binding.stageFlags_;
        vk_binding.pImmutableSamplers = binding.immutableSamplers_;
        vk_bindings.push_back(vk_binding);
    }
    return vk_bindings;
}

} // namespace VulkanEngine::RAII

