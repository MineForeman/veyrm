#include "db/database_manager.h"
#include "log.h"
#include <thread>
#include <algorithm>

#ifdef ENABLE_DATABASE

namespace db {

std::unique_ptr<DatabaseManager> DatabaseManager::instance = nullptr;

// ConnectionPool Implementation
ConnectionPool::ConnectionPool(const DatabaseConfig& cfg)
    : config(cfg) {
    connections.reserve(config.max_connections);
}

ConnectionPool::~ConnectionPool() {
    stop();
}

void ConnectionPool::createConnection() {
    try {
        auto conn = std::make_unique<Connection>(config.getConnectionString());
        Connection* ptr = conn.get();
        connections.push_back(std::move(conn));
        available.push(ptr);

        Log::info("Database connection created. Pool size: " +
                 std::to_string(connections.size()));
    } catch (const std::exception& e) {
        Log::error("Failed to create database connection: " + std::string(e.what()));
        throw ConnectionException(e.what());
    }
}

void ConnectionPool::validateConnection(Connection* conn) {
    if (!conn) return;

    try {
        // Test the connection with a simple query
        auto result = conn->exec("SELECT 1");
        if (!result.isOk()) {
            throw std::runtime_error("Connection validation failed");
        }
    } catch (const std::exception& e) {
        Log::warn("Connection validation failed, reconnecting: " + std::string(e.what()));

        try {
            *conn = Connection(config.getConnectionString());
        } catch (const std::exception& e2) {
            Log::error("Failed to reconnect: " + std::string(e2.what()));
            throw ConnectionException(e2.what());
        }
    }
}

void ConnectionPool::cleanupStale() {
    // Note: With libpq we can't easily close individual connections in the pool
    // This would need refactoring to support proper connection lifecycle
    // For now, this is a no-op
}

void ConnectionPool::initialize() {
    std::lock_guard<std::mutex> lock(mutex);

    // Create minimum connections
    for (size_t i = 0; i < config.min_connections; ++i) {
        createConnection();
    }

    Log::info("Connection pool initialized with " +
             std::to_string(config.min_connections) + " connections");
}

void ConnectionPool::stop() {
    std::lock_guard<std::mutex> lock(mutex);
    shutdown = true;

    connections.clear();
    while (!available.empty()) {
        available.pop();
    }

    cv.notify_all();
    Log::info("Connection pool stopped");
}

std::optional<ConnectionPool::PooledConnection>
ConnectionPool::acquire(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex);

    if (shutdown) {
        return std::nullopt;
    }

    // Wait for available connection or timeout
    auto deadline = std::chrono::steady_clock::now() + timeout;

    while (available.empty() && !shutdown) {
        // Try to create new connection if under limit
        if (connections.size() < config.max_connections) {
            try {
                createConnection();
                break;
            } catch (const ConnectionException&) {
                // Failed to create, wait for existing
            }
        }

        if (cv.wait_until(lock, deadline) == std::cv_status::timeout) {
            Log::warn("Connection pool timeout after " +
                     std::to_string(timeout.count()) + "ms");
            return std::nullopt;
        }
    }

    if (shutdown || available.empty()) {
        return std::nullopt;
    }

    auto* conn = available.front();
    available.pop();

    // Validate connection before use
    validateConnection(conn);
    conn->updateLastUsed();

    return PooledConnection(this, conn);
}

void ConnectionPool::release(Connection* conn) {
    std::lock_guard<std::mutex> lock(mutex);

    if (!conn || shutdown) return;

    conn->updateLastUsed();
    available.push(conn);

    cv.notify_one();
}

ConnectionPool::PooledConnection::~PooledConnection() {
    if (pool && conn) {
        pool->release(conn);
    }
}

// DatabaseManager Implementation
DatabaseManager& DatabaseManager::getInstance() {
    if (!instance) {
        instance.reset(new DatabaseManager());
    }
    return *instance;
}

void DatabaseManager::initialize(const DatabaseConfig& cfg) {
    if (initialized) {
        Log::warn("DatabaseManager already initialized");
        return;
    }

    config = cfg;

    try {
        pool = std::make_unique<ConnectionPool>(config);
        pool->initialize();

        // Test connection
        if (!testConnection()) {
            throw ConnectionException("Failed to connect to database");
        }

        initialized = true;
        Log::info("DatabaseManager initialized successfully");
        Log::info("Database version: " + getDatabaseVersion());

    } catch (const std::exception& e) {
        Log::error("Failed to initialize DatabaseManager: " + std::string(e.what()));
        pool.reset();
        throw;
    }
}

