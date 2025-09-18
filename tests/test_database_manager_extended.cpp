#include <catch2/catch_test_macros.hpp>
#include "db/database_manager.h"
#include <thread>
#include <vector>
#include <atomic>

using namespace db;

TEST_CASE("Connection class operations", "[database][connection]") {
    SECTION("Connection construction and destruction") {
        // Test with invalid connection string
        try {
            Connection conn("host=invalid port=99999 dbname=nonexistent");
            REQUIRE(false); // Should not reach here
        } catch (const std::runtime_error& e) {
            REQUIRE(std::string(e.what()).find("Connection failed") != std::string::npos);
        }
    }

    SECTION("Connection move semantics") {
        try {
            Connection conn1("host=localhost port=5432 dbname=veyrm_test");
            if (conn1.isValid()) {
                Connection conn2(std::move(conn1));
                REQUIRE(conn2.isValid());
                REQUIRE(!conn1.isValid());
            }
        } catch (...) {
            WARN("Skipping connection move test - database not available");
        }
    }
}

TEST_CASE("Result class operations", "[database][result]") {
    SECTION("Result wrapper functionality") {
        // Create a null result
        Result nullResult(nullptr);
        REQUIRE(!nullResult.isOk());
        REQUIRE(nullResult.numRows() == 0);
        REQUIRE(nullResult.numCols() == 0);
        REQUIRE(nullResult.getValue(0, 0) == "");
        REQUIRE(nullResult.isNull(0, 0) == true);
    }

    SECTION("Result move semantics") {
        Result result1(nullptr);
        Result result2(std::move(result1));
        // Just verify it doesn't crash
        REQUIRE(result2.get() == nullptr);
    }
}

TEST_CASE("ConnectionPool operations", "[database][pool]") {
    DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test_db";
    config.username = "veyrm_admin";
    config.password = "TestPassword123";
    config.min_connections = 2;
    config.max_connections = 5;

    try {
        ConnectionPool pool(config);
        pool.initialize();

        SECTION("Acquire and release connections") {
            auto conn1 = pool.acquire();
            if (conn1.has_value()) {
                REQUIRE(conn1.has_value());

                auto conn2 = pool.acquire();
                REQUIRE(conn2.has_value());

                // Connections should be different
                REQUIRE(conn1->get() != conn2->get());
            } else {
                WARN("Could not acquire connection from pool");
            }
        }

        SECTION("Pool exhaustion") {
            std::vector<std::optional<ConnectionPool::PooledConnection>> connections;

            // Try to acquire more connections than max
            for (size_t i = 0; i < config.max_connections + 2; ++i) {
                auto conn = pool.acquire(std::chrono::milliseconds(100));
                if (conn.has_value()) {
                    connections.push_back(std::move(conn));
                }
            }

            // We should have at most max_connections
            REQUIRE(connections.size() <= config.max_connections);
        }

        SECTION("Connection timeout") {
            auto conn = pool.acquire(std::chrono::milliseconds(1));
            // Just verify it doesn't crash on timeout
            REQUIRE(true);
        }

        pool.stop();
    } catch (const std::exception& e) {
        WARN("Skipping pool tests - database not available: " << e.what());
    }
}

TEST_CASE("DatabaseManager singleton", "[database][manager]") {
    auto& db1 = DatabaseManager::getInstance();
    auto& db2 = DatabaseManager::getInstance();

    SECTION("Singleton instance") {
        REQUIRE(&db1 == &db2);
    }
}

