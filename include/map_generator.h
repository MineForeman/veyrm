/**
 * @file map_generator.h
 * @brief Procedural map generation system for Veyrm
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "point.h"
#include "room.h"
#include <string>
#include <vector>
#include <random>

class Map;

/**
 * @enum MapType
 * @brief Types of maps that can be generated
 *
 * Defines the different map generation algorithms and layouts available.
 * Each type produces maps optimized for different purposes.
 */
enum class MapType {
    TEST_ROOM,      ///< Single 20x20 room for basic testing
    TEST_DUNGEON,   ///< Multi-room layout (current 5-room design) for development
    CORRIDOR_TEST,  ///< Long corridors for pathfinding and movement testing
    COMBAT_ARENA,   ///< Open space optimized for combat testing
    STRESS_TEST,    ///< Large complex map for performance testing
    PROCEDURAL      ///< Procedurally generated dungeon for gameplay
};

/**
 * @enum CorridorStyle
 * @brief Styles of corridors connecting rooms
 *
 * Defines how corridors are carved between rooms, affecting both
 * the visual appearance and tactical gameplay of the dungeon.
 */
enum class CorridorStyle {
    STRAIGHT,       ///< Direct path (shortest distance)
    L_SHAPED,       ///< One bend (current default implementation)
    S_SHAPED,       ///< Two bends for more natural appearance
    ORGANIC         ///< Natural winding path with multiple curves
};

/**
 * @enum ConnectionStrategy
 * @brief Strategies for connecting rooms with corridors
 *
 * Different algorithms for determining which rooms should be connected,
 * affecting dungeon connectivity and exploration patterns.
 */
enum class ConnectionStrategy {
    SEQUENTIAL,     ///< Connect rooms in order (simple chain)
    NEAREST,        ///< Connect each room to nearest unconnected room
    MST,            ///< Minimum spanning tree (optimal connectivity)
    RANDOM          ///< Random connections ensuring full connectivity
};

/**
 * @struct CorridorOptions
 * @brief Configuration options for corridor generation
 *
 * Contains parameters that control how corridors are generated and connected
 * between rooms in the dungeon.
 */
struct CorridorOptions {
    int width = 1;                                    ///< Width of corridors in tiles
    CorridorStyle style = CorridorStyle::L_SHAPED;    ///< Style of corridor paths
    bool placeDoors = true;                           ///< Whether to place doors at room entrances
    ConnectionStrategy strategy = ConnectionStrategy::MST; ///< Algorithm for connecting rooms
};

/**
 * @struct RoomDef
 * @brief Definition of a room for map generation
 *
 * Contains the position, size, and type information for a room.
 * Used during the room placement and validation phase of map generation.
 */
struct RoomDef {
    int x, y;           ///< Top-left corner coordinates
    int width, height;  ///< Dimensions of the room
    std::string type;   ///< Room type: "normal", "treasure", "boss"

    /**
     * @brief Construct a new room definition
     * @param x_ X coordinate of top-left corner
     * @param y_ Y coordinate of top-left corner
     * @param w Width of the room
     * @param h Height of the room
     * @param t Room type string (default: "normal")
     */
    RoomDef(int x_, int y_, int w, int h, const std::string& t = "normal")
        : x(x_), y(y_), width(w), height(h), type(t) {}

    /**
     * @brief Check if the room has valid dimensions
     * @return true if room is at least 3x3 tiles
     *
     * Rooms must be large enough to contain walkable floor space
     * after accounting for walls.
     */
    bool isValid() const {
        return width >= 3 && height >= 3;
    }

    /**
     * @brief Get the center point of the room
     * @return Point representing the room's center coordinates
     *
     * Used for corridor connections and distance calculations.
     */
    Point center() const {
        return Point(x + width/2, y + height/2);
    }

    /**
     * @brief Check if this room overlaps with another room
     * @param other The other room to check against
     * @return true if the rooms overlap, false otherwise
     *
     * Used during room placement to prevent overlapping rooms.
     */
    bool overlaps(const RoomDef& other) const {
        return !(x + width <= other.x ||
                other.x + other.width <= x ||
                y + height <= other.y ||
                other.y + other.height <= y);
    }
};

