#include "Sampler.hpp"

#include "../core/Device.hpp"
#include <stdexcept>


namespace VulkanEngine::RAII {

Sampler::Sampler(const Device& device,
                 VkFilter mag_filter,
                 VkFilter min_filter,
                 VkSamplerMipmapMode mipmap_mode,
                 VkSamplerAddressMode address_mode_u,
                 VkSamplerAddressMode address_mode_v,
                 VkSamplerAddressMode address_mode_w,
                 float mip_lod_bias,
                 VkBool32 anisotropy_enable,
                 float max_anisotropy,
                 VkBool32 compare_enable,
                 VkCompareOp compare_op,
                 float min_lod,
                 float max_lod,
                 VkBorderColor border_color,
                 VkBool32 unnormalized_coordinates)
    : device_(device.get_handle()),
      magFilter_(mag_filter),
      minFilter_(min_filter),
      mipmapMode_(mipmap_mode),
      maxAnisotropy_(max_anisotropy),
      anisotropyEnable_(anisotropy_enable) {
    VkSamplerCreateInfo create_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    create_info.magFilter = mag_filter;
    create_info.minFilter = min_filter;
    create_info.mipmapMode = mipmap_mode;
    create_info.addressModeU = address_mode_u;
    create_info.addressModeV = address_mode_v;
    create_info.addressModeW = address_mode_w;
    create_info.mipLodBias = mip_lod_bias;
    create_info.anisotropyEnable = anisotropy_enable;
    create_info.maxAnisotropy = max_anisotropy;
    create_info.compareEnable = compare_enable;
    create_info.compareOp = compare_op;
    create_info.minLod = min_lod;
    create_info.maxLod = max_lod;
    create_info.borderColor = border_color;
    create_info.unnormalizedCoordinates = unnormalized_coordinates;

    create_sampler(create_info);
}

Sampler::Sampler(const Device& device, const VkSamplerCreateInfo& create_info)
    : device_(device.get_handle()),
      magFilter_(create_info.magFilter),
      minFilter_(create_info.minFilter),
      mipmapMode_(create_info.mipmapMode),
      maxAnisotropy_(create_info.maxAnisotropy),
      anisotropyEnable_(create_info.anisotropyEnable) {
    create_sampler(create_info);
}

Sampler::~Sampler() {
    cleanup();
}

Sampler::Sampler(Sampler&& other) noexcept
    : sampler_(other.sampler_),
      device_(other.device_),
      magFilter_(other.magFilter_),
      minFilter_(other.minFilter_),
      mipmapMode_(other.mipmapMode_),
      maxAnisotropy_(other.maxAnisotropy_),
      anisotropyEnable_(other.anisotropyEnable_) {
    other.sampler_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

Sampler& Sampler::operator=(Sampler&& other) noexcept {
    if (this != &other) {
        cleanup();
        sampler_ = other.sampler_;
        device_ = other.device_;
        magFilter_ = other.magFilter_;
        minFilter_ = other.minFilter_;
        mipmapMode_ = other.mipmapMode_;
        maxAnisotropy_ = other.maxAnisotropy_;
        anisotropyEnable_ = other.anisotropyEnable_;
        other.sampler_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

Sampler Sampler::create_linear(const Device& device, VkSamplerAddressMode address_mode) {
    return Sampler(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
                   address_mode, address_mode, address_mode);
}

Sampler Sampler::create_nearest(const Device& device, VkSamplerAddressMode address_mode) {
    return Sampler(device, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,
                   address_mode, address_mode, address_mode);
}

Sampler Sampler::create_anisotropic(const Device& device, float max_anisotropy) {
    return Sampler(device,
                   VK_FILTER_LINEAR,
                   VK_FILTER_LINEAR,
                   VK_SAMPLER_MIPMAP_MODE_LINEAR,
                   VK_SAMPLER_ADDRESS_MODE_REPEAT,
                   VK_SAMPLER_ADDRESS_MODE_REPEAT,
                   VK_SAMPLER_ADDRESS_MODE_REPEAT,
                   0.0f,
                   VK_TRUE,
                   max_anisotropy);
}

Sampler Sampler::create_shadow_map(const Device& device) {
    return Sampler(device,
                   VK_FILTER_LINEAR,
                   VK_FILTER_LINEAR,
                   VK_SAMPLER_MIPMAP_MODE_LINEAR,
                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                   0.0f,
                   VK_FALSE,
                   1.0f,
                   VK_TRUE,
                   VK_COMPARE_OP_LESS);
}

Sampler Sampler::create_clamp_to_edge(const Device& device, VkFilter filter) {
    return Sampler(device,
                   filter,
                   filter,
                   VK_SAMPLER_MIPMAP_MODE_LINEAR,
                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

Sampler Sampler::create_clamp_to_border(const Device& device, VkBorderColor border_color) {
    return Sampler(device,
                   VK_FILTER_LINEAR,
                   VK_FILTER_LINEAR,
                   VK_SAMPLER_MIPMAP_MODE_LINEAR,
                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                   0.0f,
                   VK_FALSE,
                   1.0f,
                   VK_FALSE,
                   VK_COMPARE_OP_ALWAYS,
                   0.0f,
                   VK_LOD_CLAMP_NONE,
                   border_color);
}

void Sampler::create_sampler(const VkSamplerCreateInfo& create_info) {
    if (vkCreateSampler(device_, &create_info, nullptr, &sampler_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create sampler");
    }
}

void Sampler::cleanup() {
    if (sampler_ != VK_NULL_HANDLE) {
        vkDestroySampler(device_, sampler_, nullptr);
        sampler_ = VK_NULL_HANDLE;
    }
}

} // namespace VulkanEngine::RAII

