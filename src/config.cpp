#include "config.h"
#include "db/database_manager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <boost/json.hpp>

using namespace boost::json;

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

        // Parse JSON
        std::string json_content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
        boost::json::value config = boost::json::parse(json_content);
        boost::json::object const& root = config.as_object();

        // Game settings
        if (root.contains("game")) {
            auto const& game = root.at("game").as_object();
            if (game.contains("default_map")) {
                std::string map_str = game.at("default_map").as_string().c_str();
                default_map_type = parseMapType(map_str);
            }
            if (game.contains("debug_mode")) {
                debug_mode = game.at("debug_mode").as_bool();
            }
        }

        // Display settings
        if (root.contains("display")) {
            auto const& display = root.at("display").as_object();
            if (display.contains("theme")) {
                theme = display.at("theme").as_string().c_str();
            }
            if (display.contains("show_fps")) {
                show_fps = display.at("show_fps").as_bool();
            }
            if (display.contains("message_log")) {
                auto const& msg_log = display.at("message_log").as_object();
                if (msg_log.contains("max_messages")) {
                    max_messages = static_cast<int>(msg_log.at("max_messages").as_int64());
                }
                if (msg_log.contains("visible_messages")) {
                    visible_messages = static_cast<int>(msg_log.at("visible_messages").as_int64());
                }
            }
        }

        // Map generation settings
        if (root.contains("map_generation")) {
            auto const& map_gen = root.at("map_generation").as_object();
            if (map_gen.contains("procedural")) {
                auto const& proc = map_gen.at("procedural").as_object();
                if (proc.contains("width")) map_width = static_cast<int>(proc.at("width").as_int64());
                if (proc.contains("height")) map_height = static_cast<int>(proc.at("height").as_int64());
                if (proc.contains("min_rooms")) min_rooms = static_cast<int>(proc.at("min_rooms").as_int64());
                if (proc.contains("max_rooms")) max_rooms = static_cast<int>(proc.at("max_rooms").as_int64());
                if (proc.contains("min_room_size")) min_room_size = static_cast<int>(proc.at("min_room_size").as_int64());
                if (proc.contains("max_room_size")) max_room_size = static_cast<int>(proc.at("max_room_size").as_int64());
                if (proc.contains("lit_room_chance")) lit_room_chance = static_cast<float>(proc.at("lit_room_chance").as_double());
                if (proc.contains("door_chance")) door_chance = static_cast<float>(proc.at("door_chance").as_double());
                if (proc.contains("corridor_style")) corridor_style = proc.at("corridor_style").as_string().c_str();
            }
        }

        // Monster settings
        if (root.contains("monsters")) {
            auto const& monsters = root.at("monsters").as_object();
            if (monsters.contains("initial_monster_count")) {
                initial_monster_count = static_cast<int>(monsters.at("initial_monster_count").as_int64());
            }
            if (monsters.contains("max_per_level")) {
                max_monsters_per_level = static_cast<int>(monsters.at("max_per_level").as_int64());
            }
            if (monsters.contains("spawn_rate")) {
                monster_spawn_rate = static_cast<int>(monsters.at("spawn_rate").as_int64());
            }
            if (monsters.contains("spawn_outside_fov")) {
                spawn_outside_fov = monsters.at("spawn_outside_fov").as_bool();
            }
            if (monsters.contains("min_spawn_distance")) {
                min_spawn_distance = static_cast<int>(monsters.at("min_spawn_distance").as_int64());
            }
            if (monsters.contains("room_spawn_percentage")) {
                room_spawn_percentage = static_cast<float>(monsters.at("room_spawn_percentage").as_double());
            }
            if (monsters.contains("behavior")) {
                auto const& behavior = monsters.at("behavior").as_object();
                if (behavior.contains("aggression_radius")) {
                    aggression_radius = static_cast<int>(behavior.at("aggression_radius").as_int64());
                }
            }
        }

        // Player settings
        if (root.contains("player")) {
            auto const& player = root.at("player").as_object();
            if (player.contains("starting_hp")) {
                player_starting_hp = static_cast<int>(player.at("starting_hp").as_int64());
            }
            if (player.contains("starting_attack")) {
                player_starting_attack = static_cast<int>(player.at("starting_attack").as_int64());
            }
            if (player.contains("starting_defense")) {
                player_starting_defense = static_cast<int>(player.at("starting_defense").as_int64());
            }
            if (player.contains("inventory_capacity")) {
                inventory_capacity = static_cast<int>(player.at("inventory_capacity").as_int64());
            }
            if (player.contains("fov_radius")) {
                fov_radius = static_cast<int>(player.at("fov_radius").as_int64());
            }
        }

        // Path settings
        if (root.contains("paths")) {
            auto const& paths = root.at("paths").as_object();
            if (paths.contains("data_dir")) {
                data_dir = paths.at("data_dir").as_string().c_str();
            }
            if (paths.contains("save_dir")) {
                save_dir = paths.at("save_dir").as_string().c_str();
            }
            if (paths.contains("log_dir")) {
                log_dir = paths.at("log_dir").as_string().c_str();
            }
        }

        // Performance settings
        if (root.contains("performance")) {
            auto const& perf = root.at("performance").as_object();
            if (perf.contains("target_fps")) {
                target_fps = static_cast<int>(perf.at("target_fps").as_int64());
            }
        }

        // Database settings
        if (root.contains("database")) {
            auto const& db = root.at("database").as_object();
            if (db.contains("enabled")) {
                database_enabled = db.at("enabled").as_bool();
            }
            if (db.contains("connection")) {
                auto const& conn = db.at("connection").as_object();
                if (conn.contains("host")) db_host = conn.at("host").as_string().c_str();
                if (conn.contains("port")) db_port = static_cast<int>(conn.at("port").as_int64());
                if (conn.contains("database")) db_name = conn.at("database").as_string().c_str();
                if (conn.contains("username")) db_username = conn.at("username").as_string().c_str();
                if (conn.contains("password")) db_password = conn.at("password").as_string().c_str();
            }
            if (db.contains("pool")) {
                auto const& pool = db.at("pool").as_object();
                if (pool.contains("min_connections")) db_min_connections = static_cast<int>(pool.at("min_connections").as_int64());
                if (pool.contains("max_connections")) db_max_connections = static_cast<int>(pool.at("max_connections").as_int64());
                if (pool.contains("connection_timeout")) db_connection_timeout = static_cast<int>(pool.at("connection_timeout").as_int64());
            }
        }

        // Development settings
        if (root.contains("development")) {
            auto const& dev = root.at("development").as_object();
            if (dev.contains("verbose_logging")) {
                verbose_logging = dev.at("verbose_logging").as_bool();
            }
            if (dev.contains("autosave_interval")) {
                autosave_interval = static_cast<int>(dev.at("autosave_interval").as_int64());
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

bool Config::saveToFile(const std::string& filename) const {
    try {
        boost::json::object config;

        // Game settings
        boost::json::object game;
        game["default_map"] = mapTypeToString(default_map_type);
        game["debug_mode"] = debug_mode;
        config["game"] = game;

        // Display settings
        boost::json::object display;
        display["theme"] = theme;
        display["show_fps"] = show_fps;

        boost::json::object message_log;
        message_log["max_messages"] = max_messages;
        message_log["visible_messages"] = visible_messages;
        display["message_log"] = message_log;
        config["display"] = display;

        // Map generation
        boost::json::object map_generation;
        boost::json::object procedural;
        procedural["width"] = map_width;
        procedural["height"] = map_height;
        procedural["min_rooms"] = min_rooms;
        procedural["max_rooms"] = max_rooms;
        procedural["min_room_size"] = min_room_size;
        procedural["max_room_size"] = max_room_size;
        procedural["lit_room_chance"] = lit_room_chance;
        procedural["door_chance"] = door_chance;
        procedural["corridor_style"] = corridor_style;
        map_generation["procedural"] = procedural;
        config["map_generation"] = map_generation;

        // Monster settings
        boost::json::object monsters;
        monsters["initial_monster_count"] = initial_monster_count;
        monsters["max_per_level"] = max_monsters_per_level;
        monsters["spawn_rate"] = monster_spawn_rate;
        monsters["spawn_outside_fov"] = spawn_outside_fov;
        monsters["min_spawn_distance"] = min_spawn_distance;
        monsters["room_spawn_percentage"] = room_spawn_percentage;

        boost::json::object behavior;
        behavior["aggression_radius"] = aggression_radius;
        monsters["behavior"] = behavior;
        config["monsters"] = monsters;

        // Player settings
        boost::json::object player;
        player["starting_hp"] = player_starting_hp;
        player["starting_attack"] = player_starting_attack;
        player["starting_defense"] = player_starting_defense;
        player["inventory_capacity"] = inventory_capacity;
        player["fov_radius"] = fov_radius;
        config["player"] = player;

        // Paths
        boost::json::object paths;
        paths["data_dir"] = data_dir;
        paths["save_dir"] = save_dir;
        paths["log_dir"] = log_dir;
        config["paths"] = paths;

        // Performance
        boost::json::object performance;
        performance["target_fps"] = target_fps;
        config["performance"] = performance;

        // Database
        boost::json::object database;
        database["enabled"] = database_enabled;

        boost::json::object connection;
        connection["host"] = db_host;
        connection["port"] = db_port;
        connection["database"] = db_name;
        connection["username"] = db_username;
        connection["password"] = db_password;
        database["connection"] = connection;

        boost::json::object pool;
        pool["min_connections"] = db_min_connections;
        pool["max_connections"] = db_max_connections;
        pool["connection_timeout"] = db_connection_timeout;
        database["pool"] = pool;
        config["database"] = database;

        // Development
        boost::json::object development;
        development["verbose_logging"] = verbose_logging;
        development["autosave_interval"] = autosave_interval;
        config["development"] = development;

        // Write to file
        std::ofstream file(filename);
        if (!file) {
            return false;
        }
        file << boost::json::serialize(config);
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error saving config file: " << e.what() << std::endl;
        return false;
    }
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
    // Database environment variables (these override config.json values)

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