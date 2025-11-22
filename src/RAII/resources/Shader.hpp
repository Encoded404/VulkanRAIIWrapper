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

    [[nodiscard]] VkShaderModule GetHandle() const { return shaderModule_; }
    
    // Implicit conversion to VkShaderModule
    operator VkShaderModule() const { return shaderModule_; }

    // Check if the shader is valid
    [[nodiscard]] bool IsValid() const { return shaderModule_ != VK_NULL_HANDLE; }

    // Get SPIR-V code
    [[nodiscard]] const std::vector<uint32_t>& GetSpirVCode() const { return spirvCode_; }

    // Get code size in bytes
    [[nodiscard]] size_t GetCodeSize() const { return spirvCode_.size() * sizeof(uint32_t); }

    // Create pipeline shader stage info
    VkPipelineShaderStageCreateInfo CreateStageInfo(VkShaderStageFlagBits stage,
                                                    const char* entry_point = "main",
                                                    const VkSpecializationInfo* specialization_info = nullptr) const;

    // Static utility functions for loading shaders
    static std::vector<uint32_t> LoadSpirVFromFile(const std::string& filename);
    static std::vector<uint32_t> CompileGlslToSpirV(const std::string& source,
                                                    VkShaderStageFlagBits stage,
                                                    const std::string& filename = "shader");

    // Reflect shader information (requires SPIRV-Reflect or similar)
    struct ReflectionInfo {
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetBindings;
        std::vector<VkPushConstantRange> pushConstantRanges;
        std::vector<VkVertexInputAttributeDescription> inputAttributes;
        std::vector<VkVertexInputBindingDescription> inputBindings;
    };

    [[nodiscard]] ReflectionInfo Reflect() const;

private:
    VkShaderModule shaderModule_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    std::vector<uint32_t> spirvCode_;

    // Helper methods
    void CreateShaderModule(const std::vector<uint32_t>& code);
    void Cleanup();
    
    // Validation helpers
    static bool ValidateSpirV(const std::vector<uint32_t>& spirv_code);
    static VkShaderStageFlagBits InferStageFromFilename(const std::string& filename);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_SHADER_HPP