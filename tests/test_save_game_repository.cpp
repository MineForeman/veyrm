/**
 * @file test_save_game_repository.cpp
 * @brief Comprehensive tests for SaveGameRepository with real PostgreSQL database
 * @author Veyrm Team
 * @date 2025
 */

#include <catch2/catch_test_macros.hpp>
#include "db/save_game_repository.h"
#include "db/database_manager.h"
#include "db/player_repository.h"
#include "auth/authentication_service.h"
#include "auth/login_models.h"
#include <boost/json.hpp>
#include <chrono>
#include <thread>
#include <set>

class SaveGameRepositoryTest {
public:
    SaveGameRepositoryTest() {
        // Get singleton instance and initialize if needed
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

        repository = std::make_unique<db::SaveGameRepository>(db_manager_ref);
        player_repo = std::make_unique<db::PlayerRepository>(db_manager_ref);
        auth_service = std::make_unique<auth::AuthenticationService>(*player_repo, db_manager_ref);

        // Create test user
        test_username = "repo_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        auto reg_result = auth_service->registerUser(test_username, "repo_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@test.com", "TestPassword123");
        if (!reg_result.success) {
            throw std::runtime_error("Failed to create test user");
        }
        test_user_id = reg_result.user_id.value();
    }

    ~SaveGameRepositoryTest() {
        // Clean up test data using repository methods
        try {
            // Delete all saves for test user
            auto saves = repository->findByUserId(test_user_id);
            for (const auto& save : saves) {
                repository->deleteById(save.id);
            }

            // The user cleanup would need to be done via UserRepository
            // For now, we'll leave it for the database to clean up
        } catch (...) {
            // Ignore cleanup errors
        }
    }

protected:
    std::unique_ptr<db::SaveGameRepository> repository;
    std::unique_ptr<db::PlayerRepository> player_repo;
    std::unique_ptr<auth::AuthenticationService> auth_service;
    std::string test_username;
    int test_user_id;

    db::SaveGame createTestSave(int slot, const std::string& name = "Test Character") {
        db::SaveGame save;
        save.user_id = test_user_id;
        save.slot_number = slot;
        save.character_name = name;
        save.character_level = slot * 5;
        save.map_depth = slot;
        save.play_time = slot * 30;

        boost::json::object save_data;
        save_data["character"] = boost::json::object{
            {"name", name},
            {"level", slot * 5},
            {"position", boost::json::array{slot, slot}}
        };
        save_data["world"] = boost::json::object{
            {"depth", slot},
            {"seed", 12345 + slot}
        };
        save.save_data = save_data;

        return save;
    }
};

TEST_CASE_METHOD(SaveGameRepositoryTest, "Basic save operations", "[repository][save]") {
    SECTION("Save new game") {
        auto save = createTestSave(1, "Hero Alpha");
        auto result = repository->create(save);

        REQUIRE(result.has_value());
        REQUIRE(!result->id.empty());
    }

    SECTION("Load saved game") {
        // Save first
        auto save = createTestSave(2, "Hero Beta");
        auto save_result = repository->create(save);
        REQUIRE(save_result.has_value());

        // Load it back
        auto loaded = repository->findByUserAndSlot(test_user_id, 2);
        REQUIRE(loaded.has_value());

        REQUIRE(loaded->user_id == test_user_id);
        REQUIRE(loaded->slot_number == 2);
        REQUIRE(loaded->character_name == "Hero Beta");
        REQUIRE(loaded->character_level == 10);
        REQUIRE(loaded->map_depth == 2);
        REQUIRE(loaded->play_time == 60);

        // Verify JSON data
        auto& parsed = loaded->save_data;
        REQUIRE(parsed.as_object().at("character").as_object().at("name").as_string() == "Hero Beta");
    }

    SECTION("Overwrite existing save") {
        // Create initial save
        auto save1 = createTestSave(3, "Hero Gamma");
        auto result1 = repository->create(save1);
        REQUIRE(result1.has_value());

        // Overwrite with new data
        auto save2 = createTestSave(3, "Hero Gamma Updated");
        save2.character_level = 50;
        save2.play_time = 1000;
        auto result2 = repository->create(save2);
        REQUIRE(result2.has_value());

        // Verify overwrite
        auto loaded = repository->findByUserAndSlot(test_user_id, 3);
        REQUIRE(loaded.has_value());
        REQUIRE(loaded->character_name == "Hero Gamma Updated");
        REQUIRE(loaded->character_level == 50);
        REQUIRE(loaded->play_time == 1000);
    }
}

