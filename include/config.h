/**
 * @file config.h
 * @brief Game configuration management system
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <filesystem>
#include <ryml.hpp>
#include <ryml_std.hpp>  // For std::string support
#include <c4/format.hpp>
#include "map_generator.h"  // For MapType enum

// Forward declarations
namespace db {
    struct DatabaseConfig;
}

/**
 * @class Config
 * @brief Singleton configuration manager for game settings
 *
 * The Config class manages all game configuration options, including
 * display settings, gameplay parameters, map generation options,
 * and file paths. It provides a YAML-based configuration system
 * with default values and validation.
 *
 * Settings categories:
 * - Game Settings: Debug mode, default map type
 * - Display Settings: Theme, FPS display, message limits
 * - Map Generation: Dimensions, room parameters, corridor style
 * - Monster Settings: Spawn rates, AI parameters
 * - Player Settings: Starting stats, inventory capacity
 * - Paths: Data, save, and log directories
 * - Performance: Target FPS, autosave intervals
 *
 * @see MapGenerator
 * @see ColorScheme
 */
class Config {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the Config singleton
     */
    static Config& getInstance();

    /**
     * @brief Load configuration from YAML file
     * @param filename Path to configuration file (default: "config.yml")
     * @return true if loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& filename = "config.yml");

    /**
     * @brief Save current configuration to YAML file
     * @param filename Path to save file (default: "config.yml")
     * @return true if saved successfully, false otherwise
     */
    bool saveToFile(const std::string& filename = "config.yml") const;

    // === Game Settings ===

    /**
     * @brief Get default map generation type
     * @return Default map type for new games
     */
    MapType getDefaultMapType() const { return default_map_type; }

    /**
     * @brief Set default map generation type
     * @param type Map type to use as default
     */
    void setDefaultMapType(MapType type) { default_map_type = type; }

    /**
     * @brief Check if debug mode is enabled
     * @return true if debug mode is active
     */
    bool isDebugMode() const { return debug_mode; }

    /**
     * @brief Enable or disable debug mode
     * @param enabled Debug mode state
     */
    void setDebugMode(bool enabled) { debug_mode = enabled; }
    
    // === Display Settings ===

    /** @brief Get current UI theme name @return Theme name */
    std::string getTheme() const { return theme; }

    /** @brief Set UI theme @param t Theme name */
    void setTheme(const std::string& t) { theme = t; }

    /** @brief Check if FPS display is enabled @return true if showing FPS */
    bool getShowFPS() const { return show_fps; }

    /** @brief Enable/disable FPS display @param show FPS display state */
    void setShowFPS(bool show) { show_fps = show; }

    /** @brief Get maximum messages to keep in log @return Max message count */
    int getMaxMessages() const { return max_messages; }

    /** @brief Get number of visible messages in UI @return Visible message count */
    int getVisibleMessages() const { return visible_messages; }
    
    // === Map Generation ===

    /** @brief Get map width @return Map width in tiles */
    int getMapWidth() const { return map_width; }

    /** @brief Get map height @return Map height in tiles */
    int getMapHeight() const { return map_height; }

    /** @brief Get minimum room count @return Minimum rooms per level */
    int getMinRooms() const { return min_rooms; }

    /** @brief Get maximum room count @return Maximum rooms per level */
    int getMaxRooms() const { return max_rooms; }

    /** @brief Get minimum room size @return Minimum room dimension */
    int getMinRoomSize() const { return min_room_size; }

    /** @brief Get maximum room size @return Maximum room dimension */
    int getMaxRoomSize() const { return max_room_size; }

    /** @brief Get chance for rooms to be lit @return Probability (0.0-1.0) */
    float getLitRoomChance() const { return lit_room_chance; }

    /** @brief Get door generation chance @return Probability (0.0-1.0) */
    float getDoorChance() const { return door_chance; }

    /** @brief Get corridor generation style @return Corridor style name */
    std::string getCorridorStyle() const { return corridor_style; }

