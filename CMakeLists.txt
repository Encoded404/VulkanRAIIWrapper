cmake_minimum_required(VERSION 3.15)

# Set vcpkg toolchain before project() if not already set
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

project(VulkanRAIIWrapper VERSION 0.1.0 LANGUAGES CXX)

# Enable multi-config generator support
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include build configuration
include(cmake/BuildConfig.cmake)


# Optional clang-tidy integration (can be toggled via cache)
option(ENABLE_CLANG_TIDY "Run clang-tidy during builds (may slow compilation)" ON)
set(CLANG_TIDY_ARGS "" CACHE STRING "Additional arguments to pass to clang-tidy (e.g. -checks=*)")

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-19 clang-tidy-18 clang-tidy-17)
    if(CLANG_TIDY_EXE)
        # Configure CMake to invoke clang-tidy for C++ sources
        # Only append CLANG_TIDY_ARGS when it's non-empty to avoid creating
        # an empty list element which results in an extra empty argument
        # being passed to the clang-tidy wrapper (causes errors).
        if(NOT "${CLANG_TIDY_ARGS}" STREQUAL "")
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--quiet;${CLANG_TIDY_ARGS}")
        else()
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--quiet")
        endif()
        message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXE}")
    else()
        message(WARNING "ENABLE_CLANG_TIDY=ON but clang-tidy was not found in PATH")
    endif()
else()
    message(STATUS "clang-tidy disabled")
endif()


# Export compile commands for tooling like clang-tidy/run-clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Set output directories for different configurations
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# For multi-config generators
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/${CONFIG}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/${CONFIG}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/${CONFIG}/bin)
endforeach()

# Enable modern C++ features
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable compiler cache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()


find_package(Vulkan REQUIRED)

# Try to find packages via vcpkg first, fallback to system packages
find_package(volk CONFIG QUIET)
if(NOT volk_FOUND)
    message(STATUS "volk not found via vcpkg, trying system packages...")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(VOLK QUIET volk)
        if(VOLK_FOUND)
            message(STATUS "Using system Volk")
            add_library(volk::volk INTERFACE IMPORTED)
            target_include_directories(volk::volk INTERFACE ${VOLK_INCLUDE_DIRS})
            target_link_libraries(volk::volk INTERFACE ${VOLK_LIBRARIES})
        endif()
    endif()
    
    if(NOT VOLK_FOUND)
        message(FATAL_ERROR "Volk not found. Please install via vcpkg or system package manager.")
    endif()
else()
    message(STATUS "Using volk from vcpkg")
endif()

find_package(VulkanMemoryAllocator CONFIG QUIET)
if(NOT VulkanMemoryAllocator_FOUND)
    message(STATUS "VMA not found via vcpkg, trying to find vk_mem_alloc.h manually...")
    find_path(VMA_INCLUDE_DIR vk_mem_alloc.h 
        PATHS 
            /usr/include
            /usr/local/include
            ${VULKAN_SDK}/include
        PATH_SUFFIXES 
            vma
    )
    
    if(VMA_INCLUDE_DIR)
        message(STATUS "Found VMA at: ${VMA_INCLUDE_DIR}")
        add_library(GPUOpen::VulkanMemoryAllocator INTERFACE IMPORTED)
        target_include_directories(GPUOpen::VulkanMemoryAllocator INTERFACE ${VMA_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "VMA not found. Please install via vcpkg or system package manager.")
    endif()
else()
    message(STATUS "Using VMA from vcpkg")
endif()

# Include subdirectories

# Build the RAII library
add_subdirectory(src)

# Option to control whether tests are configured and built
option(BUILD_TESTS "Build and run tests" ON)

# Enable testing if this is the main project and tests are enabled
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(BUILD_TESTS)
        include(CTest)
        add_subdirectory(tests)
    endif()
endif()


# ------------------------------------------------------------
# Very slow, opt-in: run clang-tidy across the whole codebase
# ------------------------------------------------------------
# This adds a custom target that scans all source files under src/ and tests/
# and runs clang-tidy using the project's compile commands.
# NOTE: This can take a long time on larger projects.
option(CLANG_TIDY_CHECK_ALL "Add 'clang-tidy-all' target to lint all sources (VERY SLOW)" OFF)

if(CLANG_TIDY_CHECK_ALL)
    find_program(CLANG_TIDY_EXE_ALL NAMES clang-tidy clang-tidy-19 clang-tidy-18 clang-tidy-17)
    if(NOT CLANG_TIDY_EXE_ALL)
        message(WARNING "CLANG_TIDY_CHECK_ALL=ON but clang-tidy was not found in PATH; target will not be added.")
    elseif(NOT EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json)
        message(STATUS "Enabling export of compile_commands.json for clang-tidy-all")
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    endif()

    if(CLANG_TIDY_EXE_ALL)
        # Tune header filtering and extra args
        set(CLANG_TIDY_ALL_HEADER_FILTER "${CMAKE_SOURCE_DIR}/.*" CACHE STRING "Regex passed to -header-filter for clang-tidy-all")
        set(CLANG_TIDY_ALL_EXTRA_ARGS "" CACHE STRING "Extra args for clang-tidy-all, e.g. -checks=*-*,modernize-*,-bugprone-*,...")
        option(CLANG_TIDY_ALL_FAIL_ON_ERROR "Fail the build if any clang-tidy invocation errors" ON)
        include(cmake/ClangTidyAllParallel.cmake)
    endif()
endif()