TEST_CASE_METHOD(SaveGameRepositoryTest, "Multiple save slots", "[repository][slots]") {
    SECTION("Save to all manual slots (1-9)") {
        for (int slot = 1; slot <= 9; ++slot) {
            auto save = createTestSave(slot, "Hero " + std::to_string(slot));
            auto result = repository->create(save);
            REQUIRE(result.has_value());
        }

        // Verify all slots were saved
        for (int slot = 1; slot <= 9; ++slot) {
            auto loaded = repository->findByUserAndSlot(test_user_id, slot);
            REQUIRE(loaded.has_value());
            REQUIRE(loaded->slot_number == slot);
            REQUIRE(loaded->character_name == "Hero " + std::to_string(slot));
        }
    }

    SECTION("Auto-save slots (-1, -2, -3)") {
        for (int slot = -3; slot <= -1; ++slot) {
            auto save = createTestSave(slot, "Auto Save " + std::to_string(-slot));
            auto result = repository->create(save);
            REQUIRE(result.has_value());
        }

        // Verify auto-save slots
        for (int slot = -3; slot <= -1; ++slot) {
            auto loaded = repository->findByUserAndSlot(test_user_id, slot);
            REQUIRE(loaded.has_value());
            REQUIRE(loaded->slot_number == slot);
            REQUIRE(loaded->character_name == "Auto Save " + std::to_string(-slot));
        }
    }

    SECTION("List user saves") {
        // Create saves in various slots
        std::vector<int> test_slots = {1, 3, 5, 7, 9, -1, -2};
        for (int slot : test_slots) {
            auto save = createTestSave(slot, "Slot " + std::to_string(slot));
            repository->create(save);
        }

        auto saves = repository->findByUserId(test_user_id);
        REQUIRE(saves.size() >= test_slots.size());

        // Check that all our test slots are present
        std::set<int> found_slots;
        for (const auto& save : saves) {
            found_slots.insert(save.slot_number);
        }

        for (int slot : test_slots) {
            REQUIRE(found_slots.count(slot) > 0);
        }
    }
}

TEST_CASE_METHOD(SaveGameRepositoryTest, "Save deletion", "[repository][delete]") {
    SECTION("Delete specific save") {
        // Create a save
        auto save = createTestSave(4, "To Be Deleted");
        auto save_result = repository->create(save);
        REQUIRE(save_result.has_value());

        // Verify it exists
        auto loaded = repository->findByUserAndSlot(test_user_id, 4);
        REQUIRE(loaded.has_value());

        // Delete it
        bool delete_result = repository->deleteByUserAndSlot(test_user_id, 4);
        REQUIRE(delete_result);

        // Verify it's gone
        auto after_delete = repository->findByUserAndSlot(test_user_id, 4);
        REQUIRE_FALSE(after_delete.has_value());
    }

    SECTION("Delete non-existent save") {
        // Try to delete a save that doesn't exist
        bool result = repository->deleteByUserAndSlot(test_user_id, 8);
        // Should return false for non-existent save
        REQUIRE_FALSE(result);
    }
}

