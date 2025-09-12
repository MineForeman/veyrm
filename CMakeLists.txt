cmake_minimum_required(VERSION 3.25)

# Project definition
project(veyrm 
    VERSION 0.0.1
    DESCRIPTION "Modern C++ Roguelike inspired by Angband"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-DNDEBUG>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(
        /W4
        /WX
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/O2>
    )
endif()

# Platform-specific definitions
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif(UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Main executable
add_executable(veyrm src/main.cpp)

# Set executable output directory
set_target_properties(veyrm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Platform-specific settings
if(WIN32)
    # Ensure UTF-8 on Windows
    target_compile_options(veyrm PRIVATE /utf-8)
endif()

# Installation rules
install(TARGETS veyrm DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/veyrm/data)

# Testing (will be expanded in later phases)
enable_testing()
# add_subdirectory(tests) # Uncomment when tests are added

# Print configuration summary
message(STATUS "========================================")
message(STATUS "Veyrm Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Executable: ${CMAKE_BINARY_DIR}/bin/veyrm")
message(STATUS "========================================")