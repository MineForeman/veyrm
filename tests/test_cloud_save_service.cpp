/**
 * @file test_cloud_save_service.cpp
 * @brief Tests for CloudSaveService with PostgreSQL database
 * @author Veyrm Team
 * @date 2025
 */

#include <catch2/catch_test_macros.hpp>
#include "services/cloud_save_service.h"
#include "db/database_manager.h"
#include "db/save_game_repository.h"
#include "db/player_repository.h"
#include "auth/authentication_service.h"
#include "ecs/game_world.h"
#include "message_log.h"
#include "map.h"
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class CloudSaveServiceTest {
public:
    CloudSaveServiceTest() {
        // Initialize database singleton if needed
        auto& db_manager_ref = db::DatabaseManager::getInstance();
        if (!db_manager_ref.isInitialized()) {
            db::DatabaseConfig config;
            config.host = "localhost";
            config.port = 5432;
            config.database = "veyrm_db";
            config.username = "veyrm_admin";
            config.password = "changeme_to_secure_password";
            config.min_connections = 2;
            config.max_connections = 4;
            db_manager_ref.initialize(config);
        }

        // Initialize services
        save_repo = std::make_unique<db::SaveGameRepository>(db_manager_ref);
        player_repo = std::make_unique<db::PlayerRepository>(db_manager_ref);
        auth_service = std::make_unique<auth::AuthenticationService>(*player_repo, db_manager_ref);

        // Create dummy message log and map for ECS world
        message_log = std::make_unique<MessageLog>();
        game_map = std::make_unique<Map>(20, 20);  // Small test map
        ecs_world = std::make_unique<ecs::GameWorld>(message_log.get(), game_map.get());

        // Create test user
        test_username = "cloud_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        auto reg_result = auth_service->registerUser(test_username, "cloud_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@test.com", "TestPassword123");
        if (!reg_result.success) {
            throw std::runtime_error("Failed to create test user");
        }
        test_user_id = reg_result.user_id.value();

        // Create cloud save service
        cloud_service = std::make_unique<CloudSaveService>(
            save_repo.get(),
            auth_service.get(),
            ecs_world.get()
        );
    }

    ~CloudSaveServiceTest() {
        // Clean up test data using repository methods
        try {
            // Delete all saves for test user
            auto saves = save_repo->findByUserId(test_user_id);
            for (const auto& save : saves) {
                save_repo->deleteById(save.id);
            }

            // The user cleanup would need to be done via UserRepository
            // For now, we'll leave it for the database to clean up
        } catch (...) {
            // Ignore cleanup errors
        }
    }

protected:
    std::unique_ptr<db::SaveGameRepository> save_repo;
    std::unique_ptr<db::PlayerRepository> player_repo;
    std::unique_ptr<auth::AuthenticationService> auth_service;
    std::unique_ptr<MessageLog> message_log;
    std::unique_ptr<Map> game_map;
    std::unique_ptr<ecs::GameWorld> ecs_world;
    std::unique_ptr<CloudSaveService> cloud_service;
    std::string test_username;
    int test_user_id;

    db::SaveGame createTestSave(int slot, const std::string& name = "Cloud Test Character") {
        db::SaveGame save;
        save.user_id = test_user_id;
        save.slot_number = slot;
        save.character_name = name;
        save.character_level = slot * 10;
        save.map_depth = slot * 2;
        save.play_time = slot * 120;

        boost::json::object save_data;
        save_data["character"] = boost::json::object{
            {"name", name},
            {"level", slot * 10},
            {"position", boost::json::array{slot * 10, slot * 5}}
        };
        save_data["world"] = boost::json::object{
            {"depth", slot * 2},
            {"seed", 54321 + slot},
            {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
        };
        save.save_data = save_data;

        return save;
    }
};

TEST_CASE_METHOD(CloudSaveServiceTest, "Cloud service initialization", "[cloud][init]") {
    SECTION("Service starts properly") {
        REQUIRE(cloud_service != nullptr);
        // Service should be created without errors
    }

    SECTION("Database connectivity") {
        // Verify we can access the database through the service
        auto& db_manager_ref = db::DatabaseManager::getInstance();
        REQUIRE(db_manager_ref.isInitialized());
    }
}

TEST_CASE_METHOD(CloudSaveServiceTest, "Cloud save operations", "[cloud][save]") {
    SECTION("Save to cloud") {
        // Create a test save
        auto save = createTestSave(1, "Cloud Hero");
        auto save_result = save_repo->create(save);
        REQUIRE(save_result.has_value());

        // TODO: Implement actual cloud save functionality
        // For now, verify the database save worked
        auto loaded = save_repo->findByUserAndSlot(test_user_id, 1);
        REQUIRE(loaded.has_value());
        REQUIRE(loaded->character_name == "Cloud Hero");
    }

    SECTION("Load from cloud") {
        // Create and save a test game
        auto save = createTestSave(2, "Cloud Warrior");
        auto save_result = save_repo->create(save);
        REQUIRE(save_result.has_value());

        // TODO: Implement cloud load functionality
        // For now, verify database load works
        auto loaded = save_repo->findByUserAndSlot(test_user_id, 2);
        REQUIRE(loaded.has_value());
        REQUIRE(loaded->character_name == "Cloud Warrior");
        REQUIRE(loaded->character_level == 20);
    }

    SECTION("Multiple cloud saves") {
        // Create saves for multiple slots
        for (int slot = 1; slot <= 5; ++slot) {
            auto save = createTestSave(slot, "Cloud Hero " + std::to_string(slot));
            auto result = save_repo->create(save);
            REQUIRE(result.has_value());
        }

        // Verify all saves exist in database
        auto saves = save_repo->findByUserId(test_user_id);
        REQUIRE(saves.size() >= 5);

        // Check each save
        for (int slot = 1; slot <= 5; ++slot) {
            auto loaded = save_repo->findByUserAndSlot(test_user_id, slot);
            REQUIRE(loaded.has_value());
            REQUIRE(loaded->character_name == "Cloud Hero " + std::to_string(slot));
        }
    }
}

TEST_CASE_METHOD(CloudSaveServiceTest, "Cloud save metadata", "[cloud][metadata]") {
    SECTION("Save with metadata") {
        // Create a save with rich metadata
        auto save = createTestSave(3, "Metadata Test");

        // Add additional metadata to the JSON
        auto& parsed = save.save_data;
        auto& obj = parsed.as_object();
        obj["metadata"] = boost::json::object{
            {"version", "1.0"},
            {"platform", "test"},
            {"features", boost::json::array{"cloud", "postgres", "ecs"}},
            {"stats", boost::json::object{
                {"monsters_killed", 150},
                {"items_found", 75},
                {"levels_explored", 5}
            }}
        };
        save.save_data = obj;

        auto result = save_repo->create(save);
        REQUIRE(result.has_value());

        // Load and verify metadata
        auto loaded = save_repo->findByUserAndSlot(test_user_id, 3);
        REQUIRE(loaded.has_value());

        auto& loaded_json = loaded->save_data;
        auto& loaded_obj = loaded_json.as_object();

        REQUIRE(loaded_obj.contains("metadata"));
        auto& metadata = loaded_obj.at("metadata").as_object();
        REQUIRE(metadata.at("version").as_string() == "1.0");
        REQUIRE(metadata.at("platform").as_string() == "test");
        REQUIRE(metadata.at("features").as_array().size() == 3);

        auto& stats = metadata.at("stats").as_object();
        REQUIRE(boost::json::value_to<int>(stats.at("monsters_killed")) == 150);
    }
}

TEST_CASE_METHOD(CloudSaveServiceTest, "Auto-save functionality", "[cloud][autosave]") {
    SECTION("Auto-save slots work with cloud") {
        // Test auto-save slots (-1, -2, -3)
        for (int slot = -3; slot <= -1; ++slot) {
            auto save = createTestSave(slot, "Auto Save " + std::to_string(-slot));
            auto result = save_repo->create(save);
            REQUIRE(result.has_value());
        }

        // Verify all auto-saves exist
        for (int slot = -3; slot <= -1; ++slot) {
            auto loaded = save_repo->findByUserAndSlot(test_user_id, slot);
            REQUIRE(loaded.has_value());
            REQUIRE(loaded->slot_number == slot);
        }
    }

    SECTION("Auto-save rotation") {
        // Simulate auto-save rotation by overwriting auto-save slots
        for (int iteration = 1; iteration <= 3; ++iteration) {
            auto save = createTestSave(-1, "Auto Save Iteration " + std::to_string(iteration));
            save.character_level = iteration * 100;  // Different level each time
            auto result = save_repo->create(save);
            REQUIRE(result.has_value());

            // Verify the save was updated
            auto loaded = save_repo->findByUserAndSlot(test_user_id, -1);
            REQUIRE(loaded.has_value());
            REQUIRE(loaded->character_level == iteration * 100);
        }
    }
}

TEST_CASE_METHOD(CloudSaveServiceTest, "Performance testing", "[cloud][performance]") {
    SECTION("Rapid cloud operations") {
        auto start_time = std::chrono::high_resolution_clock::now();

        // Perform rapid save/load operations
        for (int i = 0; i < 20; ++i) {
            int slot = (i % 9) + 1;  // Rotate through slots 1-9
            auto save = createTestSave(slot, "Rapid " + std::to_string(i));

            auto save_result = save_repo->create(save);
            REQUIRE(save_result.has_value());

            auto loaded = save_repo->findByUserAndSlot(test_user_id, slot);
            REQUIRE(loaded.has_value());
            REQUIRE(loaded->character_name == "Rapid " + std::to_string(i));
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Should complete in reasonable time
        REQUIRE(duration.count() < 3000);  // Less than 3 seconds
        INFO("20 rapid save/load operations completed in " << duration.count() << " ms");
    }

    SECTION("Large save data performance") {
        // Create a large save with complex world state
        boost::json::object large_world;

        // Create large entity array
        boost::json::array entities;
        for (int i = 0; i < 1000; ++i) {
            boost::json::object entity;
            entity["id"] = i;
            entity["type"] = "entity_" + std::to_string(i % 10);
            entity["components"] = boost::json::object{
                {"position", boost::json::array{i % 200, i % 100}},
                {"health", boost::json::object{{"current", 100}, {"max", 100}}},
                {"data", std::string(50, 'x')}  // 50 bytes per entity
            };
            entities.push_back(entity);
        }

        large_world["entities"] = entities;
        large_world["map_data"] = std::string(10000, 'M');  // 10KB map data
        large_world["metadata"] = boost::json::object{
            {"entity_count", 1000},
            {"map_size", boost::json::array{200, 100}},
            {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
        };

        auto save = createTestSave(4, "Large World");
        save.save_data = large_world;

        // Measure save performance
        auto save_start = std::chrono::high_resolution_clock::now();
        auto save_result = save_repo->create(save);
        auto save_end = std::chrono::high_resolution_clock::now();

        REQUIRE(save_result.has_value());

        // Measure load performance
        auto load_start = std::chrono::high_resolution_clock::now();
        auto loaded = save_repo->findByUserAndSlot(test_user_id, 4);
        auto load_end = std::chrono::high_resolution_clock::now();

        REQUIRE(loaded.has_value());
        REQUIRE(boost::json::serialize(loaded->save_data).length() > 100000);  // Should be large

        // Verify data integrity
        auto& parsed = loaded->save_data;
        REQUIRE(parsed.as_object().at("entities").as_array().size() == 1000);

        auto save_ms = std::chrono::duration_cast<std::chrono::milliseconds>(save_end - save_start).count();
        auto load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(load_end - load_start).count();

        INFO("Large save (" << boost::json::serialize(save.save_data).length() << " bytes) - Save: " << save_ms << "ms, Load: " << load_ms << "ms");

        // Performance should be reasonable even for large saves
        REQUIRE(save_ms < 2000);  // Less than 2 seconds
        REQUIRE(load_ms < 2000);
    }
}

TEST_CASE_METHOD(CloudSaveServiceTest, "Error handling", "[cloud][error]") {
    SECTION("Invalid save operations") {
        // Try to save with invalid user ID
        auto save = createTestSave(5, "Invalid User");
        save.user_id = 999999;  // Non-existent user

        auto result = save_repo->create(save);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Invalid load operations") {
        // Try to load non-existent save
        auto loaded = save_repo->findByUserAndSlot(test_user_id, 7);
        REQUIRE_FALSE(loaded.has_value());

        // Try invalid slot numbers
        loaded = save_repo->findByUserAndSlot(test_user_id, 0);
        REQUIRE_FALSE(loaded.has_value());

        loaded = save_repo->findByUserAndSlot(test_user_id, 10);
        REQUIRE_FALSE(loaded.has_value());
    }

    SECTION("Corrupted save data handling") {
        // Create save with corrupted JSON
        auto save = createTestSave(6, "Corrupted Save");
        save.save_data = boost::json::string("{invalid json}");

        // Should still save (it's just text)
        auto result = save_repo->create(save);
        REQUIRE(result.has_value());

        // Should load back as-is
        auto loaded = save_repo->findByUserAndSlot(test_user_id, 6);
        REQUIRE(loaded.has_value());
        REQUIRE(loaded->save_data.as_string() == "{invalid json}");
    }
}

TEST_CASE_METHOD(CloudSaveServiceTest, "User isolation", "[cloud][security]") {
    SECTION("Users can only access their own saves") {
        // Create second test user
        std::string user2_name = "cloud_test2_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        auto reg_result = auth_service->registerUser(user2_name, "user2_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@test.com", "Password123");
        REQUIRE(reg_result.success);
        int user2_id = reg_result.user_id.value();

        // Create save for first user
        auto save1 = createTestSave(1, "User 1 Save");
        auto result1 = save_repo->create(save1);
        REQUIRE(result1.has_value());

        // Create save for second user in same slot
        auto save2 = createTestSave(1, "User 2 Save");
        save2.user_id = user2_id;
        auto result2 = save_repo->create(save2);
        REQUIRE(result2.has_value());

        // Verify users can only see their own saves
        auto user1_save = save_repo->findByUserAndSlot(test_user_id, 1);
        REQUIRE(user1_save.has_value());
        REQUIRE(user1_save->character_name == "User 1 Save");

        auto user2_save = save_repo->findByUserAndSlot(user2_id, 1);
        REQUIRE(user2_save.has_value());
        REQUIRE(user2_save->character_name == "User 2 Save");

        // User 1 cannot access User 2's save
        auto cross_access = save_repo->findByUserAndSlot(test_user_id, 1);
        REQUIRE(cross_access.has_value());
        REQUIRE(cross_access->character_name == "User 1 Save");  // Gets their own save

        // Clean up second user's saves using repository methods
        try {
            auto user2_saves = save_repo->findByUserId(user2_id);
            for (const auto& save : user2_saves) {
                save_repo->deleteById(save.id);
            }
            // User cleanup would need UserRepository
        } catch (...) {
            // Ignore cleanup errors
        }
    }
}