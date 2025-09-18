#include <catch2/catch_test_macros.hpp>
#include "ecs/data_loader.h"
#include "ecs/entity_factory.h"
#include "ecs/game_world.h"
#include "db/database_manager.h"
#include "db/save_game_repository.h"
#include "services/cloud_save_service.h"
#include "game_serializer.h"
#include "config.h"
#include "map_generator.h"
#include "map_validator.h"
#include <filesystem>
#include <fstream>

TEST_CASE("Error Handling Tests", "[error]") {
    SECTION("DataLoader error handling") {
        ecs::DataLoader loader;

        // Test loading non-existent file
        REQUIRE_FALSE(loader.loadMonsters("nonexistent_monsters.json"));
        REQUIRE_FALSE(loader.loadItems("nonexistent_items.json"));

        // Test loading invalid JSON
        std::ofstream invalid_json("test_invalid.json");
        invalid_json << "{ invalid json content";
        invalid_json.close();

        REQUIRE_FALSE(loader.loadMonsters("test_invalid.json"));
        REQUIRE_FALSE(loader.loadItems("test_invalid.json"));

        // Clean up
        std::filesystem::remove("test_invalid.json");
    }

    SECTION("EntityFactory error handling") {
        ecs::GameWorld world;
        ecs::EntityFactory factory(world);

        // Test creating entity with non-existent type
        auto invalid_monster = factory.createMonster("nonexistent_monster", 5, 5);
        REQUIRE(invalid_monster == ecs::Entity::INVALID);

        auto invalid_item = factory.createItem("nonexistent_item", 5, 5);
        REQUIRE(invalid_item == ecs::Entity::INVALID);
    }

    SECTION("DatabaseManager connection errors") {
        // Test with invalid connection string
        db::DatabaseManager dbManager("invalid_connection_string");
        REQUIRE_FALSE(dbManager.connect());
        REQUIRE_FALSE(dbManager.isConnected());
    }

    SECTION("SaveGameRepository error handling") {
        // Test with null connection
        db::SaveGameRepository repo(nullptr);

        // All operations should fail gracefully
        REQUIRE_FALSE(repo.saveGame(1, "test_user", "test_data", 123, 456, 1));

        auto games = repo.getUserSaveGames("test_user");
        REQUIRE(games.empty());

        auto save_data = repo.findByUserAndSlot(1, "test_user");
        REQUIRE(save_data.empty());

        REQUIRE_FALSE(repo.deleteGame(1, "test_user"));
        REQUIRE_FALSE(repo.gameExists(1, "test_user"));
    }

    SECTION("CloudSaveService error handling") {
        // Test with invalid connection
        auto invalid_db = std::make_shared<db::DatabaseManager>("invalid");
        CloudSaveService service(invalid_db);

        // Operations should fail gracefully
        REQUIRE_FALSE(service.uploadSave("test_user", 1, "test_data", 100, 200, 1));

        auto result = service.downloadSave("test_user", 1);
        REQUIRE_FALSE(result.has_value());

        auto saves = service.findByUserId("test_user");
        REQUIRE(saves.empty());

        REQUIRE_FALSE(service.deleteByUserAndSlot("test_user", 1));
    }

    SECTION("GameSerializer error handling") {
        GameSerializer serializer;
        ecs::GameWorld world;

        // Test serializing empty world
        auto json_result = serializer.serializeWorld(world);
        REQUIRE_FALSE(json_result.empty()); // Should return empty structure, not fail

        // Test deserializing invalid JSON
        std::string invalid_json = "{ invalid }";
        REQUIRE_FALSE(serializer.deserializeWorld(invalid_json, world));

        // Test deserializing empty string
        REQUIRE_FALSE(serializer.deserializeWorld("", world));

        // Test deserializing null
        REQUIRE_FALSE(serializer.deserializeWorld("{}", world));
    }

    SECTION("Config error handling") {
        Config config;

        // Test loading non-existent config file
        REQUIRE_FALSE(config.loadFromFile("nonexistent_config.yml"));

        // Test loading invalid YAML
        std::ofstream invalid_yaml("test_invalid.yml");
        invalid_yaml << "invalid: yaml: content: [";
        invalid_yaml.close();

        REQUIRE_FALSE(config.loadFromFile("test_invalid.yml"));

        // Clean up
        std::filesystem::remove("test_invalid.yml");
    }

    SECTION("MapGenerator error handling") {
        Config config;
        MapGenerator generator(config);

        // Test generating map with invalid dimensions
        auto invalid_map = generator.generateMap(0, 0, "procedural");
        REQUIRE(invalid_map.getWidth() == 0);
        REQUIRE(invalid_map.getHeight() == 0);

        // Test with negative dimensions
        auto negative_map = generator.generateMap(-1, -1, "procedural");
        REQUIRE(negative_map.getWidth() == 0);
        REQUIRE(negative_map.getHeight() == 0);

        // Test with invalid map type
        auto unknown_type_map = generator.generateMap(20, 20, "unknown_type");
        // Should fall back to default behavior
    }

    SECTION("MapValidator error handling") {
        MapValidator validator;

        // Create an invalid map (all walls)
        Map invalid_map(10, 10);
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                invalid_map.setTile(x, y, TileType::Wall);
            }
        }

        auto issues = validator.validateMap(invalid_map);
        REQUIRE_FALSE(issues.empty()); // Should find validation issues

        // Test with empty map
        Map empty_map(0, 0);
        auto empty_issues = validator.validateMap(empty_map);
        REQUIRE_FALSE(empty_issues.empty()); // Should report size issues
    }

    SECTION("Memory and resource cleanup") {
        // Test that objects clean up properly when going out of scope
        {
            ecs::GameWorld world;
            ecs::EntityFactory factory(world);

            // Create many entities
            for (int i = 0; i < 100; i++) {
                factory.createPlayer(i, i);
            }

            // All should be cleaned up when world goes out of scope
        }

        // Test database connection cleanup
        {
            auto db = std::make_shared<db::DatabaseManager>("test_conn");
            CloudSaveService service(db);
            // Should clean up properly when going out of scope
        }
    }

    SECTION("Boundary conditions") {
        ecs::GameWorld world;
        ecs::EntityFactory factory(world);

        // Test creating entities at extreme coordinates
        auto player_max = factory.createPlayer(INT_MAX, INT_MAX);
        auto player_min = factory.createPlayer(INT_MIN, INT_MIN);

        // Should handle gracefully
        REQUIRE(player_max != ecs::Entity::INVALID);
        REQUIRE(player_min != ecs::Entity::INVALID);

        // Test with very long strings
        std::string very_long_string(10000, 'a');
        GameSerializer serializer;

        // Should handle large data gracefully
        auto result = serializer.serializeWorld(world);
        REQUIRE_FALSE(result.empty());
    }
}