void DatabaseManager::shutdown() {
    if (!initialized) return;

    if (pool) {
        pool->stop();
        pool.reset();
    }

    initialized = false;
    Log::info("DatabaseManager shut down");
}

std::optional<ConnectionPool::PooledConnection> DatabaseManager::getConnection() {
    if (!initialized || !pool) {
        Log::error("DatabaseManager not initialized");
        return std::nullopt;
    }

    return pool->acquire(config.connection_timeout);
}

bool DatabaseManager::testConnection() {
    try {
        return executeQuery([](Connection& conn) {
            auto result = conn.exec("SELECT 1");
            return result.isOk();
        });
    } catch (const std::exception& e) {
        Log::error("Connection test failed: " + std::string(e.what()));
        return false;
    }
}

std::string DatabaseManager::getDatabaseVersion() {
    try {
        return executeQuery([](Connection& conn) {
            auto result = conn.exec("SELECT version()");
            if (result.isOk() && result.numRows() > 0) {
                return result.getValue(0, 0);
            }
            return std::string("Unknown");
        });
    } catch (const std::exception& e) {
        Log::error("Failed to get database version: " + std::string(e.what()));
        return "Error: " + std::string(e.what());
    }
}

void DatabaseManager::runMigrations() {
    if (!initialized) {
        throw std::runtime_error("DatabaseManager not initialized");
    }

    Log::info("Running database migrations...");

    executeTransaction([](Connection& conn) {
        // Create migrations table if it doesn't exist
        auto result = conn.exec(R"(
            CREATE TABLE IF NOT EXISTS schema_migrations (
                version INTEGER PRIMARY KEY,
                applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                description TEXT
            )
        )");

        if (!result.isOk()) {
            throw QueryException("CREATE TABLE schema_migrations", result.getError());
        }

        return true;
    });

    int current_version = getCurrentSchemaVersion();
    Log::info("Current schema version: " + std::to_string(current_version));

    // TODO: Load and apply migration files from migrations/ directory
    // For now, we'll just ensure the migrations table exists
}

int DatabaseManager::getCurrentSchemaVersion() {
    if (!initialized) {
        return 0;
    }

    try {
        return executeQuery([](Connection& conn) {
            auto result = conn.exec(
                "SELECT COALESCE(MAX(version), 0) FROM schema_migrations"
            );

            if (result.isOk() && result.numRows() > 0) {
                std::string val = result.getValue(0, 0);
                return val.empty() ? 0 : std::stoi(val);
            }
            return 0;
        });
    } catch (const std::exception& e) {
        // Table might not exist yet
        Log::debug("Failed to get schema version: " + std::string(e.what()));
        return 0;
    }
}

