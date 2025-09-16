#include "config.h"
#include "db/database_manager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

bool Config::loadFromFile(const std::string& filename) {
    try {
        if (!std::filesystem::exists(filename)) {
            // Config file doesn't exist, use defaults
            return true;
        }
        
        // Read file contents
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Failed to open config file: " << filename << std::endl;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string yaml_content = buffer.str();
        
        // Parse YAML using rapidyaml
        ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_content));
        ryml::ConstNodeRef root = tree.rootref();
        
        // Game settings
        if (root.has_child("game")) {
            auto game = root["game"];
            if (game.has_child("default_map")) {
                std::string map_str;
                game["default_map"] >> map_str;
                default_map_type = parseMapType(map_str);
            }
            if (game.has_child("debug_mode")) {
                game["debug_mode"] >> debug_mode;
            }
        }
        
        // Display settings
        if (root.has_child("display")) {
            auto display = root["display"];
            if (display.has_child("theme")) {
                display["theme"] >> theme;
            }
            if (display.has_child("show_fps")) {
                display["show_fps"] >> show_fps;
            }
            if (display.has_child("message_log")) {
                auto msg_log = display["message_log"];
                if (msg_log.has_child("max_messages")) {
                    msg_log["max_messages"] >> max_messages;
                }
                if (msg_log.has_child("visible_messages")) {
                    msg_log["visible_messages"] >> visible_messages;
                }
            }
        }
        
        // Map generation settings
        if (root.has_child("map_generation")) {
            auto map_gen = root["map_generation"];
            if (map_gen.has_child("procedural")) {
                auto proc = map_gen["procedural"];
                if (proc.has_child("width")) proc["width"] >> map_width;
                if (proc.has_child("height")) proc["height"] >> map_height;
                if (proc.has_child("min_rooms")) proc["min_rooms"] >> min_rooms;
                if (proc.has_child("max_rooms")) proc["max_rooms"] >> max_rooms;
                if (proc.has_child("min_room_size")) proc["min_room_size"] >> min_room_size;
                if (proc.has_child("max_room_size")) proc["max_room_size"] >> max_room_size;
                if (proc.has_child("lit_room_chance")) proc["lit_room_chance"] >> lit_room_chance;
                if (proc.has_child("door_chance")) proc["door_chance"] >> door_chance;
                if (proc.has_child("corridor_style")) proc["corridor_style"] >> corridor_style;
            }
        }
        
        // Monster settings
        if (root.has_child("monsters")) {
            auto monsters = root["monsters"];
            if (monsters.has_child("initial_monster_count")) {
                monsters["initial_monster_count"] >> initial_monster_count;
            }
            if (monsters.has_child("max_per_level")) {
                monsters["max_per_level"] >> max_monsters_per_level;
            }
            if (monsters.has_child("spawn_rate")) {
                monsters["spawn_rate"] >> monster_spawn_rate;
            }
            if (monsters.has_child("spawn_outside_fov")) {
                monsters["spawn_outside_fov"] >> spawn_outside_fov;
            }
            if (monsters.has_child("min_spawn_distance")) {
                monsters["min_spawn_distance"] >> min_spawn_distance;
            }
            if (monsters.has_child("room_spawn_percentage")) {
                monsters["room_spawn_percentage"] >> room_spawn_percentage;
            }
            if (monsters.has_child("behavior")) {
                auto behavior = monsters["behavior"];
                if (behavior.has_child("aggression_radius")) {
                    behavior["aggression_radius"] >> aggression_radius;
                }
            }
        }
        
        // Player settings
        if (root.has_child("player")) {
            auto player = root["player"];
            if (player.has_child("starting_hp")) {
                player["starting_hp"] >> player_starting_hp;
            }
            if (player.has_child("starting_attack")) {
                player["starting_attack"] >> player_starting_attack;
            }
            if (player.has_child("starting_defense")) {
                player["starting_defense"] >> player_starting_defense;
            }
            if (player.has_child("inventory_capacity")) {
                player["inventory_capacity"] >> inventory_capacity;
            }
            if (player.has_child("fov_radius")) {
                player["fov_radius"] >> fov_radius;
            }
        }
        
        // Path settings
        if (root.has_child("paths")) {
            auto paths = root["paths"];
            if (paths.has_child("data_dir")) {
                paths["data_dir"] >> data_dir;
            }
            if (paths.has_child("save_dir")) {
                paths["save_dir"] >> save_dir;
            }
            if (paths.has_child("log_dir")) {
                paths["log_dir"] >> log_dir;
            }
        }
        
        // Performance settings
        if (root.has_child("performance")) {
            auto perf = root["performance"];
            if (perf.has_child("target_fps")) {
                perf["target_fps"] >> target_fps;
            }
        }
        
        // Database settings
        if (root.has_child("database")) {
            auto db = root["database"];
            if (db.has_child("enabled")) {
                db["enabled"] >> database_enabled;
            }
            if (db.has_child("connection")) {
                auto conn = db["connection"];
                if (conn.has_child("host")) conn["host"] >> db_host;
                if (conn.has_child("port")) conn["port"] >> db_port;
                if (conn.has_child("database")) conn["database"] >> db_name;
                if (conn.has_child("username")) conn["username"] >> db_username;
                if (conn.has_child("password")) conn["password"] >> db_password;
            }
            if (db.has_child("pool")) {
                auto pool = db["pool"];
                if (pool.has_child("min_connections")) pool["min_connections"] >> db_min_connections;
                if (pool.has_child("max_connections")) pool["max_connections"] >> db_max_connections;
                if (pool.has_child("connection_timeout")) pool["connection_timeout"] >> db_connection_timeout;
            }
        }

        // Development settings
        if (root.has_child("development")) {
            auto dev = root["development"];
            if (dev.has_child("verbose_logging")) {
                dev["verbose_logging"] >> verbose_logging;
            }
            if (dev.has_child("autosave_interval")) {
                dev["autosave_interval"] >> autosave_interval;
            }
        }

        // Load environment variables after config file (env overrides config)
        loadEnvironmentVariables();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
        return false;
    }
}

