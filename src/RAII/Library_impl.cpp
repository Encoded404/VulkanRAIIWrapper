// library_impl.cpp

// Volk implementation (if using header-only mode)
#define VOLK_IMPLEMENTATION
#include <volk.h>

// VMA implementation
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

// Any other header-only library implementations
// #define STBI_IMPLEMENTATION
// #include <stb_image.h>

// #define TINYOBJLOADER_IMPLEMENTATION
// #include <tiny_obj_loader.h>