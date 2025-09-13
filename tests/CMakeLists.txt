# Tests configuration with Catch2

# Create test executable
add_executable(veyrm_tests
    test_main.cpp
    test_json.cpp
    test_basic.cpp
    test_map.cpp
    test_entity.cpp
    test_player.cpp
    test_entity_manager.cpp
    test_map_generator.cpp
    test_map_validator.cpp
    test_turn_manager.cpp
    test_input_handler.cpp
    test_message_log.cpp
    test_room_generation.cpp
    test_corridor_generation.cpp
    test_map_validation.cpp
    test_fov.cpp
    test_visibility.cpp
    test_status_bar.cpp
    test_layout_system.cpp
    test_lit_rooms.cpp
    test_monster.cpp
    test_monster_integration.cpp
    test_config.cpp
    test_spawn_manager.cpp
    test_monster_ai.cpp
    test_combat_system.cpp
    test_item.cpp
)

# Link with Catch2 and other dependencies
target_link_libraries(veyrm_tests
    PRIVATE
        Catch2::Catch2WithMain
        nlohmann_json::nlohmann_json
        ftxui::screen
        ftxui::dom
        ftxui::component
        ryml::ryml
)

# Include directories for tests
target_include_directories(veyrm_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Add source files needed for testing
target_sources(veyrm_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/map.cpp
    ${CMAKE_SOURCE_DIR}/src/entity.cpp
    ${CMAKE_SOURCE_DIR}/src/player.cpp
    ${CMAKE_SOURCE_DIR}/src/entity_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/point.cpp
    ${CMAKE_SOURCE_DIR}/src/color_scheme.cpp
    ${CMAKE_SOURCE_DIR}/src/map_generator.cpp
    ${CMAKE_SOURCE_DIR}/src/map_validator.cpp
    ${CMAKE_SOURCE_DIR}/src/turn_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/input_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/message_log.cpp
    ${CMAKE_SOURCE_DIR}/src/game_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/wall_connector.cpp
    ${CMAKE_SOURCE_DIR}/src/frame_stats.cpp
    ${CMAKE_SOURCE_DIR}/src/room.cpp
    ${CMAKE_SOURCE_DIR}/src/fov.cpp
    ${CMAKE_SOURCE_DIR}/src/map_memory.cpp
    ${CMAKE_SOURCE_DIR}/src/status_bar.cpp
    ${CMAKE_SOURCE_DIR}/src/layout_system.cpp
    ${CMAKE_SOURCE_DIR}/src/monster.cpp
    ${CMAKE_SOURCE_DIR}/src/monster_factory.cpp
    ${CMAKE_SOURCE_DIR}/src/config.cpp
    ${CMAKE_SOURCE_DIR}/src/spawn_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/monster_ai.cpp
    ${CMAKE_SOURCE_DIR}/src/pathfinding.cpp
    ${CMAKE_SOURCE_DIR}/src/combat_system.cpp
    ${CMAKE_SOURCE_DIR}/src/log.cpp
    ${CMAKE_SOURCE_DIR}/src/item.cpp
    ${CMAKE_SOURCE_DIR}/src/item_factory.cpp
    ${CMAKE_SOURCE_DIR}/src/item_manager.cpp
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