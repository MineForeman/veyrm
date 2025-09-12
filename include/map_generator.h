#pragma once

#include "point.h"
#include "room.h"
#include <string>
#include <vector>
#include <random>

class Map;

enum class MapType {
    TEST_ROOM,      // Single 20x20 room
    TEST_DUNGEON,   // Multi-room layout (current 5-room design)
    CORRIDOR_TEST,  // Long corridors for testing
    COMBAT_ARENA,   // Open space for combat
    STRESS_TEST,    // Large map for performance testing
    PROCEDURAL      // Procedurally generated dungeon
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
    // Room generation parameters
    static constexpr int MIN_ROOM_SIZE = 4;
    static constexpr int MAX_ROOM_SIZE = 12;
    static constexpr int MIN_ROOMS = 5;
    static constexpr int MAX_ROOMS = 15;
    static constexpr int MAX_PLACEMENT_ATTEMPTS = 1000;
    
    // Test map generators
    static void generateTestRoom(Map& map, int width = 20, int height = 20);
    static void generateTestDungeon(Map& map);
    static void generateCorridorTest(Map& map);
    static void generateCombatArena(Map& map);
    static void generateStressTest(Map& map);
    
    // General generation based on type
    static void generate(Map& map, MapType type);
    
    // Random room generation
    static std::vector<Room> generateRandomRooms(Map& map, unsigned int seed = 0);
    static std::vector<Room> generateRandomRooms(Map& map, std::mt19937& rng);
    static void generateProceduralDungeon(Map& map, unsigned int seed = 0);
    
    // Utilities
    static Point findSafeSpawnPoint(const Map& map);
    static Point findStairsLocation(const Map& map);
    static void placeStairs(Map& map, const Point& position);
    
    // Get spawn point for last generated map
    static Point getDefaultSpawnPoint(MapType type);
    static Point getDefaultSpawnPoint(const Map& map, MapType type);
    
private:
    // Room generation helpers
    static bool canPlaceRoom(const Map& map, int x, int y, int w, int h);
    static bool canPlaceRoom(const Map& map, const Room& room);
    static void carveRoom(Map& map, int x, int y, int w, int h);
    static void carveRoom(Map& map, const Room& room);
    static void carveCorridor(Map& map, const Point& start, const Point& end);
    
    // Create L-shaped corridor between two points
    static void carveCorridorL(Map& map, const Point& start, const Point& end);
};