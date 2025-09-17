# Tests configuration with Catch2

# Create test executable
add_executable(veyrm_tests
    test_main.cpp
    test_json.cpp
    test_basic.cpp
    test_map.cpp
    # Legacy entity tests removed - using ECS
    # test_entity.cpp
    # test_player.cpp
    # test_entity_manager.cpp
    # test_monster.cpp
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
    test_monster_integration.cpp
    test_config.cpp
    # test_combat_system.cpp removed - using ECS CombatSystem
    # test_item.cpp  # Legacy - removed, using ECS ItemComponent
    # test_inventory.cpp  # Legacy - removed, using ECS InventoryComponent
    test_ecs.cpp
    test_ecs_factory.cpp
    test_ecs_systems.cpp
    test_ecs_integration.cpp
    test_data_loader.cpp
    test_save_load_controller.cpp
    test_game_controller.cpp
)

# Add authentication tests if database is enabled
if(ENABLE_DATABASE AND ENABLE_AUTH)
    target_sources(veyrm_tests PRIVATE
        test_authentication.cpp
        test_database.cpp
    )
endif()

# Link with Catch2 and other dependencies
target_link_libraries(veyrm_tests
    PRIVATE
        Catch2::Catch2WithMain
        veyrm_core
)

# Include directories for tests
target_include_directories(veyrm_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
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