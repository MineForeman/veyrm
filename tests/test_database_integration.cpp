/**
 * @file test_database_integration.cpp
 * @brief Comprehensive PostgreSQL database integration tests
 * @author Veyrm Team
 * @date 2025
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include "db/database_manager.h"
#include "db/save_game_repository.h"
#include "db/player_repository.h"
#include "auth/authentication_service.h"
#include "config.h"
#include <chrono>
#include <thread>
#include <iostream>

using namespace std::chrono_literals;

class DatabaseTestFixture {
public:
    DatabaseTestFixture() {
        // Load test configuration
        config = &Config::getInstance();

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

        // Create repositories
        save_repo = std::make_unique<db::SaveGameRepository>(db_manager_ref);
        player_repo = std::make_unique<db::PlayerRepository>(db_manager_ref);
        auth_service = std::make_unique<auth::AuthenticationService>(*player_repo, db_manager_ref);

        // Clean up any existing test data
        cleanupTestData();
    }

    ~DatabaseTestFixture() {
        cleanupTestData();
    }

    void cleanupTestData() {
        // Note: Since we're using unique usernames with timestamps,
        // we don't need to clean up between test runs.
        // The database will naturally segregate test data.
    }

protected:
    Config* config;
    std::unique_ptr<db::SaveGameRepository> save_repo;
    std::unique_ptr<db::PlayerRepository> player_repo;
    std::unique_ptr<auth::AuthenticationService> auth_service;
};

TEST_CASE_METHOD(DatabaseTestFixture, "Database Connection", "[database][integration]") {
    SECTION("Can connect to PostgreSQL") {
        auto& db_manager_ref = db::DatabaseManager::getInstance();
        REQUIRE(db_manager_ref.isInitialized());

        // Test connection by attempting to create a repository
        REQUIRE(save_repo != nullptr);
        REQUIRE(player_repo != nullptr);
        REQUIRE(auth_service != nullptr);
    }

    SECTION("Database has required functionality") {
        // Test by creating a user - this validates table existence
        const std::string test_username = "connectivity_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        auto result = auth_service->registerUser(test_username, "test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@example.com", "TestPassword123");
        REQUIRE(result.success);
        REQUIRE(result.user_id.has_value());
    }
}

TEST_CASE_METHOD(DatabaseTestFixture, "User Authentication", "[database][auth][integration]") {
    const std::string test_username = "test_user_login_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    const std::string test_password = "TestPassword123";
    const std::string test_email = "test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@example.com";

    SECTION("User registration") {
        auto result = auth_service->registerUser(test_username, test_email, test_password);
        if (!result.success) {
            INFO("Registration failed: " << result.error_message);
        }
        REQUIRE(result.success);
        REQUIRE(result.user_id.value() > 0);
    }

    SECTION("User login after registration") {
        // Register first
        const std::string login_username = test_username + "_login";
        const std::string login_email = "login_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@example.com";
        auto reg_result = auth_service->registerUser(login_username, login_email, test_password);
        REQUIRE(reg_result.success);

        // Then login with the same username we just registered
        auto login_result = auth_service->login(login_username, test_password);
        REQUIRE(login_result.success);
        REQUIRE(login_result.user_id == reg_result.user_id);
    }

    SECTION("Password validation") {
        // Register user
        const std::string password_username = test_username + "_password";
        const std::string password_email = "password_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@example.com";
        auto reg_result = auth_service->registerUser(password_username, password_email, test_password);
        REQUIRE(reg_result.success);

        // Test wrong password
        auto wrong_login = auth_service->login(password_username, "WrongPassword123");
        REQUIRE_FALSE(wrong_login.success);
    }

    SECTION("Duplicate username prevention") {
        const std::string duplicate_username = test_username + "_duplicate";
        const std::string duplicate_email = "duplicate_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@example.com";
        auto result1 = auth_service->registerUser(duplicate_username, duplicate_email, test_password);
        REQUIRE(result1.success);

        auto result2 = auth_service->registerUser(duplicate_username, "different_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@email.com", "DifferentPassword123");
        REQUIRE_FALSE(result2.success);
    }
}

TEST_CASE_METHOD(DatabaseTestFixture, "Save Game Repository", "[database][saves][integration]") {
    const std::string test_username = "test_save_user_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    const std::string test_password = "TestPassword123";
    const std::string test_email = "save_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@example.com";

    // Register a test user first
    auto reg_result = auth_service->registerUser(test_username, test_email, test_password);
    REQUIRE(reg_result.success);
    int user_id = reg_result.user_id.value();

    SECTION("Save game creation") {
        db::SaveGame save_game;
        save_game.user_id = user_id;
        save_game.slot_number = 1;
        save_game.character_name = "Test Hero";
        save_game.character_level = 5;
        save_game.map_depth = 3;
        save_game.play_time = 120;
        save_game.save_data = boost::json::parse(R"({"player": {"level": 5, "hp": 100}, "map": {"depth": 3}})");

        auto save_result = save_repo->create(save_game);
        REQUIRE(save_result.has_value());
        REQUIRE(!save_result->id.empty());
    }

    SECTION("Save game retrieval") {
        // Create a save first
        db::SaveGame save_game;
        save_game.user_id = user_id;
        save_game.slot_number = 2;
        save_game.character_name = "Retrieval Test";
        save_game.character_level = 10;
        save_game.map_depth = 5;
        save_game.play_time = 240;
        save_game.save_data = boost::json::parse(R"({"player": {"level": 10, "hp": 150}})");

        auto save_result = save_repo->create(save_game);
        REQUIRE(save_result.has_value());

        // Retrieve it
        auto retrieved = save_repo->findByUserAndSlot(user_id, 2);
        REQUIRE(retrieved.has_value());
        REQUIRE(retrieved->character_name == "Retrieval Test");
        REQUIRE(retrieved->character_level == 10);
        REQUIRE(retrieved->map_depth == 5);
        REQUIRE(retrieved->play_time == 240);
    }

    SECTION("Multiple save slots") {
        // Create saves in different slots
        for (int slot = 1; slot <= 5; ++slot) {
            db::SaveGame save_game;
            save_game.user_id = user_id;
            save_game.slot_number = slot;
            save_game.character_name = "Hero " + std::to_string(slot);
            save_game.character_level = slot * 2;
            save_game.map_depth = slot;
            save_game.play_time = slot * 60;
            save_game.save_data = boost::json::object{{"slot", slot}};

            auto result = save_repo->create(save_game);
            REQUIRE(result.has_value());
        }

        // List all saves for user
        auto saves = save_repo->findByUserId(user_id);
        REQUIRE(saves.size() >= 5);

        // Check each slot exists
        for (int slot = 1; slot <= 5; ++slot) {
            auto save = save_repo->findByUserAndSlot(user_id, slot);
            REQUIRE(save.has_value());
            REQUIRE(save->character_name == "Hero " + std::to_string(slot));
        }
    }

    SECTION("Save overwrite") {
        // Create initial save
        db::SaveGame save1;
        save1.user_id = user_id;
        save1.slot_number = 3;
        save1.character_name = "Original";
        save1.character_level = 1;
        save1.save_data = boost::json::object{{"version", 1}};

        auto result1 = save_repo->create(save1);
        REQUIRE(result1.has_value());

        // Overwrite same slot
        db::SaveGame save2;
        save2.user_id = user_id;
        save2.slot_number = 3;
        save2.character_name = "Updated";
        save2.character_level = 10;
        save2.save_data = boost::json::object{{"version", 2}};

        auto result2 = save_repo->create(save2);
        REQUIRE(result2.has_value());

        // Verify overwrite
        auto retrieved = save_repo->findByUserAndSlot(user_id, 3);
        REQUIRE(retrieved.has_value());
        REQUIRE(retrieved->character_name == "Updated");
        REQUIRE(retrieved->character_level == 10);
    }

    SECTION("Save deletion") {
        // Create a save
        db::SaveGame save_game;
        save_game.user_id = user_id;
        save_game.slot_number = 9;
        save_game.character_name = "To Delete";
        save_game.save_data = boost::json::object{{"temp", true}};

        auto save_result = save_repo->create(save_game);
        REQUIRE(save_result.has_value());

        // Verify it exists
        auto before_delete = save_repo->findByUserAndSlot(user_id, 9);
        REQUIRE(before_delete.has_value());

        // Delete it
        bool delete_result = save_repo->deleteByUserAndSlot(user_id, 9);
        REQUIRE(delete_result);

        // Verify it's gone
        auto after_delete = save_repo->findByUserAndSlot(user_id, 9);
        REQUIRE_FALSE(after_delete.has_value());
    }
}

TEST_CASE_METHOD(DatabaseTestFixture, "Auto-save functionality", "[database][autosave][integration]") {
    const std::string test_username = "test_autosave_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    const std::string test_password = "TestPassword123";
    const std::string test_email = "autosave_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@example.com";

    auto reg_result = auth_service->registerUser(test_username, test_email, test_password);
    REQUIRE(reg_result.success);
    int user_id = reg_result.user_id.value();

    SECTION("Auto-save slots (-1, -2, -3)") {
        for (int slot = -3; slot <= -1; ++slot) {
            db::SaveGame autosave;
            autosave.user_id = user_id;
            autosave.slot_number = slot;
            autosave.character_name = "Auto " + std::to_string(-slot);
            autosave.character_level = -slot * 5;
            autosave.save_data = boost::json::object{{"auto", true}, {"slot", slot}};

            auto result = save_repo->create(autosave);
            REQUIRE(result.has_value());
        }

        // Verify all auto-saves exist
        for (int slot = -3; slot <= -1; ++slot) {
            auto save = save_repo->findByUserAndSlot(user_id, slot);
            REQUIRE(save.has_value());
            REQUIRE(save->character_name == "Auto " + std::to_string(-slot));
        }
    }
}

TEST_CASE_METHOD(DatabaseTestFixture, "Performance and stress testing", "[database][performance][integration]") {
    const std::string test_username = "test_perf_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    const std::string test_password = "TestPassword123";
    const std::string test_email = "perf_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@example.com";

    auto reg_result = auth_service->registerUser(test_username, test_email, test_password);
    REQUIRE(reg_result.success);
    int user_id = reg_result.user_id.value();

    SECTION("Rapid save operations") {
        auto start = std::chrono::high_resolution_clock::now();

        // Perform 50 rapid saves
        for (int i = 0; i < 50; ++i) {
            db::SaveGame save_game;
            save_game.user_id = user_id;
            save_game.slot_number = (i % 9) + 1;  // Rotate through slots 1-9
            save_game.character_name = "Rapid " + std::to_string(i);
            save_game.character_level = i;
            save_game.save_data = boost::json::parse(R"({"iteration": )" + std::to_string(i) + ", \"large_data\": \"" +
                                 std::string(1000, 'x') + "\"}");  // 1KB of data

            auto result = save_repo->create(save_game);
            REQUIRE(result.has_value());
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Should complete in reasonable time (less than 5 seconds)
        REQUIRE(duration.count() < 5000);
        INFO("50 save operations completed in " << duration.count() << " ms");
    }

    SECTION("Large save data") {
        // Test with large save data (simulating complex game state)
        boost::json::array entities;
        for (int i = 0; i < 1000; ++i) {
            boost::json::object entity;
            entity["id"] = i;
            entity["type"] = "monster";
            entity["pos"] = boost::json::array{i % 100, i % 50};
            entities.push_back(entity);
        }

        boost::json::object large_data;
        large_data["world"] = boost::json::object{{"entities", entities}};

        db::SaveGame large_save;
        large_save.user_id = user_id;
        large_save.slot_number = 1;
        large_save.character_name = "Large Save Test";
        large_save.character_level = 50;
        large_save.save_data = large_data;

        auto save_start = std::chrono::high_resolution_clock::now();
        auto save_result = save_repo->create(large_save);
        auto save_end = std::chrono::high_resolution_clock::now();

        REQUIRE(save_result.has_value());

        auto load_start = std::chrono::high_resolution_clock::now();
        auto loaded = save_repo->findByUserAndSlot(user_id, 1);
        auto load_end = std::chrono::high_resolution_clock::now();

        REQUIRE(loaded.has_value());
        REQUIRE(boost::json::serialize(loaded->save_data).length() > 40000);  // Verify large data was saved

        auto save_time = std::chrono::duration_cast<std::chrono::milliseconds>(save_end - save_start);
        auto load_time = std::chrono::duration_cast<std::chrono::milliseconds>(load_end - load_start);

        INFO("Large save (" << boost::json::serialize(large_save.save_data).length() << " bytes) saved in " << save_time.count() << " ms");
        INFO("Large save loaded in " << load_time.count() << " ms");

        // Should handle large saves efficiently
        REQUIRE(save_time.count() < 1000);  // Less than 1 second
        REQUIRE(load_time.count() < 1000);
    }
}

TEST_CASE_METHOD(DatabaseTestFixture, "Error handling and recovery", "[database][error][integration]") {
    SECTION("Invalid user operations") {
        // Try to save for non-existent user
        db::SaveGame invalid_save;
        invalid_save.user_id = 999999;  // Non-existent user
        invalid_save.slot_number = 1;
        invalid_save.character_name = "Invalid";
        invalid_save.save_data = "{}";

        auto result = save_repo->create(invalid_save);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Invalid slot numbers") {
        const std::string test_username = "test_error_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        auto reg_result = auth_service->registerUser(test_username, "error_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "@test.com", "Password123");
        REQUIRE(reg_result.success);

        // Try invalid slot numbers
        auto save = save_repo->findByUserAndSlot(reg_result.user_id.value(), 10);  // Slot too high
        REQUIRE_FALSE(save.has_value());

        save = save_repo->findByUserAndSlot(reg_result.user_id.value(), 0);  // Invalid slot
        REQUIRE_FALSE(save.has_value());

        save = save_repo->findByUserAndSlot(reg_result.user_id.value(), -4);  // Auto-save slot too low
        REQUIRE_FALSE(save.has_value());
    }
}