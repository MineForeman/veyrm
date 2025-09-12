# Tests configuration with Catch2

# Create test executable
add_executable(veyrm_tests
    test_main.cpp
    test_json.cpp
    test_basic.cpp
)

# Link with Catch2 and other dependencies
target_link_libraries(veyrm_tests
    PRIVATE
        Catch2::Catch2WithMain
        nlohmann_json::nlohmann_json
)

# Add compile definitions for tests
target_compile_definitions(veyrm_tests PRIVATE
    TESTING_MODE
)

# Register tests with CTest
include(CTest)
include(Catch)
catch_discover_tests(veyrm_tests)

# Set test output directory
set_target_properties(veyrm_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "Tests configured with Catch2")