/**
 * @class MapGenerator
 * @brief Procedural map generation system for dungeons and test maps
 *
 * The MapGenerator class provides various algorithms for generating game maps,
 * from simple test layouts to complex procedural dungeons. It handles room
 * placement, corridor generation, door placement, and connectivity validation.
 *
 * The system supports multiple generation strategies:
 * - Predefined test maps for development and debugging
 * - Procedural room-and-corridor dungeons with configurable parameters
 * - Various corridor styles and connection algorithms
 * - Automatic spawn point and stair placement
 *
 * Maps are generated using classic roguelike algorithms including random room
 * placement with overlap detection, minimum spanning tree connectivity, and
 * L-shaped corridor carving.
 *
 * @see Map
 * @see Room
 * @see MapType
 * @see CorridorStyle
 * @see ConnectionStrategy
 */
class MapGenerator {
public:
    // Room generation parameters (scaled for Angband-sized maps)

    /// Minimum size for generated rooms (width or height)
    static constexpr int MIN_ROOM_SIZE = 4;
    /// Maximum size for generated rooms (width or height)
    static constexpr int MAX_ROOM_SIZE = 20;
    /// Minimum number of rooms in procedural dungeons
    static constexpr int MIN_ROOMS = 15;
    /// Maximum number of rooms in procedural dungeons
    static constexpr int MAX_ROOMS = 40;
    /// Maximum attempts to place a room before giving up
    static constexpr int MAX_PLACEMENT_ATTEMPTS = 2000;
    
    // Test map generators

    /**
     * @brief Generate a simple single room for basic testing
     * @param map The map to generate into
     * @param width Width of the test room (default: 20)
     * @param height Height of the test room (default: 20)
     *
     * Creates a single rectangular room centered on the map.
     * Useful for testing basic gameplay mechanics without
     * navigation complexity.
     */
    static void generateTestRoom(Map& map, int width = 20, int height = 20);

    /**
     * @brief Generate a multi-room test dungeon
     * @param map The map to generate into
     *
     * Creates a predefined 5-room layout with L-shaped corridors.
     * Used for consistent testing of room connectivity and pathfinding.
     */
    static void generateTestDungeon(Map& map);

    /**
     * @brief Generate long corridors for pathfinding testing
     * @param map The map to generate into
     *
     * Creates a map with extended corridors to test movement
     * algorithms and corridor navigation.
     */
    static void generateCorridorTest(Map& map);

    /**
     * @brief Generate an open arena optimized for combat testing
     * @param map The map to generate into
     *
     * Creates a large open space for testing combat mechanics
     * without environmental obstacles.
     */
    static void generateCombatArena(Map& map);

    /**
     * @brief Generate a large complex map for performance testing
     * @param map The map to generate into
     *
     * Creates a map with many rooms and corridors to stress-test
     * rendering, pathfinding, and field of view algorithms.
     */
    static void generateStressTest(Map& map);
    
    // General generation based on type

    /**
     * @brief Generate a map based on the specified type
     * @param map The map to generate into
     * @param type The type of map to generate
     *
     * Dispatches to the appropriate specialized generation method
     * based on the MapType. Uses random seed for procedural types.
     *
     * @see MapType
     */
    static void generate(Map& map, MapType type);

    /**
     * @brief Generate a map with a specific seed
     * @param map The map to generate into
     * @param type The type of map to generate
     * @param seed Random seed for reproducible generation
     *
     * Same as generate() but with explicit seed control for
     * deterministic map generation.
     */
    static void generate(Map& map, MapType type, unsigned int seed);

    // Random room generation

    /**
     * @brief Generate random rooms without corridors
     * @param map The map to place rooms in
     * @param seed Random seed (0 = random seed)
     * @return Vector of successfully placed rooms
     *
     * Places random rooms on the map using overlap detection.
     * Does not connect rooms with corridors.
     */
    static std::vector<Room> generateRandomRooms(Map& map, unsigned int seed = 0);

    /**
     * @brief Generate random rooms using provided RNG
     * @param map The map to place rooms in
     * @param rng Random number generator to use
     * @return Vector of successfully placed rooms
     *
     * Version that uses an existing RNG state for integration
     * with other procedural generation systems.
     */
    static std::vector<Room> generateRandomRooms(Map& map, std::mt19937& rng);

    /**
     * @brief Generate a complete procedural dungeon
     * @param map The map to generate into
     * @param seed Random seed (0 = random seed)
     *
     * Creates a full dungeon with rooms, corridors, doors, and
     * spawn/exit points using default corridor options.
     */
    static void generateProceduralDungeon(Map& map, unsigned int seed = 0);

