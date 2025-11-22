#include "DescriptorPool.hpp"

#include "../core/Device.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>


namespace VulkanEngine::RAII {

DescriptorPool::DescriptorPool(const Device& device,
                               uint32_t max_sets,
                               const std::vector<VkDescriptorPoolSize>& pool_sizes,
                               VkDescriptorPoolCreateFlags flags)
    : device_(device.GetHandle()),
      maxSets_(max_sets),
      poolSizes_(pool_sizes),
      allowsIndividualFree_((flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) != 0) {
    if (device == VK_NULL_HANDLE) {
        throw std::invalid_argument("DescriptorPool requires a valid device");
    }
    if (maxSets_ == 0) {
        throw std::invalid_argument("DescriptorPool maxSets must be greater than zero");
    }
    CreateDescriptorPool(flags);
}

DescriptorPool::DescriptorPool(const Device& device,
                               uint32_t max_sets,
                               VkDescriptorType descriptor_type,
                               uint32_t descriptor_count,
                               VkDescriptorPoolCreateFlags flags)
    : DescriptorPool(device,
                     max_sets,
                     std::vector<VkDescriptorPoolSize>{{descriptor_type, descriptor_count * max_sets}},
                     flags) {}

DescriptorPool::~DescriptorPool() {
    Cleanup();
}

DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
    : descriptorPool_(other.descriptorPool_),
      device_(other.device_),
      maxSets_(other.maxSets_),
      poolSizes_(std::move(other.poolSizes_)),
      allowsIndividualFree_(other.allowsIndividualFree_) {
    other.descriptorPool_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.maxSets_ = 0;
    other.allowsIndividualFree_ = false;
}

DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept {
    if (this != &other) {
        Cleanup();
        descriptorPool_ = other.descriptorPool_;
        device_ = other.device_;
        maxSets_ = other.maxSets_;
        poolSizes_ = std::move(other.poolSizes_);
        allowsIndividualFree_ = other.allowsIndividualFree_;

        other.descriptorPool_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.maxSets_ = 0;
        other.allowsIndividualFree_ = false;
    }
    return *this;
}

std::vector<VkDescriptorSet> DescriptorPool::AllocateDescriptorSets(const std::vector<VkDescriptorSetLayout>& layouts) {
    if (layouts.empty()) {
        return {};
    }

    std::vector<VkDescriptorSet> descriptor_sets(layouts.size());
    VkDescriptorSetAllocateInfo alloc_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = descriptorPool_;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    alloc_info.pSetLayouts = layouts.data();

    VkResult result = vkAllocateDescriptorSets(device_, &alloc_info, descriptor_sets.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets");
    }
    return descriptor_sets;
}

VkDescriptorSet DescriptorPool::AllocateDescriptorSet(VkDescriptorSetLayout layout) {
    auto sets = AllocateDescriptorSets({layout});
    return sets.front();
}

void DescriptorPool::FreeDescriptorSets(const std::vector<VkDescriptorSet>& descriptor_sets) {
    if (!allowsIndividualFree_ || descriptor_sets.empty()) {
        return;
    }
    vkFreeDescriptorSets(device_,
                         descriptorPool_,
                         static_cast<uint32_t>(descriptor_sets.size()),
                         descriptor_sets.data());
}

void DescriptorPool::FreeDescriptorSet(VkDescriptorSet descriptor_set) {
    if (descriptor_set == VK_NULL_HANDLE) {
        return;
    }
    FreeDescriptorSets({descriptor_set});
}

void DescriptorPool::Reset(VkDescriptorPoolResetFlags flags) {
    vkResetDescriptorPool(device_, descriptorPool_, flags);
}

void DescriptorPool::UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writes,
                                            const std::vector<VkCopyDescriptorSet>& copies) const {
    if (writes.empty() && copies.empty()) {
        return;
    }
    vkUpdateDescriptorSets(device_,
                           static_cast<uint32_t>(writes.size()),
                           writes.empty() ? nullptr : writes.data(),
                           static_cast<uint32_t>(copies.size()),
                           copies.empty() ? nullptr : copies.data());
}

DescriptorPool DescriptorPool::CreateForUniformBuffers(const Device& device,
                                                        uint32_t max_sets,
                                                        uint32_t uniform_buffer_count) {
    VkDescriptorPoolSize pool_size{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                  std::max(1u, uniform_buffer_count) * max_sets};
    return DescriptorPool(device, max_sets, {pool_size});
}

DescriptorPool DescriptorPool::CreateForTextures(const Device& device,
                                                 uint32_t max_sets,
                                                 uint32_t texture_count) {
    VkDescriptorPoolSize pool_size{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                  std::max(1u, texture_count) * max_sets};
    return DescriptorPool(device, max_sets, {pool_size});
}

DescriptorPool DescriptorPool::CreateForMixed(const Device& device,
                                              uint32_t max_sets,
                                              uint32_t uniform_buffer_count,
                                              uint32_t sampler_count,
                                              uint32_t storage_buffer_count) {
    std::vector<VkDescriptorPoolSize> pool_sizes;
    if (uniform_buffer_count > 0) {
        pool_sizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform_buffer_count * max_sets});
    }
    if (sampler_count > 0) {
        pool_sizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sampler_count * max_sets});
    }
    if (storage_buffer_count > 0) {
        pool_sizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, storage_buffer_count * max_sets});
    }
    if (pool_sizes.empty()) {
        pool_sizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, max_sets});
    }
    return DescriptorPool(device, max_sets, pool_sizes);
}

void DescriptorPool::CreateDescriptorPool(VkDescriptorPoolCreateFlags flags) {
    VkDescriptorPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags = flags;
    pool_info.maxSets = maxSets_;
    pool_info.poolSizeCount = static_cast<uint32_t>(poolSizes_.size());
    pool_info.pPoolSizes = poolSizes_.data();

    if (vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptorPool_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

void DescriptorPool::Cleanup() {
    if (descriptorPool_ != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
        descriptorPool_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

