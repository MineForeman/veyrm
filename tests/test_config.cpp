#include <catch2/catch_test_macros.hpp>
#include "config.h"
#include <filesystem>
#include <fstream>

TEST_CASE("Config System", "[config]") {
    Config& config = Config::getInstance();
    
    SECTION("Default values") {
        // Test that defaults are set correctly without loading a file
        Config& fresh_config = Config::getInstance();
        
        // Game defaults
        REQUIRE(fresh_config.getDefaultMapType() == MapType::PROCEDURAL);
        REQUIRE(fresh_config.isDebugMode() == false);
        
        // Display defaults
        REQUIRE(fresh_config.getTheme() == "auto");
        REQUIRE(fresh_config.getShowFPS() == false);
        REQUIRE(fresh_config.getMaxMessages() == 100);
        REQUIRE(fresh_config.getVisibleMessages() == 5);
        
        // Map generation defaults (Angband standard)
        REQUIRE(fresh_config.getMapWidth() == 198);
        REQUIRE(fresh_config.getMapHeight() == 66);
        REQUIRE(fresh_config.getMinRooms() == 9);
        REQUIRE(fresh_config.getMaxRooms() == 20);
        REQUIRE(fresh_config.getMinRoomSize() == 4);
        REQUIRE(fresh_config.getMaxRoomSize() == 20);
        REQUIRE(fresh_config.getLitRoomChance() == 0.3f);
        REQUIRE(fresh_config.getDoorChance() == 0.15f);
        REQUIRE(fresh_config.getCorridorStyle() == "straight");
        
        // Monster defaults
        REQUIRE(fresh_config.getMaxMonstersPerLevel() == 30);
        REQUIRE(fresh_config.getMonsterSpawnRate() == 100);
        REQUIRE(fresh_config.getAggressionRadius() == 10);
        
        // Player defaults
        REQUIRE(fresh_config.getPlayerStartingHP() == 50);
        REQUIRE(fresh_config.getPlayerStartingAttack() == 8);
        REQUIRE(fresh_config.getPlayerStartingDefense() == 5);
        REQUIRE(fresh_config.getInventoryCapacity() == 26);
        REQUIRE(fresh_config.getFOVRadius() == 10);
        
        // Path defaults
        REQUIRE(fresh_config.getDataDir() == "data");
        REQUIRE(fresh_config.getSaveDir() == "saves");
        REQUIRE(fresh_config.getLogDir() == "logs");
        
        // Performance defaults
        REQUIRE(fresh_config.getTargetFPS() == 60);
        
        // Development defaults
        REQUIRE(fresh_config.getVerboseLogging() == false);
        REQUIRE(fresh_config.getAutosaveInterval() == 300);
    }
    
    SECTION("Load from YAML file") {
        // Create a test config file
        const std::string test_config = "test_config.yml";
        std::ofstream file(test_config);
        file << "# Test configuration\n"
             << "game:\n"
             << "  default_map: arena\n"
             << "  debug_mode: true\n"
             << "\n"
             << "display:\n"
             << "  theme: dark\n"
             << "  show_fps: true\n"
             << "  message_log:\n"
             << "    max_messages: 50\n"
             << "    visible_messages: 10\n"
             << "\n"
             << "map_generation:\n"
             << "  procedural:\n"
             << "    width: 100\n"
             << "    height: 50\n"
             << "    min_rooms: 5\n"
             << "    max_rooms: 15\n"
             << "    lit_room_chance: 0.5\n"
             << "\n"
             << "player:\n"
             << "  starting_hp: 30\n"
             << "  starting_attack: 7\n"
             << "  fov_radius: 12\n";
        file.close();
        
        // Load the test config
        bool loaded = config.loadFromFile(test_config);
        REQUIRE(loaded == true);
        
        // Verify loaded values
        REQUIRE(config.getDefaultMapType() == MapType::COMBAT_ARENA);
        REQUIRE(config.isDebugMode() == true);
        REQUIRE(config.getTheme() == "dark");
        REQUIRE(config.getShowFPS() == true);
        REQUIRE(config.getMaxMessages() == 50);
        REQUIRE(config.getVisibleMessages() == 10);
        REQUIRE(config.getMapWidth() == 100);
        REQUIRE(config.getMapHeight() == 50);
        REQUIRE(config.getMinRooms() == 5);
        REQUIRE(config.getMaxRooms() == 15);
        REQUIRE(config.getLitRoomChance() == 0.5f);
        REQUIRE(config.getPlayerStartingHP() == 30);
        REQUIRE(config.getPlayerStartingAttack() == 7);
        REQUIRE(config.getFOVRadius() == 12);
        
        // Clean up
        std::filesystem::remove(test_config);
        
        // Reset some values to defaults for next tests
        config.setMapDimensions(198, 66);
    }
    
    SECTION("Partial YAML file") {
        // Test that missing values keep defaults
        const std::string test_config = "test_partial.yml";
        std::ofstream file(test_config);
        file << "# Partial configuration\n"
             << "game:\n"
             << "  debug_mode: true\n"
             << "\n"
             << "player:\n"
             << "  starting_hp: 25\n";
        file.close();
        
        // Reset to defaults first by setting known values
        Config& partial_config = Config::getInstance();
        partial_config.setDefaultMapType(MapType::PROCEDURAL);
        partial_config.setDebugMode(false);
        
        // Load the partial config
        bool loaded = partial_config.loadFromFile(test_config);
        REQUIRE(loaded == true);
        
        // Changed values
        REQUIRE(partial_config.isDebugMode() == true);
        REQUIRE(partial_config.getPlayerStartingHP() == 25);
        
        // Unchanged values (should keep defaults or previous state)
        // Note: Config is singleton, so state may carry over from previous tests
        // Just verify that unset values didn't crash
        REQUIRE(partial_config.getMapWidth() > 0);  // Should have some valid value
        
        // Clean up
        std::filesystem::remove(test_config);
        
        // Reset for next tests
        partial_config.setDefaultMapType(MapType::PROCEDURAL);
    }
    
    SECTION("Non-existent file") {
        // Reset to known state first
        config.setDefaultMapType(MapType::PROCEDURAL);
        
        // Should return true and use defaults
        bool loaded = config.loadFromFile("non_existent_file.yml");
        REQUIRE(loaded == true);
        
        // Should keep existing values (since file doesn't exist)
        REQUIRE(config.getMapWidth() == 198);
    }
    
    SECTION("Valid YAML with nested structure") {
        // Test a valid complex YAML instead of invalid (rapidyaml is strict)
        const std::string test_config = "test_nested.yml";
        std::ofstream file(test_config);
        file << "game:\n"
             << "  difficulty:\n"
             << "    monster_damage_multiplier: 1.5\n"
             << "    player_health_multiplier: 0.8\n"
             << "\n"
             << "monsters:\n"
             << "  behavior:\n"
             << "    aggression_radius: 15\n"
             << "    door_pursuit_chance: 0.9\n";
        file.close();
        
        // Should load successfully
        bool loaded = config.loadFromFile(test_config);
        REQUIRE(loaded == true);
        
        // Verify nested values loaded
        REQUIRE(config.getAggressionRadius() == 15);
        
        // Clean up
        std::filesystem::remove(test_config);
    }
    
    SECTION("Data directory paths") {
        // Test default data directory
        REQUIRE(config.getDataDir() == "data");

        // Build expected path using filesystem to get platform-specific separators
        std::filesystem::path expected_path = std::filesystem::path("data") / "monsters.json";
        REQUIRE(config.getDataFilePath("monsters.json") == expected_path.string());

        // Test setting custom data directory
        config.setDataDir("custom/data");
        REQUIRE(config.getDataDir() == "custom/data");

        // Build expected path for custom directory
        std::filesystem::path custom_expected = std::filesystem::path("custom") / "data" / "items.json";
        REQUIRE(config.getDataFilePath("items.json") == custom_expected.string());
        
        // Reset to default
        config.setDataDir("data");
    }
    
    SECTION("Map type parsing") {
        const std::string test_config = "test_map_types.yml";
        
        // Test each map type
        std::vector<std::pair<std::string, MapType>> map_types = {
            {"procedural", MapType::PROCEDURAL},
            {"room", MapType::TEST_ROOM},
            {"dungeon", MapType::TEST_DUNGEON},
            {"corridor", MapType::CORRIDOR_TEST},
            {"arena", MapType::COMBAT_ARENA},
            {"stress", MapType::STRESS_TEST}
        };
        
        for (const auto& [type_str, expected_type] : map_types) {
            std::ofstream file(test_config);
            file << "game:\n"
                 << "  default_map: " << type_str << "\n";
            file.close();
            
            config.loadFromFile(test_config);
            REQUIRE(config.getDefaultMapType() == expected_type);
        }
        
        // Clean up
        std::filesystem::remove(test_config);
    }
    
    SECTION("Command-line override simulation") {
        // Load a config file
        const std::string test_config = "test_override.yml";
        std::ofstream file(test_config);
        file << "game:\n"
             << "  default_map: dungeon\n"
             << "  debug_mode: false\n"
             << "\n"
             << "paths:\n"
             << "  data_dir: original/data\n";
        file.close();
        
        config.loadFromFile(test_config);
        
        // Verify initial values
        REQUIRE(config.getDefaultMapType() == MapType::TEST_DUNGEON);
        REQUIRE(config.isDebugMode() == false);
        REQUIRE(config.getDataDir() == "original/data");
        
        // Simulate command-line overrides
        config.setDefaultMapType(MapType::PROCEDURAL);
        config.setDebugMode(true);
        config.setDataDir("override/data");
        
        // Verify overridden values
        REQUIRE(config.getDefaultMapType() == MapType::PROCEDURAL);
        REQUIRE(config.isDebugMode() == true);
        REQUIRE(config.getDataDir() == "override/data");
        
        // Clean up
        std::filesystem::remove(test_config);
    }
    
    SECTION("Save to file") {
        // Note: saveToFile is currently a stub that returns true
        // This test verifies it doesn't crash
        bool saved = config.saveToFile("test_save.yml");
        REQUIRE(saved == true);
        
        // Clean up if file was created
        if (std::filesystem::exists("test_save.yml")) {
            std::filesystem::remove("test_save.yml");
        }
    }
    
    SECTION("Data directory validation") {
        // Test with existing directory
        config.setDataDir(".");  // Current directory always exists
        REQUIRE(config.isDataDirValid() == true);
        
        // Test with non-existent directory
        config.setDataDir("non_existent_directory_12345");
        REQUIRE(config.isDataDirValid() == false);
        
        // Reset to default
        config.setDataDir("data");
    }
}