cmake_minimum_required(VERSION 3.25)

# Project definition
project(veyrm 
    VERSION 0.0.2
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

message(STATUS "========================================")
message(STATUS "Veyrm Build Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# ========================================
# FetchContent for dependencies
# ========================================
include(FetchContent)

# Set FetchContent to be quieter
set(FETCHCONTENT_QUIET ON)

# FTXUI
message(STATUS "Configuring FTXUI...")
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v5.0.0
    GIT_SHALLOW TRUE
)

# nlohmann/json
message(STATUS "Configuring nlohmann/json...")
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

# Catch2
message(STATUS "Configuring Catch2...")
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.1
    GIT_SHALLOW TRUE
)

# Make dependencies available
FetchContent_MakeAvailable(ftxui json Catch2)

message(STATUS "All dependencies configured successfully")

# ========================================
# Compiler configuration
# ========================================
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
    # AddressSanitizer disabled - causes issues with FTXUI
    # Uncomment to enable: add -fsanitize=address to compile options
    # and add_link_options(-fsanitize=address) for debug mode
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(
        /W4
        /WX
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/RTC1>
        $<$<CONFIG:Release>:/O2>
    )
    # UTF-8 support for MSVC
    add_compile_options(/utf-8)
endif()

# Platform-specific definitions
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
    # Windows specific FTXUI color fix
    add_compile_definitions(FTXUI_MICROSOFT_TERMINAL_FALLBACK_COLORS)
elseif(APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif(UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# ========================================
# Include directories
# ========================================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ========================================
# Main executable
# ========================================
add_executable(veyrm 
    src/main.cpp
    src/game_screen.cpp
)

# Link libraries
target_link_libraries(veyrm 
    PRIVATE 
        ftxui::screen
        ftxui::dom
        ftxui::component
        nlohmann_json::nlohmann_json
)

# Set executable output directory
set_target_properties(veyrm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ========================================
# Testing
# ========================================
enable_testing()
add_subdirectory(tests)

# ========================================
# Installation rules
# ========================================
install(TARGETS veyrm DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/veyrm/data)

# ========================================
# Configuration summary
# ========================================
message(STATUS "========================================")
message(STATUS "Veyrm Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Executable: ${CMAKE_BINARY_DIR}/bin/veyrm")
message(STATUS "  Dependencies:")
message(STATUS "    - FTXUI: v5.0.0")
message(STATUS "    - nlohmann/json: v3.11.3")
message(STATUS "    - Catch2: v3.5.1")
message(STATUS "========================================")