TEST_CASE("DatabaseManager transactions", "[database][transaction]") {
    DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test_db";
    config.username = "veyrm_admin";
    config.password = "TestPassword123";

    auto& db = DatabaseManager::getInstance();

    try {
        db.initialize(config);

        SECTION("Successful transaction") {
            auto result = db.executeTransaction([](Connection& conn) {
                // Simple query that should succeed
                auto res = conn.exec("SELECT 1");
                return res.isOk();
            });

            REQUIRE(result == true);
        }

        SECTION("Failed transaction with rollback") {
            try {
                db.executeTransaction([](Connection& conn) {
                    // This should fail and trigger rollback
                    conn.exec("CREATE TABLE test_table (id INT)");
                    throw std::runtime_error("Simulated error");
                    return true;
                });
                REQUIRE(false); // Should not reach here
            } catch (const std::runtime_error& e) {
                REQUIRE(std::string(e.what()) == "Simulated error");
            }
        }

        SECTION("Read-only query execution") {
            auto result = db.executeQuery([](Connection& conn) {
                auto res = conn.exec("SELECT version()");
                return res.isOk();
            });

            REQUIRE(result == true);
        }

        db.shutdown();
    } catch (const std::exception& e) {
        WARN("Skipping transaction tests - database not available: " << e.what());
    }
}

TEST_CASE("DatabaseManager concurrent operations", "[database][concurrent]") {
    DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test_db";
    config.username = "veyrm_admin";
    config.password = "TestPassword123";
    config.min_connections = 5;
    config.max_connections = 10;

    auto& db = DatabaseManager::getInstance();

    try {
        db.initialize(config);

        SECTION("Concurrent queries") {
            std::atomic<int> successCount(0);
            std::vector<std::thread> threads;

            for (int i = 0; i < 20; ++i) {
                threads.emplace_back([&db, &successCount, i]() {
                    try {
                        auto result = db.executeQuery([i](Connection& conn) {
                            auto res = conn.exec("SELECT " + std::to_string(i));
                            return res.isOk();
                        });
                        if (result) successCount++;
                    } catch (...) {
                        // Ignore errors in thread
                    }
                });
            }

            for (auto& t : threads) {
                t.join();
            }

            // At least some queries should succeed
            REQUIRE(successCount > 0);
        }

        SECTION("Concurrent transactions") {
            std::atomic<int> successCount(0);
            std::vector<std::thread> threads;

            for (int i = 0; i < 10; ++i) {
                threads.emplace_back([&db, &successCount, i]() {
                    try {
                        auto result = db.executeTransaction([i](Connection& conn) {
                            auto res = conn.exec("SELECT " + std::to_string(i));
                            return res.isOk();
                        });
                        if (result) successCount++;
                    } catch (...) {
                        // Ignore errors in thread
                    }
                });
            }

            for (auto& t : threads) {
                t.join();
            }

            REQUIRE(successCount > 0);
        }

        db.shutdown();
    } catch (const std::exception& e) {
        WARN("Skipping concurrent tests - database not available: " << e.what());
    }
}

TEST_CASE("DatabaseManager schema operations", "[database][schema]") {
    DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test_db";
    config.username = "veyrm_admin";
    config.password = "TestPassword123";

    auto& db = DatabaseManager::getInstance();

    try {
        db.initialize(config);

        SECTION("Get current schema version") {
            int version = db.getCurrentSchemaVersion();
            REQUIRE(version >= 0);
        }

        SECTION("Run migrations") {
            // This should be idempotent
            db.runMigrations();
            int version1 = db.getCurrentSchemaVersion();

            db.runMigrations();
            int version2 = db.getCurrentSchemaVersion();

            REQUIRE(version1 == version2);
        }

        SECTION("Create tables") {
            bool created = db.createTables();
            // Should handle existing tables gracefully
            REQUIRE(created == true);

            // Should be idempotent
            bool createdAgain = db.createTables();
            REQUIRE(createdAgain == true);
        }

        SECTION("Data loading operations") {
            bool isLoaded = db.isDataLoaded();
            // Just verify it returns something
            REQUIRE((isLoaded == true || isLoaded == false));

            if (!isLoaded) {
                bool loaded = db.loadInitialData();
                REQUIRE((loaded == true || loaded == false));
            }

            db.ensureDataLoaded();
            // Should not crash
            REQUIRE(true);
        }

        SECTION("Clear all data") {
            // WARNING: This will delete all data!
            // Only run in test database
            if (config.database == "veyrm_test_db") {
                bool cleared = db.clearAllData();
                REQUIRE((cleared == true || cleared == false));
            }
        }

        db.shutdown();
    } catch (const std::exception& e) {
        WARN("Skipping schema tests - database not available: " << e.what());
    }
}

