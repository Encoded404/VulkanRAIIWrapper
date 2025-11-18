#include "Shader.hpp"

#include "../core/Device.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>


namespace VulkanEngine::RAII {

Shader::Shader(const Device& device, const std::vector<uint32_t>& spirv_code)
    : device_(device.get_handle()),
      spirvCode_(spirv_code) {
    if (device == VK_NULL_HANDLE) {
        throw std::invalid_argument("Shader requires a valid device");
    }
    if (!validate_spir_v(spirvCode_)) {
        throw std::invalid_argument("Invalid SPIR-V code");
    }
    create_shader_module(spirvCode_);
}

Shader::Shader(const Device& device, const std::string& filename)
    : device_(device.get_handle()),
      spirvCode_(load_spir_v_from_file(filename)) {
    if (device == VK_NULL_HANDLE) {
        throw std::invalid_argument("Shader requires a valid device");
    }
    if (!validate_spir_v(spirvCode_)) {
        throw std::invalid_argument("Invalid SPIR-V file");
    }
    create_shader_module(spirvCode_);
}

Shader::~Shader() {
    cleanup();
}

Shader::Shader(Shader&& other) noexcept
    : shaderModule_(other.shaderModule_),
      device_(other.device_),
      spirvCode_(std::move(other.spirvCode_)) {
    other.shaderModule_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        cleanup();
        shaderModule_ = other.shaderModule_;
        device_ = other.device_;
        spirvCode_ = std::move(other.spirvCode_);

        other.shaderModule_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

VkPipelineShaderStageCreateInfo Shader::create_stage_info(VkShaderStageFlagBits stage,
                                                        const char* entry_point,
                                                        const VkSpecializationInfo* specialization_info) const {
    VkPipelineShaderStageCreateInfo stage_info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stage_info.stage = stage;
    stage_info.module = shaderModule_;
    stage_info.pName = entry_point ? entry_point : "main";
    stage_info.pSpecializationInfo = specialization_info;
    return stage_info;
}

std::vector<uint32_t> Shader::load_spir_v_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open SPIR-V file: " + filename);
    }

    std::streamsize file_size = file.tellg();
    if (file_size <= 0 || file_size % sizeof(uint32_t) != 0) {
        throw std::runtime_error("Invalid SPIR-V file size: " + filename);
    }

    file.seekg(0, std::ios::beg);
    std::vector<uint32_t> buffer(static_cast<size_t>(file_size / sizeof(uint32_t)));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
        throw std::runtime_error("Failed to read SPIR-V file: " + filename);
    }
    return buffer;
}

std::vector<uint32_t> Shader::compile_glsl_to_spir_v(const std::string& /*source*/,
                                                 VkShaderStageFlagBits /*stage*/,
                                                 const std::string& filename) {
    throw std::runtime_error("GLSL to SPIR-V compilation is not available. Provide precompiled SPIR-V (" + filename + ")");
}

Shader::ReflectionInfo Shader::reflect() const {
    // Reflection requires additional dependencies (e.g., SPIRV-Reflect).
    // For now, return empty reflection info.
    return {};
}

void Shader::create_shader_module(const std::vector<uint32_t>& code) {
    VkShaderModuleCreateInfo create_info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    create_info.codeSize = code.size() * sizeof(uint32_t);
    create_info.pCode = code.data();

    if (vkCreateShaderModule(device_, &create_info, nullptr, &shaderModule_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }
}

void Shader::cleanup() {
    if (shaderModule_ != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device_, shaderModule_, nullptr);
        shaderModule_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

bool Shader::validate_spir_v(const std::vector<uint32_t>& spirv_code) {
    return !spirv_code.empty();
}
VkShaderStageFlagBits Shader::infer_stage_from_filename(const std::string& filename) {
    namespace fs = std::filesystem; // NOLINT(readability-identifier-naming)
    std::string extension = fs::path(filename).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (extension == ".vert") {
        return VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (extension == ".frag" || extension == ".fs") {
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (extension == ".comp") {
        return VK_SHADER_STAGE_COMPUTE_BIT;
    }
    if (extension == ".geom") {
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    }
    if (extension == ".tesc") {
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    }
    if (extension == ".tese") {
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    }
    return VK_SHADER_STAGE_VERTEX_BIT;
}

} // namespace VulkanEngine::RAII