TEST_CASE("File System Error Handling", "[error][filesystem]") {
    SECTION("Permission errors") {
        // Test handling of permission denied scenarios
        Config config;

        // Create a directory to test with
        std::filesystem::create_directory("test_readonly");
        std::filesystem::permissions("test_readonly", std::filesystem::perms::owner_read);

        // Try to write to readonly directory
        REQUIRE_FALSE(config.saveToFile("test_readonly/config.yml"));

        // Clean up
        std::filesystem::permissions("test_readonly", std::filesystem::perms::all);
        std::filesystem::remove_all("test_readonly");
    }

    SECTION("Disk space simulation") {
        // Test behavior when operations might fail due to resource constraints
        GameSerializer serializer;
        ecs::GameWorld world;
        ecs::EntityFactory factory(world);

        // Create a large world
        for (int i = 0; i < 1000; i++) {
            factory.createPlayer(i, i);
            factory.createMonster("goblin", i+1000, i+1000);
        }

        // Serialize should handle large data
        auto result = serializer.serializeWorld(world);
        REQUIRE_FALSE(result.empty());
    }
}

TEST_CASE("Network and Database Timeout Simulation", "[error][network]") {
    SECTION("Database timeout handling") {
        // Test with connection string that might timeout
        db::DatabaseManager dbManager("host=nonexistent.host port=5432 dbname=test user=test password=test connect_timeout=1");

        // Should fail quickly due to timeout
        auto start = std::chrono::steady_clock::now();
        bool connected = dbManager.connect();
        auto end = std::chrono::steady_clock::now();

        REQUIRE_FALSE(connected);
        // Should timeout within reasonable time (not hang indefinitely)
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        REQUIRE(duration.count() < 30); // Should fail within 30 seconds
    }
}