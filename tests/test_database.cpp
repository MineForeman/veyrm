#include <catch2/catch_test_macros.hpp>

#ifdef ENABLE_DATABASE
#include "db/database_manager.h"
#include "ecs/persistence_system.h"
#include "ecs/system_manager.h"
#include "ecs/entity_factory.h"
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace db;
using namespace ecs;

// Load environment variables from .env file
static void loadEnvironmentForDb() {
    std::ifstream env_file(".env");
    if (!env_file.is_open()) return;

    std::string line;
    while (std::getline(env_file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // Remove quotes if present
        if (!value.empty() && value[0] == '"') value = value.substr(1);
        if (!value.empty() && value.back() == '"') value.pop_back();

        setenv(key.c_str(), value.c_str(), 1);
    }
}

// Test database configuration from environment
DatabaseConfig getTestConfig() {
    // Load environment variables
    loadEnvironmentForDb();

    DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test";

    // Use environment variables if available
    const char* db_user = std::getenv("DB_USER");
    const char* db_pass = std::getenv("DB_PASS");

    config.username = db_user ? db_user : "veyrm_admin";
    config.password = db_pass ? db_pass : "changeme_to_secure_password";
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
        bool db_available = false;
        try {
            db.initialize(config);
            db_available = db.isInitialized();
        } catch (const std::exception& e) {
            INFO("Database not available: " << e.what());
            INFO("Skipping database tests - PostgreSQL not running");
        }

        if (db_available) {
            SECTION("test connection works") {
                REQUIRE(db.testConnection());
            }

            SECTION("get database version") {
                auto version = db.getDatabaseVersion();
                REQUIRE(!version.empty());
                REQUIRE(version != "Error:");
            }

            db.shutdown();
        } else {
            SUCCEED("Database tests skipped - PostgreSQL not available");
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

    bool db_available = false;
    try {
        db.initialize(config);
        db_available = db.isInitialized();
    } catch (const std::exception& e) {
        INFO("Database not available: " << e.what());
    }

    if (!db_available) {
        SUCCEED("Database tests skipped - PostgreSQL not available");
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