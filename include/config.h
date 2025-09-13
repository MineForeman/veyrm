#pragma once

#include <string>
#include <filesystem>
#include <ryml.hpp>
#include <ryml_std.hpp>  // For std::string support
#include <c4/format.hpp>
#include "map_generator.h"  // For MapType enum

class Config {
public:
    static Config& getInstance();
    
    // Load configuration from file
    bool loadFromFile(const std::string& filename = "config.yml");
    
    // Save current configuration to file
    bool saveToFile(const std::string& filename = "config.yml") const;
    
    // === Game Settings ===
    MapType getDefaultMapType() const { return default_map_type; }
    void setDefaultMapType(MapType type) { default_map_type = type; }
    
    bool isDebugMode() const { return debug_mode; }
    void setDebugMode(bool enabled) { debug_mode = enabled; }
    
    // === Display Settings ===
    std::string getTheme() const { return theme; }
    void setTheme(const std::string& t) { theme = t; }
    
    bool getShowFPS() const { return show_fps; }
    void setShowFPS(bool show) { show_fps = show; }
    
    int getMaxMessages() const { return max_messages; }
    int getVisibleMessages() const { return visible_messages; }
    
    // === Map Generation ===
    int getMapWidth() const { return map_width; }
    int getMapHeight() const { return map_height; }
    int getMinRooms() const { return min_rooms; }
    int getMaxRooms() const { return max_rooms; }
    int getMinRoomSize() const { return min_room_size; }
    int getMaxRoomSize() const { return max_room_size; }
    float getLitRoomChance() const { return lit_room_chance; }
    float getDoorChance() const { return door_chance; }
    std::string getCorridorStyle() const { return corridor_style; }
    
    void setMapDimensions(int width, int height) { 
        map_width = width; 
        map_height = height; 
    }
    
    // === Monster Settings ===
    int getInitialMonsterCount() const { return initial_monster_count; }
    int getMaxMonstersPerLevel() const { return max_monsters_per_level; }
    int getMonsterSpawnRate() const { return monster_spawn_rate; }
    int getAggressionRadius() const { return aggression_radius; }
    bool getSpawnOutsideFOV() const { return spawn_outside_fov; }
    int getMinSpawnDistance() const { return min_spawn_distance; }
    float getRoomSpawnPercentage() const { return room_spawn_percentage; }
    
    // === Player Settings ===
    int getPlayerStartingHP() const { return player_starting_hp; }
    int getPlayerStartingAttack() const { return player_starting_attack; }
    int getPlayerStartingDefense() const { return player_starting_defense; }
    int getInventoryCapacity() const { return inventory_capacity; }
    int getFOVRadius() const { return fov_radius; }
    
    // === Path Settings ===
    std::string getDataDir() const { return data_dir; }
    void setDataDir(const std::string& path) { data_dir = path; }
    std::string getSaveDir() const { return save_dir; }
    std::string getLogDir() const { return log_dir; }
    
    // Get full path to a data file
    std::string getDataFilePath(const std::string& filename) const;
    
    // Check if data directory exists
    bool isDataDirValid() const;
    
    // === Performance Settings ===
    int getTargetFPS() const { return target_fps; }
    
    // === Development Settings ===
    bool getVerboseLogging() const { return verbose_logging; }
    int getAutosaveInterval() const { return autosave_interval; }
    
private:
    // Game settings
    MapType default_map_type = MapType::PROCEDURAL;
    bool debug_mode = false;
    
    // Display settings
    std::string theme = "auto";
    bool show_fps = false;
    int max_messages = 100;
    int visible_messages = 5;
    
    // Map generation settings
    int map_width = 198;  // Angband standard
    int map_height = 66;  // Angband standard
    int min_rooms = 9;
    int max_rooms = 20;
    int min_room_size = 4;
    int max_room_size = 20;
    float lit_room_chance = 0.3f;
    float door_chance = 0.15f;
    std::string corridor_style = "straight";
    
    // Monster settings
    int initial_monster_count = 10;
    int max_monsters_per_level = 30;
    int monster_spawn_rate = 100;
    int aggression_radius = 10;
    bool spawn_outside_fov = true;
    int min_spawn_distance = 5;
    float room_spawn_percentage = 0.95f;
    
    // Player settings
    int player_starting_hp = 50;
    int player_starting_attack = 8;
    int player_starting_defense = 5;
    int inventory_capacity = 26;
    int fov_radius = 10;
    
    // Paths
    std::string data_dir = "data";
    std::string save_dir = "saves";
    std::string log_dir = "logs";
    
    // Performance
    int target_fps = 60;
    
    // Development
    bool verbose_logging = false;
    int autosave_interval = 300;
    
    // Helper to parse MapType from string
    MapType parseMapType(const std::string& str) const;
    std::string mapTypeToString(MapType type) const;
    
    // Singleton
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
};