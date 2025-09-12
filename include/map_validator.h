#pragma once

#include "point.h"
#include <string>
#include <vector>

class Map;

class MapValidator {
public:
    struct ValidationResult {
        bool valid = true;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        
        // Statistics
        int room_count = 0;
        int walkable_tiles = 0;
        int wall_tiles = 0;
        int void_tiles = 0;
        bool has_stairs_down = false;
        bool has_stairs_up = false;
        bool has_spawn_point = false;
        bool is_connected = false;
        
        // Helper to add error and mark invalid
        void addError(const std::string& error) {
            errors.push_back(error);
            valid = false;
        }
        
        void addWarning(const std::string& warning) {
            warnings.push_back(warning);
        }
    };
    
    // Main validation function
    static ValidationResult validate(const Map& map);
    
    // Individual validation checks
    static bool checkConnectivity(const Map& map);
    static bool hasWalkableTiles(const Map& map);
    static int countWalkableTiles(const Map& map);
    static int countWallTiles(const Map& map);
    static int countRooms(const Map& map);
    static bool hasStairs(const Map& map);
    static Point findWalkableTile(const Map& map);
    
private:
    // Flood fill to check connectivity
    static void floodFill(const Map& map, int x, int y, 
                          std::vector<std::vector<bool>>& visited);
    
    // Check if tile is walkable
    static bool isWalkable(const Map& map, int x, int y);
};