    /**
     * @brief Generate a procedural dungeon with custom options
     * @param map The map to generate into
     * @param seed Random seed
     * @param options Configuration for corridor generation
     *
     * Full procedural generation with customizable corridor
     * styles and connection strategies.
     */
    static void generateProceduralDungeon(Map& map, unsigned int seed, const CorridorOptions& options);
    
    // Utilities

    /**
     * @brief Find a safe location for player spawn
     * @param map The map to search
     * @return Point representing a safe spawn location
     *
     * Searches for a floor tile that is not adjacent to walls
     * and provides good visibility. Prefers room centers.
     */
    static Point findSafeSpawnPoint(const Map& map);

    /**
     * @brief Find an appropriate location for stairs
     * @param map The map to search
     * @return Point representing stairs location
     *
     * Finds a floor tile suitable for stair placement,
     * typically in a room corner or corridor end.
     */
    static Point findStairsLocation(const Map& map);

    /**
     * @brief Place stairs at the specified position
     * @param map The map to modify
     * @param position Location to place stairs
     *
     * Sets the tile at the given position to stairs type.
     * Does not validate position - use findStairsLocation() first.
     */
    static void placeStairs(Map& map, const Point& position);

    // Get spawn point for last generated map

    /**
     * @brief Get the default spawn point for a map type
     * @param type The map type
     * @return Default spawn point coordinates
     *
     * Returns predefined spawn points for test maps and
     * calculated spawn points for procedural maps.
     */
    static Point getDefaultSpawnPoint(MapType type);

    /**
     * @brief Get spawn point based on map content and type
     * @param map The generated map
     * @param type The map type that was generated
     * @return Appropriate spawn point coordinates
     *
     * Combines map analysis with type-specific logic to
     * determine the best spawn location.
     */
    static Point getDefaultSpawnPoint(const Map& map, MapType type);
    
    // Room generation helpers

    /**
     * @brief Check if a room can be placed at the given coordinates
     * @param map The map to check placement on
     * @param x X coordinate of room's top-left corner
     * @param y Y coordinate of room's top-left corner
     * @param w Width of the room
     * @param h Height of the room
     * @return true if room can be placed without overlapping
     *
     * Validates that the room fits within map boundaries and
     * does not overlap with existing rooms or corridors.
     */
    static bool canPlaceRoom(const Map& map, int x, int y, int w, int h);

    /**
     * @brief Check if a room can be placed on the map
     * @param map The map to check placement on
     * @param room The room to test placement for
     * @return true if room can be placed without conflicts
     *
     * Convenience method that uses Room object coordinates.
     *
     * @see canPlaceRoom(const Map&, int, int, int, int)
     */
    static bool canPlaceRoom(const Map& map, const Room& room);

    /**
     * @brief Carve a room into the map
     * @param map The map to carve the room into
     * @param x X coordinate of room's top-left corner
     * @param y Y coordinate of room's top-left corner
     * @param w Width of the room
     * @param h Height of the room
     *
     * Creates a rectangular room by setting interior tiles to floor
     * and perimeter tiles to walls. Assumes valid placement.
     */
    static void carveRoom(Map& map, int x, int y, int w, int h);

    /**
     * @brief Carve a room using a Room object
     * @param map The map to carve the room into
     * @param room The room definition to carve
     *
     * Convenience method that uses Room object coordinates.
     *
     * @see carveRoom(Map&, int, int, int, int)
     */
    static void carveRoom(Map& map, const Room& room);
    
    /**
     * @brief Create an L-shaped corridor between two points
     * @param map The map to carve the corridor into
     * @param start Starting point of the corridor
     * @param end Ending point of the corridor
     *
     * Creates a corridor with one right-angle turn. This is the
     * default corridor style used in most dungeon generation.
     * The turn point is chosen to create a natural-looking path.
     */
    static void carveCorridorL(Map& map, const Point& start, const Point& end);

    // Corridor generation methods

    /**
     * @brief Connect rooms with corridors using specified options
     * @param map The map containing the rooms
     * @param rooms Vector of rooms to connect
     * @param options Configuration for corridor generation
     *
     * Creates corridors between rooms using the specified connection
     * strategy and corridor style. Ensures all rooms are reachable.
     */
    static void connectRooms(Map& map, std::vector<Room>& rooms, const CorridorOptions& options);