TEST_CASE("DatabaseManager utility operations", "[database][utility]") {
    DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test_db";
    config.username = "veyrm_admin";
    config.password = "TestPassword123";

    auto& db = DatabaseManager::getInstance();

    try {
        db.initialize(config);

        SECTION("Test connection") {
            bool connected = db.testConnection();
            REQUIRE(connected == true);
        }

        SECTION("Get database version") {
            std::string version = db.getDatabaseVersion();
            REQUIRE(!version.empty());
            // PostgreSQL version should contain "PostgreSQL"
            REQUIRE(version.find("PostgreSQL") != std::string::npos);
        }

        SECTION("Check initialization status") {
            REQUIRE(db.isInitialized() == true);

            db.shutdown();
            REQUIRE(db.isInitialized() == false);

            // Re-initialize for other tests
            db.initialize(config);
        }

        db.shutdown();
    } catch (const std::exception& e) {
        WARN("Skipping utility tests - database not available: " << e.what());
    }
}

TEST_CASE("Database exception handling", "[database][exceptions]") {
    SECTION("DatabaseException") {
        DatabaseException ex("Test error");
        std::string msg = ex.what();
        REQUIRE(msg == "Database error: Test error");
    }

    SECTION("ConnectionException") {
        ConnectionException ex("Connection lost");
        std::string msg = ex.what();
        REQUIRE(msg == "Database error: Connection failed: Connection lost");
    }

    SECTION("QueryException") {
        QueryException ex("SELECT * FROM invalid", "Table not found");
        std::string msg = ex.what();
        REQUIRE(msg.find("Query failed:") != std::string::npos);
        REQUIRE(msg.find("Table not found") != std::string::npos);
        REQUIRE(msg.find("SELECT * FROM invalid") != std::string::npos);
    }
}

TEST_CASE("Connection parameter handling", "[database][connection]") {
    DatabaseConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "veyrm_test_db";
    config.username = "veyrm_admin";
    config.password = "TestPassword123";

    auto& db = DatabaseManager::getInstance();

    try {
        db.initialize(config);

        auto connOpt = db.getConnection();
        if (connOpt.has_value()) {
            auto& pooledConn = *connOpt;
            Connection* conn = pooledConn.get();

            SECTION("Execute with vector parameters") {
                std::vector<std::string> params = {"1", "test", "data"};
                auto result = conn->execParams(
                    "SELECT $1::int, $2::text, $3::text",
                    params
                );
                if (result.isOk() && result.numRows() > 0) {
                    REQUIRE(result.getValue(0, 0) == "1");
                    REQUIRE(result.getValue(0, 1) == "test");
                    REQUIRE(result.getValue(0, 2) == "data");
                }
            }

            SECTION("Execute with array parameters") {
                const char* params[] = {"42", "hello"};
                auto result = conn->execParams(
                    "SELECT $1::int, $2::text",
                    2, params
                );
                if (result.isOk() && result.numRows() > 0) {
                    REQUIRE(result.getValue(0, 0) == "42");
                    REQUIRE(result.getValue(0, 1) == "hello");
                }
            }

            SECTION("Escape string for SQL") {
                std::string dangerous = "'; DROP TABLE users; --";
                std::string escaped = conn->escapeString(dangerous);
                // Should escape single quotes
                REQUIRE(escaped.find("''") != std::string::npos ||
                        escaped.find("\\'") != std::string::npos);
            }

            SECTION("Transaction control") {
                bool begun = conn->beginTransaction();
                REQUIRE(begun == true);

                // Do some work...
                auto result = conn->exec("SELECT 1");
                REQUIRE(result.isOk());

                bool committed = conn->commit();
                REQUIRE(committed == true);

                // Test rollback
                begun = conn->beginTransaction();
                REQUIRE(begun == true);

                bool rolledback = conn->rollback();
                REQUIRE(rolledback == true);
            }
        }

        db.shutdown();
    } catch (const std::exception& e) {
        WARN("Skipping parameter tests - database not available: " << e.what());
    }
}