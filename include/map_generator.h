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

enum class CorridorStyle {
    STRAIGHT,       // Direct path (shortest)
    L_SHAPED,       // One bend (current implementation)
    S_SHAPED,       // Two bends
    ORGANIC         // Natural winding path
};

enum class ConnectionStrategy {
    SEQUENTIAL,     // Connect rooms in order (current)
    NEAREST,        // Connect to nearest unconnected room
    MST,            // Minimum spanning tree
    RANDOM          // Random connections ensuring connectivity
};

struct CorridorOptions {
    int width = 1;
    CorridorStyle style = CorridorStyle::L_SHAPED;
    bool placeDoors = true;
    ConnectionStrategy strategy = ConnectionStrategy::MST;
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
    // Room generation parameters (scaled for Angband-sized maps)
    static constexpr int MIN_ROOM_SIZE = 4;
    static constexpr int MAX_ROOM_SIZE = 20;   // Larger rooms for bigger map
    static constexpr int MIN_ROOMS = 15;       // More rooms for larger map
    static constexpr int MAX_ROOMS = 40;       // Many more rooms possible
    static constexpr int MAX_PLACEMENT_ATTEMPTS = 2000;  // More attempts needed
    
    // Test map generators
    static void generateTestRoom(Map& map, int width = 20, int height = 20);
    static void generateTestDungeon(Map& map);
    static void generateCorridorTest(Map& map);
    static void generateCombatArena(Map& map);
    static void generateStressTest(Map& map);
    
    // General generation based on type
    static void generate(Map& map, MapType type);
    static void generate(Map& map, MapType type, unsigned int seed);
    
    // Random room generation
    static std::vector<Room> generateRandomRooms(Map& map, unsigned int seed = 0);
    static std::vector<Room> generateRandomRooms(Map& map, std::mt19937& rng);
    static void generateProceduralDungeon(Map& map, unsigned int seed = 0);
    static void generateProceduralDungeon(Map& map, unsigned int seed, const CorridorOptions& options);
    
    // Utilities
    static Point findSafeSpawnPoint(const Map& map);
    static Point findStairsLocation(const Map& map);
    static void placeStairs(Map& map, const Point& position);
    
    // Get spawn point for last generated map
    static Point getDefaultSpawnPoint(MapType type);
    static Point getDefaultSpawnPoint(const Map& map, MapType type);
    
    // Room generation helpers
    static bool canPlaceRoom(const Map& map, int x, int y, int w, int h);
    static bool canPlaceRoom(const Map& map, const Room& room);
    static void carveRoom(Map& map, int x, int y, int w, int h);
    static void carveRoom(Map& map, const Room& room);
    
    // Create L-shaped corridor between two points
    static void carveCorridorL(Map& map, const Point& start, const Point& end);
    
    // Corridor generation methods
    static void connectRooms(Map& map, std::vector<Room>& rooms, const CorridorOptions& options);
    static void carveCorridorStraight(Map& map, const Point& start, const Point& end, int width = 1);
    static void carveCorridorS(Map& map, const Point& start, const Point& end, int width = 1);
    static void carveCorridor(Map& map, const Point& start, const Point& end, 
                             CorridorStyle style, int width = 1);
    
    // Connection strategies
    static std::vector<std::pair<int, int>> getMSTConnections(const std::vector<Room>& rooms);
    static std::vector<std::pair<int, int>> getNearestConnections(const std::vector<Room>& rooms);
    static std::vector<std::pair<int, int>> getSequentialConnections(const std::vector<Room>& rooms);
    
    // Door placement
    static void placeDoorAtIntersection(Map& map, const Point& pos);
    static void placeDoorsAtRoomEntrances(Map& map, const std::vector<Room>& rooms);
    static void checkAndPlaceDoor(Map& map, int x, int y);
    static std::vector<Point> findCorridorRoomIntersections(const Map& map, const std::vector<Room>& rooms);
    
private:
    // Legacy corridor method
    static void carveCorridor(Map& map, const Point& start, const Point& end);
};