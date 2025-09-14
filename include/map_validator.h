/**
 * @file map_validator.h
 * @brief Map validation and connectivity analysis
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "point.h"
#include <string>
#include <vector>
#include <set>

class Map;

struct ConnectivityResult {
    bool isFullyConnected;
    int numComponents;
    std::vector<std::set<Point>> components;
    std::set<Point> largestComponent;
    std::set<Point> unreachableTiles;
    int totalFloorTiles;
    int reachableFloorTiles;
    
    float connectivityRatio() const {
        if (totalFloorTiles == 0) return 0.0f;
        return static_cast<float>(reachableFloorTiles) / totalFloorTiles;
    }
};

class MapValidator {
public:
    static constexpr int MIN_PLAYABLE_TILES = 50;
    
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
    
    // Enhanced validation with auto-correction
    static bool validateAndFix(Map& map);
    
    // Advanced connectivity checking
    static ConnectivityResult checkAdvancedConnectivity(const Map& map);
    static bool isReachable(const Map& map, const Point& from, const Point& to);
    static std::set<Point> getReachableTiles(const Map& map, const Point& start);
    static std::vector<std::set<Point>> findAllComponents(const Map& map);
    
    // Auto-correction functions
    static void connectComponents(Map& map, const std::vector<std::set<Point>>& components);
    static bool ensureStairsReachable(Map& map);
    
    // Individual validation checks
    static bool checkConnectivity(const Map& map);
    static bool hasWalkableTiles(const Map& map);
    static int countWalkableTiles(const Map& map);
    static int countWallTiles(const Map& map);
    static int countRooms(const Map& map);
    static bool hasStairs(const Map& map);
    static Point findWalkableTile(const Map& map);
    static Point findStairs(const Map& map);
    static Point findFirstFloorTile(const Map& map);
    
private:
    // Flood fill to check connectivity
    static void floodFill(const Map& map, int x, int y, 
                          std::vector<std::vector<bool>>& visited);
    
    // Enhanced BFS flood fill
    static std::set<Point> bfsFloodFill(const Map& map, const Point& start);
    
    // Helper functions
    static bool isWalkable(const Map& map, int x, int y);
    static bool isWalkable(const Map& map, const Point& p);
    static void findClosestPoints(const std::set<Point>& comp1, 
                                   const std::set<Point>& comp2,
                                   Point& p1, Point& p2);
    static int manhattanDistance(const Point& a, const Point& b);
};