    /**
     * @brief Set map dimensions
     * @param width New map width
     * @param height New map height
     */
    void setMapDimensions(int width, int height) {
        map_width = width;
        map_height = height;
    }
    
    // === Monster Settings ===

    /** @brief Get initial monster count @return Starting monsters per level */
    int getInitialMonsterCount() const { return initial_monster_count; }

    /** @brief Get maximum monsters per level @return Monster cap */
    int getMaxMonstersPerLevel() const { return max_monsters_per_level; }

    /** @brief Get monster spawn rate @return Spawn frequency */
    int getMonsterSpawnRate() const { return monster_spawn_rate; }

    /** @brief Get aggression detection radius @return Tiles */
    int getAggressionRadius() const { return aggression_radius; }

    /** @brief Check if monsters spawn outside FOV @return Spawn outside vision */
    bool getSpawnOutsideFOV() const { return spawn_outside_fov; }

    /** @brief Get minimum spawn distance from player @return Distance in tiles */
    int getMinSpawnDistance() const { return min_spawn_distance; }

    /** @brief Get percentage of monsters spawned in rooms @return Percentage (0.0-1.0) */
    float getRoomSpawnPercentage() const { return room_spawn_percentage; }
    
    // === Player Settings ===

    /** @brief Get player starting hit points @return Starting HP */
    int getPlayerStartingHP() const { return player_starting_hp; }

    /** @brief Get player starting attack value @return Attack stat */
    int getPlayerStartingAttack() const { return player_starting_attack; }

    /** @brief Get player starting defense value @return Defense stat */
    int getPlayerStartingDefense() const { return player_starting_defense; }

    /** @brief Get inventory slot capacity @return Max inventory items */
    int getInventoryCapacity() const { return inventory_capacity; }

    /** @brief Get field of view radius @return FOV distance in tiles */
    int getFOVRadius() const { return fov_radius; }
    
    // === Path Settings ===

    /** @brief Get data directory path @return Data directory */
    std::string getDataDir() const { return data_dir; }

    /** @brief Set data directory path @param path New data directory */
    void setDataDir(const std::string& path) { data_dir = path; }

    /** @brief Get save directory path @return Save directory */
    std::string getSaveDir() const { return save_dir; }

    /** @brief Get log directory path @return Log directory */
    std::string getLogDir() const { return log_dir; }

    /**
     * @brief Get full path to a data file
     * @param filename Data file name
     * @return Complete file path
     */
    std::string getDataFilePath(const std::string& filename) const;

    /**
     * @brief Check if data directory exists and is accessible
     * @return true if data directory is valid
     */
    bool isDataDirValid() const;
    
    // === Database Settings ===

    /** @brief Check if database features are enabled @return Database enabled state */
    bool isDatabaseEnabled() const { return database_enabled; }

    /** @brief Get database host @return Database hostname */
    std::string getDatabaseHost() const { return db_host; }

    /** @brief Get database port @return Database port number */
    int getDatabasePort() const { return db_port; }

    /** @brief Get database name @return Database name */
    std::string getDatabaseName() const { return db_name; }

    /** @brief Get database username @return Database username */
    std::string getDatabaseUsername() const { return db_username; }

    /** @brief Get database password @return Database password */
    std::string getDatabasePassword() const { return db_password; }

    /** @brief Get minimum database connections @return Min connections */
    int getDatabaseMinConnections() const { return db_min_connections; }

    /** @brief Get maximum database connections @return Max connections */
    int getDatabaseMaxConnections() const { return db_max_connections; }

    /** @brief Get database connection timeout @return Timeout in milliseconds */
    int getDatabaseConnectionTimeout() const { return db_connection_timeout; }

    /**
     * @brief Create DatabaseConfig struct from current settings
     * @return DatabaseConfig object with current database settings
     */
    db::DatabaseConfig getDatabaseConfig() const;

    // === Performance Settings ===

    /** @brief Get target frames per second @return Target FPS */
    int getTargetFPS() const { return target_fps; }

    // === Development Settings ===