    /**
     * @brief Create a straight corridor between two points
     * @param map The map to carve the corridor into
     * @param start Starting point
     * @param end Ending point
     * @param width Width of the corridor (default: 1)
     *
     * Creates the shortest path between two points. May pass
     * through walls and other obstacles.
     */
    static void carveCorridorStraight(Map& map, const Point& start, const Point& end, int width = 1);

    /**
     * @brief Create an S-shaped corridor with two bends
     * @param map The map to carve the corridor into
     * @param start Starting point
     * @param end Ending point
     * @param width Width of the corridor (default: 1)
     *
     * Creates a corridor with two turns for a more natural
     * appearance than straight or L-shaped corridors.
     */
    static void carveCorridorS(Map& map, const Point& start, const Point& end, int width = 1);

    /**
     * @brief Create a corridor using the specified style
     * @param map The map to carve the corridor into
     * @param start Starting point
     * @param end Ending point
     * @param style Style of corridor to create
     * @param width Width of the corridor (default: 1)
     *
     * Dispatches to the appropriate corridor generation method
     * based on the specified style.
     *
     * @see CorridorStyle
     */
    static void carveCorridor(Map& map, const Point& start, const Point& end,
                             CorridorStyle style, int width = 1);
    
    // Connection strategies

    /**
     * @brief Get room connections using Minimum Spanning Tree algorithm
     * @param rooms Vector of rooms to connect
     * @return Vector of room index pairs to connect
     *
     * Uses MST algorithm to find the minimum set of corridors
     * needed to connect all rooms. Provides optimal connectivity
     * with minimal corridor length.
     */
    static std::vector<std::pair<int, int>> getMSTConnections(const std::vector<Room>& rooms);

    /**
     * @brief Get connections using nearest-neighbor strategy
     * @param rooms Vector of rooms to connect
     * @return Vector of room index pairs to connect
     *
     * Connects each room to its nearest unconnected neighbor.
     * May create longer overall corridor networks but provides
     * intuitive local connectivity.
     */
    static std::vector<std::pair<int, int>> getNearestConnections(const std::vector<Room>& rooms);

    /**
     * @brief Get sequential room connections
     * @param rooms Vector of rooms to connect
     * @return Vector of room index pairs to connect
     *
     * Connects rooms in order (0-1, 1-2, 2-3, etc.) creating
     * a simple chain. Useful for testing and simple layouts.
     */
    static std::vector<std::pair<int, int>> getSequentialConnections(const std::vector<Room>& rooms);
    
    // Door placement

    /**
     * @brief Place a door at a corridor/room intersection
     * @param map The map to place the door on
     * @param pos Position to place the door
     *
     * Places a door tile at the specified position. Typically called
     * at points where corridors meet room walls.
     */
    static void placeDoorAtIntersection(Map& map, const Point& pos);

    /**
     * @brief Place doors at all room entrances
     * @param map The map containing rooms and corridors
     * @param rooms Vector of rooms to add doors to
     *
     * Automatically finds corridor-room intersections and places
     * doors at appropriate entrance points.
     */
    static void placeDoorsAtRoomEntrances(Map& map, const std::vector<Room>& rooms);

    /**
     * @brief Check if a door should be placed at coordinates and place it
     * @param map The map to check and modify
     * @param x X coordinate to check
     * @param y Y coordinate to check
     *
     * Analyzes the surrounding tiles to determine if this location
     * is appropriate for a door, and places one if so.
     */
    static void checkAndPlaceDoor(Map& map, int x, int y);

    /**
     * @brief Find all points where corridors intersect with rooms
     * @param map The map to analyze
     * @param rooms Vector of rooms to check intersections for
     * @return Vector of points where corridors meet room boundaries
     *
     * Scans the map to identify locations where corridors connect
     * to rooms, which are candidates for door placement.
     */
    static std::vector<Point> findCorridorRoomIntersections(const Map& map, const std::vector<Room>& rooms);
    
private:
    /**
     * @brief Legacy corridor carving method
     * @param map The map to carve the corridor into
     * @param start Starting point
     * @param end Ending point
     *
     * Legacy method for creating corridors. Kept for backward
     * compatibility but replaced by the style-based carveCorridor method.
     *
     * @deprecated Use carveCorridor(Map&, const Point&, const Point&, CorridorStyle, int) instead
     */
    static void carveCorridor(Map& map, const Point& start, const Point& end);
};