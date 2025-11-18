# Shader compilation utilities for Vulkan
find_program(GLSLC_EXECUTABLE glslc HINTS ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE})
find_program(GLSLANG_VALIDATOR glslangValidator HINTS ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE})

if(NOT GLSLC_EXECUTABLE AND NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslc or glslangValidator not found! Please install Vulkan SDK.")
endif()

# Function to compile shaders automatically
function(compile_shaders TARGET_NAME)
    set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${SHADER_SOURCE_DIR}/*.frag"
        "${SHADER_SOURCE_DIR}/*.vert"
        "${SHADER_SOURCE_DIR}/*.comp"
        "${SHADER_SOURCE_DIR}/*.geom"
        "${SHADER_SOURCE_DIR}/*.tesc"
        "${SHADER_SOURCE_DIR}/*.tese"
        "${SHADER_SOURCE_DIR}/*.mesh"
        "${SHADER_SOURCE_DIR}/*.task"
        "${SHADER_SOURCE_DIR}/*.rgen"
        "${SHADER_SOURCE_DIR}/*.rchit"
        "${SHADER_SOURCE_DIR}/*.rmiss"
    )

    set(SPIRV_BINARY_FILES)
    
    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
        
        if(GLSLC_EXECUTABLE)
            add_custom_command(
                OUTPUT ${SPIRV}
                COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
                COMMAND ${GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
                DEPENDS ${GLSL}
                COMMENT "Compiling shader: ${FILE_NAME}"
            )
        else()
            add_custom_command(
                OUTPUT ${SPIRV}
                COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
                COMMAND ${GLSLANG_VALIDATOR} -V --quiet -o ${SPIRV} ${GLSL}
                DEPENDS ${GLSL}
                COMMENT "Compiling shader: ${FILE_NAME}"
            )
        endif()
        
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(GLSL)
    
    add_custom_target(${TARGET_NAME}
        DEPENDS ${SPIRV_BINARY_FILES}
        COMMENT "Compiling shaders"
    )
    
    # Set output directory for runtime access
    set_target_properties(${TARGET_NAME} PROPERTIES
        SHADER_OUTPUT_DIR ${SHADER_BINARY_DIR}
    )
endfunction()

# Function to add shader dependency to a target
function(target_link_shaders TARGET_NAME SHADER_TARGET)
    add_dependencies(${TARGET_NAME} ${SHADER_TARGET})
    
    # Get shader output directory
    get_target_property(SHADER_DIR ${SHADER_TARGET} SHADER_OUTPUT_DIR)
    
    # Define shader directory for the target
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        SHADER_DIR="$<$<CONFIG:Debug>:${SHADER_DIR}>$<$<CONFIG:Release>:${SHADER_DIR}>$<$<CONFIG:RelWithDebInfo>:${SHADER_DIR}>$<$<CONFIG:MinSizeRel>:${SHADER_DIR}>"
    )
endfunction()
