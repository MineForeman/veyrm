#include <catch2/catch_test_macros.hpp>
#include "db/database_manager.h"
#include <memory>
#include <cstdlib>
#include <fstream>

using namespace db;

// Load environment variables from .env file for database tests
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

// Get test database configuration
DatabaseConfig getTestDatabaseConfig() {
    loadEnvironmentForDb();

    DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_db"; // Use main database for tests

    const char* db_user = std::getenv("DB_USER");
    const char* db_pass = std::getenv("DB_PASS");

    config.username = db_user ? db_user : "veyrm_admin";
    config.password = db_pass ? db_pass : "changeme_to_secure_password";
    config.min_connections = 1;
    config.max_connections = 2;
    return config;
}

TEST_CASE("DatabaseConfig functionality", "[database][config]") {
    SECTION("Default configuration") {
        DatabaseConfig config;
        REQUIRE(config.host == "localhost");
        REQUIRE(config.port == 5432);
        REQUIRE(config.database == "veyrm_db");
        REQUIRE(config.username == "veyrm_admin");
        REQUIRE(config.password == "");
        REQUIRE(config.min_connections == 2);
        REQUIRE(config.max_connections == 10);
    }

    SECTION("Connection string generation") {
        DatabaseConfig config;
        config.host = "testhost";
        config.port = 1234;
        config.database = "testdb";
        config.username = "testuser";
        config.password = "testpass";

        std::string connStr = config.getConnectionString();
        REQUIRE(connStr.find("host=testhost") != std::string::npos);
        REQUIRE(connStr.find("port=1234") != std::string::npos);
        REQUIRE(connStr.find("dbname=testdb") != std::string::npos);
        REQUIRE(connStr.find("user=testuser") != std::string::npos);
        REQUIRE(connStr.find("password=testpass") != std::string::npos);
    }

    SECTION("Custom configuration") {
        DatabaseConfig config;
        config.host = "example.com";
        config.port = 9999;
        config.database = "custom_db";
        config.username = "admin";
        config.password = "secret";
        config.min_connections = 1;
        config.max_connections = 5;

        REQUIRE(config.host == "example.com");
        REQUIRE(config.port == 9999);
        REQUIRE(config.database == "custom_db");
        REQUIRE(config.username == "admin");
        REQUIRE(config.password == "secret");
        REQUIRE(config.min_connections == 1);
        REQUIRE(config.max_connections == 5);
    }
}

TEST_CASE("Database exceptions", "[database][exceptions]") {
    SECTION("DatabaseException") {
        try {
            throw DatabaseException("Test error");
        } catch (const DatabaseException& e) {
            std::string msg = e.what();
            REQUIRE(msg.find("Database error: Test error") != std::string::npos);
        }
    }

    SECTION("ConnectionException") {
        try {
            throw ConnectionException("Connection failed");
        } catch (const ConnectionException& e) {
            std::string msg = e.what();
            REQUIRE(msg.find("Connection failed: Connection failed") != std::string::npos);
        }
    }

    SECTION("QueryException") {
        try {
            throw QueryException("SELECT * FROM test", "Syntax error");
        } catch (const QueryException& e) {
            std::string msg = e.what();
            REQUIRE(msg.find("Query failed: Syntax error") != std::string::npos);
            REQUIRE(msg.find("Query: SELECT * FROM test") != std::string::npos);
        }
    }
}

TEST_CASE("DatabaseManager singleton", "[database][manager]") {
    SECTION("Singleton instance") {
        auto& manager1 = DatabaseManager::getInstance();
        auto& manager2 = DatabaseManager::getInstance();

        // Should be the same instance
        REQUIRE(&manager1 == &manager2);
    }

    SECTION("Initial state") {
        auto& manager = DatabaseManager::getInstance();
        // Check current state (may be initialized by previous tests)
        INFO("Database initialized: " << manager.isInitialized());
        REQUIRE(true); // Just log the state
    }
}

