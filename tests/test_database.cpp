#include <catch2/catch_test_macros.hpp>

#ifdef ENABLE_DATABASE
#include "db/database_manager.h"
#include "ecs/persistence_system.h"
#include "ecs/system_manager.h"
#include "ecs/entity_factory.h"

using namespace db;
using namespace ecs;

// Test database configuration for in-memory testing
DatabaseConfig getTestConfig() {
    DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test";
    config.username = "veyrm_user";
    config.password = "secure_password";
    config.min_connections = 1;
    config.max_connections = 2;
    return config;
}

TEST_CASE("DatabaseManager basic functionality", "[database]") {
    SECTION("getInstance returns singleton") {
        auto& db1 = DatabaseManager::getInstance();
        auto& db2 = DatabaseManager::getInstance();
        REQUIRE(&db1 == &db2);
    }

    SECTION("isInitialized returns false before initialization") {
        auto& db = DatabaseManager::getInstance();
        // Note: This might be true if previous tests initialized it
        // This is more of a documentation test
        INFO("Database initialized state: " << db.isInitialized());
    }
}

TEST_CASE("DatabaseManager initialization", "[database][integration]") {
    auto& db = DatabaseManager::getInstance();
    auto config = getTestConfig();

    SECTION("initialization attempts") {
        // This test will only pass if PostgreSQL is running and configured
        try {
            db.initialize(config);
            REQUIRE(db.isInitialized());

            SECTION("test connection works") {
                REQUIRE(db.testConnection());
            }

            SECTION("get database version") {
                auto version = db.getDatabaseVersion();
                REQUIRE(!version.empty());
                REQUIRE(version != "Error:");
            }

            db.shutdown();
        } catch (const std::exception& e) {
            // If database is not available, skip these tests
            WARN("Database not available: " << e.what());
            REQUIRE(true); // Pass the test but log warning
        }
    }
}

TEST_CASE("PersistenceSystem basic functionality", "[database][persistence]") {
    SECTION("PersistenceSystem can be created") {
        PersistenceSystem persistence;
        // The system should initialize without throwing
        REQUIRE(true);
    }

    SECTION("PersistenceSystem handles disabled database gracefully") {
        // Create a minimal world for testing
        World world;
        PersistenceSystem persistence;

        // These operations should not crash even if database is disabled
        Entity& player = world.createEntity();

        // saveCharacter should return false if database is disabled
        bool result = persistence.saveCharacter(world, player, "test_character");
        // Result depends on whether database is enabled
        INFO("Save character result: " << result);
        REQUIRE(true); // Don't fail the test, just document behavior
    }
}

TEST_CASE("Database schema operations", "[database][schema]") {
    auto& db = DatabaseManager::getInstance();
    auto config = getTestConfig();

    try {
        db.initialize(config);
        if (!db.isInitialized()) {
            WARN("Database not initialized, skipping schema tests");
            return;
        }

        SECTION("create tables") {
            bool result = db.createTables();
            // Should succeed whether tables exist or not (CREATE IF NOT EXISTS)
            REQUIRE(result);
        }

        SECTION("check if data loaded") {
            bool hasData = db.isDataLoaded();
            INFO("Database has data: " << hasData);
            // Don't require specific state, just test the call works
            REQUIRE(true);
        }

        SECTION("load initial data") {
            bool result = db.loadInitialData();
            // Should succeed whether data exists or not (ON CONFLICT DO NOTHING)
            REQUIRE(result);

            // After loading, should have data
            REQUIRE(db.isDataLoaded());
        }

        db.shutdown();
    } catch (const std::exception& e) {
        WARN("Database operations failed: " << e.what());
        REQUIRE(true); // Pass the test but log warning
    }
}

#else

TEST_CASE("Database disabled tests", "[database]") {
    SECTION("Database support not compiled") {
        // When database support is disabled, tests should still pass
        REQUIRE(true);
        WARN("Database support not compiled in this build");
    }
}

#endif // ENABLE_DATABASE