bool DatabaseManager::createTables() {
    if (!initialized) return false;

    Log::info("Creating database tables...");

    try {
        return executeTransaction([](Connection& conn) {
            // Create the initial schema based on our integration plan
            std::vector<std::string> table_sqls = {
                // Colors table
                R"(CREATE TABLE IF NOT EXISTS colors (
                    id SERIAL PRIMARY KEY,
                    name VARCHAR(50) UNIQUE NOT NULL,
                    hex_code VARCHAR(7),
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Abilities table
                R"(CREATE TABLE IF NOT EXISTS abilities (
                    id SERIAL PRIMARY KEY,
                    code VARCHAR(50) UNIQUE NOT NULL,
                    name VARCHAR(100) NOT NULL,
                    description TEXT,
                    effect_type VARCHAR(50),
                    effect_data JSONB,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Tags table
                R"(CREATE TABLE IF NOT EXISTS tags (
                    id SERIAL PRIMARY KEY,
                    name VARCHAR(50) UNIQUE NOT NULL,
                    description TEXT,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Monsters table
                R"(CREATE TABLE IF NOT EXISTS monsters (
                    id SERIAL PRIMARY KEY,
                    code VARCHAR(50) UNIQUE NOT NULL,
                    name VARCHAR(100) NOT NULL,
                    glyph VARCHAR(10) NOT NULL,
                    color_id INTEGER REFERENCES colors(id),
                    base_hp INTEGER NOT NULL DEFAULT 10,
                    base_attack INTEGER NOT NULL DEFAULT 1,
                    base_defense INTEGER NOT NULL DEFAULT 0,
                    base_speed INTEGER NOT NULL DEFAULT 100,
                    base_xp INTEGER NOT NULL DEFAULT 10,
                    threat_level VARCHAR(20) DEFAULT 'normal',
                    spawn_depth_min INTEGER DEFAULT 1,
                    spawn_depth_max INTEGER DEFAULT 100,
                    ai_behavior VARCHAR(50) DEFAULT 'wandering',
                    vision_range INTEGER DEFAULT 6,
                    description TEXT,
                    version VARCHAR(20) DEFAULT '0.0.3',
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Items table
                R"(CREATE TABLE IF NOT EXISTS items (
                    id SERIAL PRIMARY KEY,
                    code VARCHAR(50) UNIQUE NOT NULL,
                    name VARCHAR(100) NOT NULL,
                    glyph VARCHAR(10) NOT NULL,
                    color_id INTEGER REFERENCES colors(id),
                    item_type VARCHAR(50) NOT NULL,
                    category VARCHAR(50),
                    rarity VARCHAR(20) DEFAULT 'common',
                    value INTEGER DEFAULT 0,
                    weight DECIMAL(5,2) DEFAULT 0.0,
                    stackable BOOLEAN DEFAULT FALSE,
                    max_stack INTEGER DEFAULT 1,
                    consumable BOOLEAN DEFAULT FALSE,
                    equipment_slot VARCHAR(50),
                    stats_modifiers JSONB,
                    use_effect JSONB,
                    description TEXT,
                    spawn_depth_min INTEGER DEFAULT 1,
                    spawn_depth_max INTEGER DEFAULT 100,
                    version VARCHAR(20) DEFAULT '0.0.3',
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Junction tables
                R"(CREATE TABLE IF NOT EXISTS monster_abilities (
                    monster_id INTEGER REFERENCES monsters(id) ON DELETE CASCADE,
                    ability_id INTEGER REFERENCES abilities(id) ON DELETE RESTRICT,
                    PRIMARY KEY (monster_id, ability_id)
                ))",

                R"(CREATE TABLE IF NOT EXISTS monster_tags (
                    monster_id INTEGER REFERENCES monsters(id) ON DELETE CASCADE,
                    tag_id INTEGER REFERENCES tags(id) ON DELETE RESTRICT,
                    PRIMARY KEY (monster_id, tag_id)
                ))",

                R"(CREATE TABLE IF NOT EXISTS item_abilities (
                    item_id INTEGER REFERENCES items(id) ON DELETE CASCADE,
                    ability_id INTEGER REFERENCES abilities(id) ON DELETE RESTRICT,
                    PRIMARY KEY (item_id, ability_id)
                ))",

                R"(CREATE TABLE IF NOT EXISTS item_tags (
                    item_id INTEGER REFERENCES items(id) ON DELETE CASCADE,
                    tag_id INTEGER REFERENCES tags(id) ON DELETE RESTRICT,
                    PRIMARY KEY (item_id, tag_id)
                ))",

                // Save games table
                R"(CREATE TABLE IF NOT EXISTS save_games (
                    id SERIAL PRIMARY KEY,
                    character_id VARCHAR(100) UNIQUE NOT NULL,
                    player_id VARCHAR(100),
                    character_name VARCHAR(100),
                    level INTEGER DEFAULT 1,
                    experience INTEGER DEFAULT 0,
                    gold INTEGER DEFAULT 0,
                    current_depth INTEGER DEFAULT 1,
                    game_state JSONB NOT NULL,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Leaderboards table
                R"(CREATE TABLE IF NOT EXISTS leaderboards (
                    id SERIAL PRIMARY KEY,
                    player_name VARCHAR(100) NOT NULL,
                    score INTEGER NOT NULL,
                    depth_reached INTEGER NOT NULL,
                    play_time INTEGER NOT NULL,
                    death_reason TEXT,
                    submitted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Telemetry table
                R"(CREATE TABLE IF NOT EXISTS telemetry (
                    id SERIAL PRIMARY KEY,
                    event_type VARCHAR(100) NOT NULL,
                    event_data JSONB,
                    game_version VARCHAR(20) DEFAULT '0.0.3',
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Schema migrations table
                R"(CREATE TABLE IF NOT EXISTS schema_migrations (
                    version INTEGER PRIMARY KEY,
                    applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    description TEXT
                ))",

                // === PHASE 2: AUTHENTICATION TABLES ===

                // Users table for authentication
                R"(CREATE TABLE IF NOT EXISTS users (
                    id SERIAL PRIMARY KEY,
                    username VARCHAR(50) UNIQUE NOT NULL,
                    email VARCHAR(255) UNIQUE NOT NULL,
                    password_hash VARCHAR(255) NOT NULL,
                    salt VARCHAR(255) NOT NULL,
                    email_verified BOOLEAN DEFAULT FALSE,
                    account_locked BOOLEAN DEFAULT FALSE,
                    failed_login_attempts INTEGER DEFAULT 0,
                    last_failed_login TIMESTAMP,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    last_login TIMESTAMP
                ))",

                // User sessions table
                R"(CREATE TABLE IF NOT EXISTS user_sessions (
                    id SERIAL PRIMARY KEY,
                    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                    session_token VARCHAR(255) UNIQUE NOT NULL,
                    refresh_token VARCHAR(255) UNIQUE,
                    expires_at TIMESTAMP NOT NULL,
                    refresh_expires_at TIMESTAMP,
                    ip_address INET,
                    user_agent TEXT,
                    remember_me BOOLEAN DEFAULT FALSE,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    last_used TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    revoked BOOLEAN DEFAULT FALSE,
                    revoked_at TIMESTAMP
                ))",

                // User profiles table
                R"(CREATE TABLE IF NOT EXISTS user_profiles (
                    id SERIAL PRIMARY KEY,
                    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE UNIQUE,
                    display_name VARCHAR(100),
                    avatar_url TEXT,
                    timezone VARCHAR(50) DEFAULT 'UTC',
                    language VARCHAR(10) DEFAULT 'en',
                    theme VARCHAR(20) DEFAULT 'auto',
                    privacy_settings JSONB DEFAULT '{}',
                    game_settings JSONB DEFAULT '{}',
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Password reset tokens table
                R"(CREATE TABLE IF NOT EXISTS password_reset_tokens (
                    id SERIAL PRIMARY KEY,
                    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                    token VARCHAR(255) UNIQUE NOT NULL,
                    expires_at TIMESTAMP NOT NULL,
                    used BOOLEAN DEFAULT FALSE,
                    used_at TIMESTAMP,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // Email verification tokens table
                R"(CREATE TABLE IF NOT EXISTS email_verification_tokens (
                    id SERIAL PRIMARY KEY,
                    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                    token VARCHAR(255) UNIQUE NOT NULL,
                    expires_at TIMESTAMP NOT NULL,
                    used BOOLEAN DEFAULT FALSE,
                    used_at TIMESTAMP,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                ))",

                // User login history table
                R"(CREATE TABLE IF NOT EXISTS user_login_history (
                    id SERIAL PRIMARY KEY,
                    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
                    login_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    ip_address INET,
                    user_agent TEXT,
                    success BOOLEAN NOT NULL,
                    failure_reason VARCHAR(100),
                    session_id INTEGER REFERENCES user_sessions(id) ON DELETE SET NULL
                ))",

                // Update save_games to link with users
                R"(ALTER TABLE save_games
                   ADD COLUMN IF NOT EXISTS user_id INTEGER REFERENCES users(id) ON DELETE CASCADE)"
            };

            for (const auto& sql : table_sqls) {
                auto result = conn.exec(sql);
                if (!result.isOk()) {
                    throw QueryException("CREATE TABLE", result.getError());
                }
            }

            Log::info("Database tables created successfully");
            return true;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to create tables: " + std::string(e.what()));
        return false;
    }
}

