#pragma once

#include "point.h"
#include <string>
#include <vector>

class Map;

enum class MapType {
    TEST_ROOM,      // Single 20x20 room
    TEST_DUNGEON,   // Multi-room layout (current 5-room design)
    CORRIDOR_TEST,  // Long corridors for testing
    COMBAT_ARENA,   // Open space for combat
    STRESS_TEST     // Large map for performance testing
};

struct RoomDef {
    int x, y;
    int width, height;
    std::string type;  // "normal", "treasure", "boss"
    
    RoomDef(int x_, int y_, int w, int h, const std::string& t = "normal")
        : x(x_), y(y_), width(w), height(h), type(t) {}
    
    bool isValid() const {
        return width >= 3 && height >= 3;
    }
    
    Point center() const {
        return Point(x + width/2, y + height/2);
    }
    
    bool overlaps(const RoomDef& other) const {
        return !(x + width <= other.x || 
                other.x + other.width <= x ||
                y + height <= other.y ||
                other.y + other.height <= y);
    }
};

class MapGenerator {
public:
    // Test map generators
    static void generateTestRoom(Map& map, int width = 20, int height = 20);
    static void generateTestDungeon(Map& map);
    static void generateCorridorTest(Map& map);
    static void generateCombatArena(Map& map);
    static void generateStressTest(Map& map);
    
    // General generation based on type
    static void generate(Map& map, MapType type);
    
    // Utilities
    static Point findSafeSpawnPoint(const Map& map);
    static Point findStairsLocation(const Map& map);
    static void placeStairs(Map& map, const Point& position);
    
    // Get spawn point for last generated map
    static Point getDefaultSpawnPoint(MapType type);
    
private:
    // Room generation helpers
    static bool canPlaceRoom(const Map& map, int x, int y, int w, int h);
    static void carveRoom(Map& map, int x, int y, int w, int h);
    static void carveCorridor(Map& map, const Point& start, const Point& end);
    
    // Create L-shaped corridor between two points
    static void carveCorridorL(Map& map, const Point& start, const Point& end);
};