    /** @brief Check if verbose logging is enabled @return Verbose logging state */
    bool getVerboseLogging() const { return verbose_logging; }

    /** @brief Get autosave interval @return Seconds between autosaves */
    int getAutosaveInterval() const { return autosave_interval; }
    
private:
    // Game settings
    MapType default_map_type = MapType::PROCEDURAL; ///< Default map generation type
    bool debug_mode = false;                        ///< Debug mode enabled

    // Display settings
    std::string theme = "auto";      ///< UI theme name
    bool show_fps = false;           ///< Show FPS counter
    int max_messages = 100;          ///< Maximum messages in log
    int visible_messages = 5;        ///< Visible messages in UI

    // Map generation settings
    int map_width = 198;             ///< Map width (Angband standard)
    int map_height = 66;             ///< Map height (Angband standard)
    int min_rooms = 9;               ///< Minimum rooms per level
    int max_rooms = 20;              ///< Maximum rooms per level
    int min_room_size = 4;           ///< Minimum room dimension
    int max_room_size = 20;          ///< Maximum room dimension
    float lit_room_chance = 0.3f;    ///< Probability of lit rooms
    float door_chance = 0.15f;       ///< Probability of doors
    std::string corridor_style = "straight"; ///< Corridor generation style

    // Monster settings
    int initial_monster_count = 10;     ///< Starting monsters per level
    int max_monsters_per_level = 30;    ///< Maximum monsters per level
    int monster_spawn_rate = 100;       ///< Monster spawn frequency
    int aggression_radius = 10;         ///< Monster aggression detection radius
    bool spawn_outside_fov = true;      ///< Spawn monsters outside player FOV
    int min_spawn_distance = 5;         ///< Minimum spawn distance from player
    float room_spawn_percentage = 0.95f; ///< Percentage of monsters spawned in rooms

    // Player settings
    int player_starting_hp = 50;        ///< Player starting hit points
    int player_starting_attack = 8;     ///< Player starting attack
    int player_starting_defense = 5;    ///< Player starting defense
    int inventory_capacity = 26;        ///< Player inventory slots
    int fov_radius = 10;                ///< Player field of view radius

    // Paths
    std::string data_dir = "data";      ///< Data files directory
    std::string save_dir = "saves";     ///< Save games directory
    std::string log_dir = "logs";       ///< Log files directory

    // Performance
    int target_fps = 60;                ///< Target frames per second

    // Database settings
    bool database_enabled = false;      ///< Database features enabled
    std::string db_host = "localhost";  ///< Database host
    int db_port = 5432;                 ///< Database port
    std::string db_name = "veyrm_db";   ///< Database name
    std::string db_username = "veyrm_admin"; ///< Database username
    std::string db_password = "";       ///< Database password (loaded from env)
    int db_min_connections = 2;         ///< Minimum database connections
    int db_max_connections = 10;        ///< Maximum database connections
    int db_connection_timeout = 5000;   ///< Connection timeout in milliseconds

    // Development
    bool verbose_logging = false;       ///< Enable verbose logging
    int autosave_interval = 300;        ///< Autosave interval in seconds

    /**
     * @brief Parse MapType from string
     * @param str String representation of MapType
     * @return Parsed MapType
     */
    MapType parseMapType(const std::string& str) const;

    /**
     * @brief Convert MapType to string
     * @param type MapType to convert
     * @return String representation
     */
    std::string mapTypeToString(MapType type) const;

    /**
     * @brief Get environment variable with optional default
     * @param name Environment variable name
     * @param default_value Default value if not set
     * @return Environment variable value or default
     */
    std::string getEnvironmentVariable(const std::string& name, const std::string& default_value = "") const;

    /**
     * @brief Load environment variables into configuration
     * This loads database credentials and other sensitive settings from environment
     */
    void loadEnvironmentVariables();

    // Singleton pattern
    Config() = default;                  ///< Private constructor
    Config(const Config&) = delete;      ///< No copy constructor
    Config& operator=(const Config&) = delete; ///< No assignment operator
};