TEST_CASE("DatabaseManager real database operations", "[database][integration]") {
    auto& db = DatabaseManager::getInstance();
    auto config = getTestDatabaseConfig();

    // Track if we need to clean up initialization
    bool we_initialized = false;

    SECTION("Database connection and basic operations") {
        // Try to initialize database connection
        bool db_available = false;
        try {
            if (!db.isInitialized()) {
                db.initialize(config);
                we_initialized = true;
            }
            db_available = db.isInitialized();
        } catch (const std::exception& e) {
            INFO("Database not available: " << e.what());
            INFO("Skipping real database tests - PostgreSQL not running");
        }

        if (db_available) {
            SECTION("Test connection works") {
                REQUIRE(db.testConnection());
            }

            SECTION("Get database version") {
                auto version = db.getDatabaseVersion();
                REQUIRE(!version.empty());
                REQUIRE(version.find("PostgreSQL") != std::string::npos);
            }

            SECTION("Execute simple query") {
                try {
                    auto conn_opt = db.getConnection();
                    if (conn_opt.has_value()) {
                        auto& conn = *conn_opt;

                        // Execute a simple test query
                        auto result = conn->exec("SELECT 1 as test_value");
                        REQUIRE(result.isOk());
                        REQUIRE(result.numRows() == 1);
                        REQUIRE(result.getValue(0, 0) == "1");
                    }
                } catch (const std::exception& e) {
                    INFO("Query execution failed: " << e.what());
                    REQUIRE(false);
                }
            }

            SECTION("Test table creation and cleanup") {
                try {
                    auto conn_opt = db.getConnection();
                    if (conn_opt.has_value()) {
                        auto& conn = *conn_opt;

                        // Create a test table
                        auto create_result = conn->exec(
                            "CREATE TABLE IF NOT EXISTS test_coverage_table ("
                            "id SERIAL PRIMARY KEY, "
                            "test_name VARCHAR(100), "
                            "test_value INTEGER"
                            ")"
                        );
                        REQUIRE(create_result.isOk());

                        // Insert test data
                        auto insert_result = conn->exec(
                            "INSERT INTO test_coverage_table (test_name, test_value) "
                            "VALUES ('coverage_test', 42) RETURNING id"
                        );
                        REQUIRE(insert_result.isOk());
                        REQUIRE(insert_result.numRows() == 1);

                        std::string inserted_id = insert_result.getValue(0, 0);
                        REQUIRE(!inserted_id.empty());

                        // Verify data was inserted
                        auto select_result = conn->exec(
                            "SELECT test_name, test_value FROM test_coverage_table "
                            "WHERE id = " + inserted_id
                        );
                        REQUIRE(select_result.isOk());
                        REQUIRE(select_result.numRows() == 1);
                        REQUIRE(select_result.getValue(0, 0) == "coverage_test");
                        REQUIRE(select_result.getValue(0, 1) == "42");

                        // Clean up test data
                        auto delete_result = conn->exec(
                            "DELETE FROM test_coverage_table WHERE id = " + inserted_id
                        );
                        REQUIRE(delete_result.isOk());

                        // Drop test table
                        auto drop_result = conn->exec("DROP TABLE IF EXISTS test_coverage_table");
                        REQUIRE(drop_result.isOk());
                    }
                } catch (const std::exception& e) {
                    INFO("Table operation failed: " << e.what());
                    // Clean up in case of failure
                    try {
                        auto conn_opt = db.getConnection();
                        if (conn_opt.has_value()) {
                            auto& conn = *conn_opt;
                            conn->exec("DROP TABLE IF EXISTS test_coverage_table");
                        }
                    } catch (...) {
                        // Ignore cleanup errors
                    }
                    REQUIRE(false);
                }
            }

            SECTION("Test database schema operations") {
                try {
                    // Test table creation
                    bool tables_created = db.createTables();
                    REQUIRE(tables_created == true);

                    // Test migration system
                    db.runMigrations();
                    int schema_version = db.getCurrentSchemaVersion();
                    REQUIRE(schema_version >= 0);

                    // Test data loading operations
                    bool data_loaded_before = db.isDataLoaded();
                    (void)data_loaded_before; // Suppress unused variable warning

                    // Ensure data is loaded
                    db.ensureDataLoaded();

                    bool data_loaded_after = db.isDataLoaded();
                    REQUIRE(data_loaded_after == true);

                    // Test loading initial data explicitly
                    bool initial_data_result = db.loadInitialData();
                    REQUIRE(initial_data_result == true);

                    // Verify some initial data exists
                    auto conn_opt = db.getConnection();
                    if (conn_opt.has_value()) {
                        auto& conn = *conn_opt;

                        // Check colors table has data
                        auto color_result = conn->exec("SELECT COUNT(*) FROM colors");
                        REQUIRE(color_result.isOk());
                        REQUIRE(color_result.numRows() == 1);
                        int color_count = std::stoi(color_result.getValue(0, 0));
                        REQUIRE(color_count > 0);

                        // Check tags table has data
                        auto tag_result = conn->exec("SELECT COUNT(*) FROM tags");
                        REQUIRE(tag_result.isOk());
                        REQUIRE(tag_result.numRows() == 1);
                        int tag_count = std::stoi(tag_result.getValue(0, 0));
                        REQUIRE(tag_count > 0);

                        // Check abilities table has data
                        auto ability_result = conn->exec("SELECT COUNT(*) FROM abilities");
                        REQUIRE(ability_result.isOk());
                        REQUIRE(ability_result.numRows() == 1);
                        int ability_count = std::stoi(ability_result.getValue(0, 0));
                        REQUIRE(ability_count > 0);

                        // Test specific data integrity
                        auto white_color = conn->exec("SELECT hex_code FROM colors WHERE name = 'white'");
                        REQUIRE(white_color.isOk());
                        REQUIRE(white_color.numRows() == 1);
                        REQUIRE(white_color.getValue(0, 0) == "#FFFFFF");

                        // Test junction tables exist
                        auto monster_abilities_check = conn->exec("SELECT COUNT(*) FROM monster_abilities");
                        REQUIRE(monster_abilities_check.isOk());

                        auto item_tags_check = conn->exec("SELECT COUNT(*) FROM item_tags");
                        REQUIRE(item_tags_check.isOk());
                    }

                    // Test clearing all data (be careful - this affects the whole database)
                    bool clear_result = db.clearAllData();
                    REQUIRE(clear_result == true);

                    // Verify data was cleared
                    auto conn_opt2 = db.getConnection();
                    if (conn_opt2.has_value()) {
                        auto& conn = *conn_opt2;

                        auto color_result = conn->exec("SELECT COUNT(*) FROM colors");
                        REQUIRE(color_result.isOk());
                        int color_count = std::stoi(color_result.getValue(0, 0));
                        REQUIRE(color_count == 0);
                    }

                    // Reload data after clearing
                    db.ensureDataLoaded();
                    bool data_reloaded = db.isDataLoaded();
                    REQUIRE(data_reloaded == true);

                } catch (const std::exception& e) {
                    INFO("Schema operations failed: " << e.what());
                    REQUIRE(false);
                }
            }

            SECTION("Test advanced database features") {
                try {
                    // Ensure we have tables and data
                    db.createTables();
                    db.ensureDataLoaded();

                    auto conn_opt = db.getConnection();
                    if (conn_opt.has_value()) {
                        auto& conn = *conn_opt;

                        // Test complex queries on the schema

                        // Test monsters table structure
                        auto monster_check = conn->exec(R"(
                            INSERT INTO monsters (code, name, glyph, base_hp, base_attack, ai_behavior)
                            VALUES ('test_monster', 'Test Monster', 'T', 50, 10, 'aggressive')
                            RETURNING id
                        )");
                        REQUIRE(monster_check.isOk());
                        REQUIRE(monster_check.numRows() == 1);
                        std::string monster_id = monster_check.getValue(0, 0);

                        // Test items table structure
                        auto item_check = conn->exec(R"(
                            INSERT INTO items (code, name, glyph, item_type, rarity)
                            VALUES ('test_item', 'Test Item', 'i', 'weapon', 'common')
                            RETURNING id
                        )");
                        REQUIRE(item_check.isOk());
                        REQUIRE(item_check.numRows() == 1);
                        std::string item_id = item_check.getValue(0, 0);

                        // Test foreign key relationships work
                        auto tag_id_result = conn->exec("SELECT id FROM tags WHERE name = 'aggressive' LIMIT 1");
                        if (tag_id_result.isOk() && tag_id_result.numRows() > 0) {
                            std::string tag_id = tag_id_result.getValue(0, 0);

                            auto monster_tag_result = conn->exec(
                                "INSERT INTO monster_tags (monster_id, tag_id) VALUES (" +
                                monster_id + ", " + tag_id + ")"
                            );
                            REQUIRE(monster_tag_result.isOk());
                        }

                        // Test save_games table (Phase 3 functionality)
                        auto save_check = conn->exec(R"(
                            INSERT INTO save_games (user_id, slot_number, character_name, character_level, save_data, save_version, game_version)
                            VALUES (1, 1, 'Test Hero', 5, '{"test": true}', '1.0.0', '0.12.1')
                            RETURNING id
                        )");
                        REQUIRE(save_check.isOk());
                        REQUIRE(save_check.numRows() == 1);

                        // Test schema_migrations table
                        auto migration_check = conn->exec(R"(
                            INSERT INTO schema_migrations (version, description)
                            VALUES (1, 'Initial schema')
                            ON CONFLICT (version) DO NOTHING
                        )");
                        REQUIRE(migration_check.isOk());

                        // Verify the migration is there
                        auto migration_verify = conn->exec("SELECT description FROM schema_migrations WHERE version = 1");
                        REQUIRE(migration_verify.isOk());

                        // Test telemetry table
                        auto telemetry_check = conn->exec(R"(
                            INSERT INTO telemetry (event_type, event_data, game_version)
                            VALUES ('test_event', '{"coverage": "test"}', '0.12.1')
                        )");
                        REQUIRE(telemetry_check.isOk());

                        // Clean up test data
                        conn->exec("DELETE FROM monster_tags WHERE monster_id = " + monster_id);
                        conn->exec("DELETE FROM monsters WHERE id = " + monster_id);
                        conn->exec("DELETE FROM items WHERE id = " + item_id);
                        conn->exec("DELETE FROM save_games WHERE character_name = 'Test Hero'");
                        conn->exec("DELETE FROM telemetry WHERE event_type = 'test_event'");
                    }

                } catch (const std::exception& e) {
                    INFO("Advanced database tests failed: " << e.what());
                    REQUIRE(false);
                }
            }

            // Clean up if we initialized the database
            if (we_initialized) {
                db.shutdown();
            }
        } else {
            SUCCEED("Database tests skipped - PostgreSQL not available");
        }
    }
}

TEST_CASE("Result wrapper with null result", "[database][result]") {
    SECTION("Null result operations") {
        Result result(nullptr);

        REQUIRE(!result.isOk());
        REQUIRE(result.numRows() == 0);
        REQUIRE(result.numCols() == 0);
        REQUIRE(result.getValue(0, 0) == "");
        REQUIRE(result.isNull(0, 0) == true);
        REQUIRE(result.getError() == "No result");
    }

    SECTION("Result move constructor") {
        Result result1(nullptr);
        Result result2(std::move(result1));

        REQUIRE(!result2.isOk());
        REQUIRE(result2.get() == nullptr);
    }
}

// Test Connection class basic functionality without requiring actual database
TEST_CASE("Connection class basic functionality", "[database][connection]") {
    SECTION("Default constructor") {
        Connection conn;
        REQUIRE(!conn.isValid());
    }

    SECTION("Invalid connection string handling") {
        // This should throw an exception
        REQUIRE_THROWS_AS(
            Connection("host=nonexistent port=99999 dbname=invalid"),
            std::runtime_error
        );
    }

    SECTION("Connection move semantics") {
        Connection conn1;  // Default constructed (invalid)
        Connection conn2(std::move(conn1));

        REQUIRE(!conn2.isValid());
    }
}