TEST_CASE_METHOD(SaveGameRepositoryTest, "Complex save data", "[repository][complex]") {
    SECTION("Large JSON save data") {
        boost::json::object large_data;

        // Create complex world state
        boost::json::array entities;
        for (int i = 0; i < 500; ++i) {
            boost::json::object entity;
            entity["id"] = i;
            entity["type"] = (i % 3 == 0) ? "monster" : "item";
            entity["position"] = boost::json::array{i % 100, i % 50};
            entity["data"] = std::string(100, 'x');  // 100 bytes per entity
            entities.push_back(entity);
        }

        large_data["entities"] = entities;
        large_data["metadata"] = boost::json::object{
            {"version", "1.0"},
            {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()},
            {"checksum", "abc123def456"}
        };

        db::SaveGame save;
        save.user_id = test_user_id;
        save.slot_number = 5;
        save.character_name = "Complex Save";
        save.character_level = 25;
        save.map_depth = 10;
        save.play_time = 500;
        save.save_data = large_data;

        // Save the complex data
        auto start_time = std::chrono::high_resolution_clock::now();
        auto result = repository->create(save);
        auto save_duration = std::chrono::high_resolution_clock::now() - start_time;

        REQUIRE(result.has_value());

        // Load it back
        start_time = std::chrono::high_resolution_clock::now();
        auto loaded = repository->findByUserAndSlot(test_user_id, 5);
        auto load_duration = std::chrono::high_resolution_clock::now() - start_time;

        REQUIRE(loaded.has_value());
        REQUIRE(boost::json::serialize(loaded->save_data).length() > 50000);  // Should be large

        // Verify data integrity
        auto& parsed = loaded->save_data;
        REQUIRE(parsed.as_object().at("entities").as_array().size() == 500);

        // Performance check (should be reasonably fast)
        auto save_ms = std::chrono::duration_cast<std::chrono::milliseconds>(save_duration).count();
        auto load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(load_duration).count();

        REQUIRE(save_ms < 1000);  // Less than 1 second
        REQUIRE(load_ms < 1000);

        INFO("Large save data (" << boost::json::serialize(save.save_data).length() << " bytes) - Save: " << save_ms << "ms, Load: " << load_ms << "ms");
    }
}

TEST_CASE_METHOD(SaveGameRepositoryTest, "Concurrent operations", "[repository][concurrent]") {
    SECTION("Rapid saves to different slots") {
        std::vector<std::thread> threads;
        std::vector<bool> results(5, false);

        // Launch multiple threads saving to different slots
        for (int i = 0; i < 5; ++i) {
            threads.emplace_back([this, i, &results]() {
                auto save = createTestSave(i + 1, "Concurrent " + std::to_string(i));
                auto result = repository->create(save);
                results[i] = result.has_value();
            });
        }

        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }

        // Check all operations succeeded
        for (bool result : results) {
            REQUIRE(result);
        }

        // Verify all saves exist
        for (int i = 1; i <= 5; ++i) {
            auto loaded = repository->findByUserAndSlot(test_user_id, i);
            REQUIRE(loaded.has_value());
            REQUIRE(loaded->character_name == "Concurrent " + std::to_string(i - 1));
        }
    }
}

TEST_CASE_METHOD(SaveGameRepositoryTest, "Error conditions", "[repository][error]") {
    SECTION("Invalid user ID") {
        auto save = createTestSave(1, "Invalid User");
        save.user_id = 999999;  // Non-existent user

        auto result = repository->create(save);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Invalid slot numbers") {
        // Test invalid slot numbers
        auto loaded = repository->findByUserAndSlot(test_user_id, 0);    // Invalid
        REQUIRE_FALSE(loaded.has_value());

        loaded = repository->findByUserAndSlot(test_user_id, 10);   // Too high
        REQUIRE_FALSE(loaded.has_value());

        loaded = repository->findByUserAndSlot(test_user_id, -4);   // Too low
        REQUIRE_FALSE(loaded.has_value());
    }

    SECTION("Malformed JSON data") {
        db::SaveGame save = createTestSave(6, "Bad JSON");
        save.save_data = boost::json::string("{invalid json data}");  // Malformed JSON as string value

        // Repository should still save it (it's just text data)
        auto result = repository->create(save);
        REQUIRE(result.has_value());

        // Should be able to load it back as-is
        auto loaded = repository->findByUserAndSlot(test_user_id, 6);
        REQUIRE(loaded.has_value());
        REQUIRE(loaded->save_data.as_string() == "{invalid json data}");
    }
}