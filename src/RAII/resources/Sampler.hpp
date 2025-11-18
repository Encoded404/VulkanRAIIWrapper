#ifndef VULKAN_RAII_RESOURCES_SAMPLER_HPP
#define VULKAN_RAII_RESOURCES_SAMPLER_HPP

#include <volk.h>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

class Sampler {
public:
    // Constructor that creates a sampler with detailed settings
    Sampler(const Device& device,
            VkFilter mag_filter = VK_FILTER_LINEAR,
            VkFilter min_filter = VK_FILTER_LINEAR,
            VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            VkSamplerAddressMode address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VkSamplerAddressMode address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VkSamplerAddressMode address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            float mip_lod_bias = 0.0f,
            VkBool32 anisotropy_enable = VK_FALSE,
            float max_anisotropy = 1.0f,
            VkBool32 compare_enable = VK_FALSE,
            VkCompareOp compare_op = VK_COMPARE_OP_ALWAYS,
            float min_lod = 0.0f,
            float max_lod = VK_LOD_CLAMP_NONE,
            VkBorderColor border_color = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            VkBool32 unnormalized_coordinates = VK_FALSE);

    // Constructor with sampler create info
    Sampler(const Device& device, const VkSamplerCreateInfo& create_info);

    // Destructor
    ~Sampler();

    // Move constructor and assignment
    Sampler(Sampler&& other) noexcept;
    Sampler& operator=(Sampler&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkSampler by only allowing moving.
    Sampler(const Sampler&) = delete;
    Sampler& operator=(const Sampler&) = delete;

    [[nodiscard]] VkSampler get_handle() const { return sampler_; }
    
    // Implicit conversion to VkSampler
    operator VkSampler() const { return sampler_; }

    // Check if the sampler is valid
    [[nodiscard]] bool is_valid() const { return sampler_ != VK_NULL_HANDLE; }

    // Get sampler properties
    [[nodiscard]] VkFilter get_mag_filter() const { return magFilter_; }
    [[nodiscard]] VkFilter get_min_filter() const { return minFilter_; }
    [[nodiscard]] VkSamplerMipmapMode get_mipmap_mode() const { return mipmapMode_; }
    [[nodiscard]] float get_max_anisotropy() const { return maxAnisotropy_; }
    [[nodiscard]] bool is_anisotropy_enabled() const { return anisotropyEnable_; }

    // Helper constructors for common sampler types
    static Sampler create_linear(const Device& device, VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    static Sampler create_nearest(const Device& device, VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    static Sampler create_anisotropic(const Device& device, float max_anisotropy = 16.0f);
    static Sampler create_shadow_map(const Device& device);
    static Sampler create_clamp_to_edge(const Device& device, VkFilter filter = VK_FILTER_LINEAR);
    static Sampler create_clamp_to_border(const Device& device, VkBorderColor border_color = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

private:
    VkSampler sampler_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device

    // Cached properties for query functions
    VkFilter magFilter_{VK_FILTER_LINEAR};
    VkFilter minFilter_{VK_FILTER_LINEAR};
    VkSamplerMipmapMode mipmapMode_{VK_SAMPLER_MIPMAP_MODE_LINEAR};
    float maxAnisotropy_{1.0f};
    VkBool32 anisotropyEnable_{VK_FALSE};

    // Helper methods
    void create_sampler(const VkSamplerCreateInfo& create_info);
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_SAMPLER_HPP