bool Config::saveToFile([[maybe_unused]] const std::string& filename) const {
    // Mini-yaml doesn't have a built-in serializer/emitter
    // For now, we'll just return true since config is typically read-only
    // In the future, we could manually write YAML format
    return true;
}

std::string Config::getDataFilePath(const std::string& filename) const {
    std::filesystem::path data_path(data_dir);
    return (data_path / filename).string();
}

bool Config::isDataDirValid() const {
    return std::filesystem::exists(data_dir) && 
           std::filesystem::is_directory(data_dir);
}

MapType Config::parseMapType(const std::string& str) const {
    if (str == "procedural") return MapType::PROCEDURAL;
    if (str == "room") return MapType::TEST_ROOM;
    if (str == "dungeon") return MapType::TEST_DUNGEON;
    if (str == "corridor") return MapType::CORRIDOR_TEST;
    if (str == "arena") return MapType::COMBAT_ARENA;
    if (str == "stress") return MapType::STRESS_TEST;
    return MapType::PROCEDURAL;  // Default
}

std::string Config::mapTypeToString(MapType type) const {
    switch (type) {
        case MapType::PROCEDURAL: return "procedural";
        case MapType::TEST_ROOM: return "room";
        case MapType::TEST_DUNGEON: return "dungeon";
        case MapType::CORRIDOR_TEST: return "corridor";
        case MapType::COMBAT_ARENA: return "arena";
        case MapType::STRESS_TEST: return "stress";
        default: return "procedural";
    }
}

std::string Config::getEnvironmentVariable(const std::string& name, const std::string& default_value) const {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : default_value;
}

void Config::loadEnvironmentVariables() {
    // Database environment variables (these override config.yml values)

    // Database connection settings
    std::string env_host = getEnvironmentVariable("DB_HOST");
    if (!env_host.empty()) {
        db_host = env_host;
    }

    std::string env_port = getEnvironmentVariable("DB_PORT");
    if (!env_port.empty()) {
        try {
            db_port = std::stoi(env_port);
        } catch (const std::exception&) {
            std::cerr << "Warning: Invalid DB_PORT environment variable, using config value" << std::endl;
        }
    }

    std::string env_name = getEnvironmentVariable("DB_NAME");
    if (!env_name.empty()) {
        db_name = env_name;
    }

    std::string env_user = getEnvironmentVariable("DB_USER");
    if (!env_user.empty()) {
        db_username = env_user;
    }

    // Database password - prioritize specific env vars
    std::string env_password = getEnvironmentVariable("DB_PASS");
    if (env_password.empty()) {
        env_password = getEnvironmentVariable("VEYRM_DB_PASSWORD");
    }
    if (env_password.empty()) {
        env_password = getEnvironmentVariable("POSTGRES_PASSWORD");
    }
    if (!env_password.empty()) {
        db_password = env_password;
    }

    // Alternative PostgreSQL environment variables (for compatibility)
    std::string pg_host = getEnvironmentVariable("PGHOST");
    if (!pg_host.empty()) {
        db_host = pg_host;
    }

    std::string pg_port = getEnvironmentVariable("PGPORT");
    if (!pg_port.empty()) {
        try {
            db_port = std::stoi(pg_port);
        } catch (const std::exception&) {
            std::cerr << "Warning: Invalid PGPORT environment variable, using config value" << std::endl;
        }
    }

    std::string pg_database = getEnvironmentVariable("PGDATABASE");
    if (!pg_database.empty()) {
        db_name = pg_database;
    }

    std::string pg_user = getEnvironmentVariable("PGUSER");
    if (!pg_user.empty()) {
        db_username = pg_user;
    }

    std::string pg_password = getEnvironmentVariable("PGPASSWORD");
    if (!pg_password.empty()) {
        db_password = pg_password;
    }
}

db::DatabaseConfig Config::getDatabaseConfig() const {
    db::DatabaseConfig config;
    config.host = db_host;
    config.port = db_port;
    config.database = db_name;
    config.username = db_username;
    config.password = db_password;
    config.min_connections = db_min_connections;
    config.max_connections = db_max_connections;
    config.connection_timeout = std::chrono::milliseconds(db_connection_timeout);
    return config;
}