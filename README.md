# VulkanRAIIWrapper

A standalone RAII-centric C++17 wrapper layer around Vulkan objects (Instance, PhysicalDevice selection helpers, Device, Queues, Swapchain helpers, CommandPools, Images/Buffers via VMA, Debug Messenger, etc.). Designed for reuse across multiple Vulkan projects.

## Features

Most features mirror those in the engine that are relevant to the RAII layer:
- **Cross-platform support** - Works on Windows, Linux, and macOS
- **RAII-based resource management** - Automatic cleanup and memory safety
- **Modern C++17** - Leverages contemporary C++ features
- **Dependency management** - Integrated vcpkg support for easy setup

## Status

This library was originally embedded inside `VulkanEngine_v4`, but now lives here so it can evolve independently and be consumed as a focused dependency.

## Integration Overview

Add this repository as a submodule, use CMake FetchContent, or install and consume via `find_package`. It focuses purely on Vulkan object lifetime and resource helpers; higher-level orchestration and rendering examples live in the `VulkanEngine v4` repository.

## Quick Start (as a dependency)

Use one of the options below to bring this wrapper into your project. The target name used here is `VulkanRAIIWrapper`.

## Integrating Into Another Project

### Option 1: Git Submodule
```bash
git submodule add https://github.com/Encoded404/VulkanRAIIWrapper.git external/VulkanRAIIWrapper
git submodule update --init --recursive
```
```cmake
add_subdirectory(external/VulkanRAIIWrapper)
target_link_libraries(YourApp PRIVATE VulkanRAIIWrapper)
```

### Option 2: FetchContent
```cmake
include(FetchContent)
FetchContent_Declare(
  VulkanRAIIWrapper
  GIT_REPOSITORY https://github.com/Encoded404/VulkanRAIIWrapper.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(VulkanRAIIWrapper)
target_link_libraries(YourApp PRIVATE VulkanRAIIWrapper)
```

### Option 3: find_package (Installed)
After you `cmake --install` this library:
```cmake
find_package(VulkanRAIIWrapper CONFIG REQUIRED)
target_link_libraries(YourApp PRIVATE VulkanRAIIWrapper)
```

## Basic Usage Example

See the engine that uses this wrapper: [`VulkanEngine v4`](https://github.com/Encoded404/VulkanEngine_v4/)

## Dependencies

This project uses the following dependencies managed via vcpkg:
- **Volk**: Meta-loader for Vulkan API
- **VMA (Vulkan Memory Allocator)**: Memory allocation library for Vulkan
- **SDL3**: Windowing/input with Vulkan support


## vcpkg Setup

This project uses vcpkg for dependency management. Here's how to set it up:

### 1. Install vcpkg

**Linux/macOS:**
```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh

# Make vcpkg available globally (optional)
export PATH="$PATH:$(pwd)"
export VCPKG_ROOT="$(pwd)"
```

**Windows:**
```cmd
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Make vcpkg available globally (optional)
set PATH=%PATH%;%CD%
set VCPKG_ROOT=%CD%
```

### 2. Set Environment Variable

Set the `VCPKG_ROOT` environment variable to point to your vcpkg installation:

**Linux/macOS (add to ~/.bashrc or ~/.zshrc):**
```bash
export VCPKG_ROOT=/path/to/vcpkg
```

**Windows (add to system environment variables):**
```cmd
set VCPKG_ROOT=C:\path\to\vcpkg
```

### 3. Verify Installation

```bash
# Check vcpkg is working
vcpkg version

# List available packages (optional)
vcpkg search volk
vcpkg search vulkan-memory-allocator
vcpkg search sdl3
```

## Building

Standalone build and install instructions will be added once the CMake scaffolding lands in this repository. For now, prefer integrating via submodule or FetchContent.

## Clang-Tidy (Optional)

If you mirror the engine's setup, you can enable clang-tidy in your consumer project. Wrapper-specific guidance will be added once the CMake targets are published here.

## Troubleshooting

**CMake can't find vcpkg:**
- Ensure `VCPKG_ROOT` environment variable is set correctly
- Verify vcpkg is in your system PATH

**Vulkan SDK not found:**
- Install the Vulkan SDK from [LunarG](https://www.lunarg.com/vulkan-sdk/)
- Ensure `VULKAN_SDK` environment variable is set

**Permission errors on Linux/macOS:**
- Ensure you have write permissions in the build directory
- Consider using `sudo` for system-wide vcpkg installation

## Version Pinning

For reproducible builds, pin a tag or commit (avoid floating `main` in production). Example with FetchContent:
```cmake
FetchContent_Declare(
  VulkanRAIIWrapper
  GIT_REPOSITORY https://github.com/Encoded404/VulkanRAIIWrapper.git
  GIT_TAG        v1.0.0
)
```

## Contributing
Issues and PRs welcome. Keep changes modular and cross-compatible so the wrapper stays lean and engine+os agnostic.
