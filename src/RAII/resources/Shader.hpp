#ifndef VULKAN_RAII_RESOURCES_SHADER_HPP
#define VULKAN_RAII_RESOURCES_SHADER_HPP

#include <volk.h>
#include <vector>
#include <string>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

class Shader {
public:
    // Constructor that creates a shader module from SPIR-V bytecode
    Shader(const Device& device, 
           const std::vector<uint32_t>& spirv_code);

    // Constructor that creates a shader module from file
    Shader(const Device& device, 
           const std::string& filename);

    // Destructor
    ~Shader();

    // Move constructor and assignment
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkShaderModule by only allowing moving.
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    [[nodiscard]] VkShaderModule get_handle() const { return shaderModule_; }
    
    // Implicit conversion to VkShaderModule
    operator VkShaderModule() const { return shaderModule_; }

    // Check if the shader is valid
    [[nodiscard]] bool is_valid() const { return shaderModule_ != VK_NULL_HANDLE; }

    // Get SPIR-V code
    [[nodiscard]] const std::vector<uint32_t>& get_spir_v_code() const { return spirvCode_; }

    // Get code size in bytes
    [[nodiscard]] size_t get_code_size() const { return spirvCode_.size() * sizeof(uint32_t); }

    // Create pipeline shader stage info
    VkPipelineShaderStageCreateInfo create_stage_info(VkShaderStageFlagBits stage,
                                                    const char* entry_point = "main",
                                                    const VkSpecializationInfo* specialization_info = nullptr) const;

    // Static utility functions for loading shaders
    static std::vector<uint32_t> load_spir_v_from_file(const std::string& filename);
    static std::vector<uint32_t> compile_glsl_to_spir_v(const std::string& source,
                                                    VkShaderStageFlagBits stage,
                                                    const std::string& filename = "shader");

    // Reflect shader information (requires SPIRV-Reflect or similar)
    struct ReflectionInfo {
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetBindings_;
        std::vector<VkPushConstantRange> pushConstantRanges_;
        std::vector<VkVertexInputAttributeDescription> inputAttributes_;
        std::vector<VkVertexInputBindingDescription> inputBindings_;
    };

    [[nodiscard]] ReflectionInfo reflect() const;

private:
    VkShaderModule shaderModule_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    std::vector<uint32_t> spirvCode_;

    // Helper methods
    void create_shader_module(const std::vector<uint32_t>& code);
    void cleanup();
    
    // Validation helpers
    static bool validate_spir_v(const std::vector<uint32_t>& spirv_code);
    static VkShaderStageFlagBits infer_stage_from_filename(const std::string& filename);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_SHADER_HPP