bool DatabaseManager::clearAllData() {
    if (!initialized) return false;

    Log::info("Clearing all database data...");

    try {
        return executeTransaction([](Connection& conn) {
            std::vector<std::string> clear_sqls = {
                "DELETE FROM monster_abilities",
                "DELETE FROM monster_tags",
                "DELETE FROM item_abilities",
                "DELETE FROM item_tags",
                "DELETE FROM save_games",
                "DELETE FROM leaderboards",
                "DELETE FROM telemetry",
                "DELETE FROM monsters",
                "DELETE FROM items",
                "DELETE FROM abilities",
                "DELETE FROM tags",
                "DELETE FROM colors"
            };

            for (const auto& sql : clear_sqls) {
                auto result = conn.exec(sql);
                if (!result.isOk()) {
                    throw QueryException("DELETE", result.getError());
                }
            }

            Log::info("Database data cleared successfully");
            return true;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to clear data: " + std::string(e.what()));
        return false;
    }
}

bool DatabaseManager::loadInitialData() {
    if (!initialized) return false;

    Log::info("Loading initial database data...");

    try {
        return executeTransaction([](Connection& conn) {
            // Load basic colors
            auto color_result = conn.exec(R"(
                INSERT INTO colors (name, hex_code) VALUES
                ('white', '#FFFFFF'),
                ('black', '#000000'),
                ('red', '#FF0000'),
                ('green', '#00FF00'),
                ('blue', '#0000FF'),
                ('yellow', '#FFFF00'),
                ('cyan', '#00FFFF'),
                ('magenta', '#FF00FF'),
                ('gray', '#808080'),
                ('brown', '#8B4513')
                ON CONFLICT (name) DO NOTHING
            )");

            if (!color_result.isOk()) {
                throw QueryException("INSERT colors", color_result.getError());
            }

            // Load basic tags
            auto tag_result = conn.exec(R"(
                INSERT INTO tags (name, description) VALUES
                ('undead', 'Undead creatures'),
                ('magical', 'Magical creatures or effects'),
                ('boss', 'Boss-level enemies'),
                ('rare', 'Rare spawns'),
                ('aggressive', 'Naturally hostile'),
                ('peaceful', 'Non-aggressive by default'),
                ('flying', 'Can fly over obstacles'),
                ('aquatic', 'Water-dwelling creatures')
                ON CONFLICT (name) DO NOTHING
            )");

            if (!tag_result.isOk()) {
                throw QueryException("INSERT tags", tag_result.getError());
            }

            // Load basic abilities
            auto ability_result = conn.exec(R"(
                INSERT INTO abilities (code, name, description, effect_type) VALUES
                ('attack_basic', 'Basic Attack', 'Standard melee attack', 'combat'),
                ('regeneration', 'Regeneration', 'Slowly heals over time', 'passive'),
                ('poison_touch', 'Poison Touch', 'Attacks inflict poison', 'combat'),
                ('magic_missile', 'Magic Missile', 'Ranged magic attack', 'spell'),
                ('teleport', 'Teleport', 'Instantly move to nearby location', 'movement'),
                ('heal_minor', 'Minor Heal', 'Restores small amount of health', 'healing'),
                ('speed_boost', 'Speed Boost', 'Temporarily increases movement speed', 'buff')
                ON CONFLICT (code) DO NOTHING
            )");

            if (!ability_result.isOk()) {
                throw QueryException("INSERT abilities", ability_result.getError());
            }

            Log::info("Initial database data loaded successfully");
            return true;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to load initial data: " + std::string(e.what()));
        return false;
    }
}

bool DatabaseManager::isDataLoaded() {
    if (!initialized) return false;

    try {
        return executeQuery([](Connection& conn) {
            auto result = conn.exec("SELECT COUNT(*) FROM colors");
            if (result.isOk() && result.numRows() > 0) {
                int count = std::stoi(result.getValue(0, 0));
                return count > 0;
            }
            return false;
        });
    } catch (const std::exception& e) {
        Log::debug("Failed to check if data loaded: " + std::string(e.what()));
        return false;
    }
}

void DatabaseManager::ensureDataLoaded() {
    if (!initialized) return;

    try {
        // First ensure tables exist
        createTables();

        // Check if data is already loaded
        if (!isDataLoaded()) {
            Log::info("Database is empty, loading initial data...");
            loadInitialData();
        } else {
            Log::debug("Database already contains data");
        }
    } catch (const std::exception& e) {
        Log::error("Failed to ensure data loaded: " + std::string(e.what()));
    }
}

} // namespace db

#endif // ENABLE_DATABASE