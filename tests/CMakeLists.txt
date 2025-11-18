# Tests for VulkanRAIIWrapper
include(FetchContent)

# Fetch Catch2 for testing
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(Catch2)

# Test executable
add_executable(VulkanRAIIWrapperTests
    test_main.cpp
)

target_compile_features(VulkanRAIIWrapperTests PRIVATE cxx_std_17)

# Compiler warnings
target_compile_options(VulkanRAIIWrapperTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>
)

# Link libraries
target_link_libraries(VulkanRAIIWrapperTests PRIVATE 
    VulkanRAIIWrapper 
    Catch2::Catch2WithMain
)

# Register tests with CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(VulkanRAIIWrapperTests)
