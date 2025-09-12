#include "map_generator.h"
#include "map.h"
#include "map_validator.h"
#include <algorithm>
#include <random>
#include <climits>

void MapGenerator::generateTestRoom(Map& map, int width, int height) {
    // Fill with void
    map.fill(TileType::VOID);
    
    // Ensure room fits in map
    width = std::min(width, map.getWidth() - 4);
    height = std::min(height, map.getHeight() - 4);
    
    // Calculate room position (centered)
    int room_x = (map.getWidth() - width) / 2;
    int room_y = (map.getHeight() - height) / 2;
    
    // Create room
    carveRoom(map, room_x, room_y, width, height);
    
    // Place stairs in bottom-right corner
    map.setTile(room_x + width - 2, room_y + height - 2, TileType::STAIRS_DOWN);
}

void MapGenerator::generateTestDungeon(Map& map) {
    // This is the current 5-room layout from GameManager
    // Clear map
    map.fill(TileType::VOID);
    
    // Create rooms
    carveRoom(map, 10, 5, 20, 10);   // Top-left room
    carveRoom(map, 35, 5, 20, 10);   // Top-right room
    carveRoom(map, 10, 18, 20, 10);  // Bottom-left room
    carveRoom(map, 35, 18, 25, 10);  // Bottom-right room (larger)
    carveRoom(map, 22, 10, 16, 12);  // Central room
    
    // Connect rooms with corridors
    // Use points that are actually inside the rooms (past the walls)
    carveCorridorL(map, Point(29, 10), Point(35, 10));  // Top-left to top-right through central
    carveCorridorL(map, Point(29, 23), Point(35, 23));  // Bottom-left to bottom-right through central
    carveCorridorL(map, Point(20, 14), Point(30, 21));  // Top-left to central
    carveCorridorL(map, Point(45, 14), Point(37, 21));  // Top-right to central
    carveCorridorL(map, Point(30, 15), Point(30, 18));  // Central vertical connection
    
    // Place stairs in bottom-right room (inside the room, not on the wall)
    map.setTile(55, 22, TileType::STAIRS_DOWN);
}

void MapGenerator::generateCorridorTest(Map& map) {
    // Create a map with long corridors for testing movement
    map.fill(TileType::VOID);
    
    // Create two rooms connected by corridors
    carveRoom(map, 5, 5, 10, 10);
    carveRoom(map, 65, 15, 10, 8);
    
    // Long horizontal corridor with walls (extend to connect to first room)
    for (int x = 13; x <= 65; x++) {
        map.setTile(x, 10, TileType::FLOOR);
        // Add walls
        if (map.getTile(x, 9) == TileType::VOID) {
            map.setTile(x, 9, TileType::WALL);
        }
        if (map.getTile(x, 11) == TileType::VOID) {
            map.setTile(x, 11, TileType::WALL);
        }
    }
    
    // Vertical corridor with walls (extend to connect to second room)
    for (int y = 10; y <= 16; y++) {
        map.setTile(65, y, TileType::FLOOR);
        // Add walls
        if (map.getTile(64, y) == TileType::VOID) {
            map.setTile(64, y, TileType::WALL);
        }
        if (map.getTile(66, y) == TileType::VOID) {
            map.setTile(66, y, TileType::WALL);
        }
    }
    
    // Narrow 1-tile corridor with walls (connect to first room)
    for (int x = 10; x < 20; x++) {
        map.setTile(x, 20, TileType::FLOOR);
        // Add walls
        if (map.getTile(x, 19) == TileType::VOID) {
            map.setTile(x, 19, TileType::WALL);
        }
        if (map.getTile(x, 21) == TileType::VOID) {
            map.setTile(x, 21, TileType::WALL);
        }
    }
    
    // Connect narrow corridor to first room
    for (int y = 14; y <= 20; y++) {
        map.setTile(10, y, TileType::FLOOR);
        // Add walls
        if (map.getTile(9, y) == TileType::VOID) {
            map.setTile(9, y, TileType::WALL);
        }
        if (map.getTile(11, y) == TileType::VOID) {
            map.setTile(11, y, TileType::WALL);
        }
    }
    
    // Place stairs
    map.setTile(70, 19, TileType::STAIRS_DOWN);
}

void MapGenerator::generateCombatArena(Map& map) {
    // Large open space for combat testing
    map.fill(TileType::VOID);
    
    // Create large arena
    carveRoom(map, 20, 5, 40, 18);
    
    // Add some pillars for tactical positioning
    map.setTile(30, 10, TileType::WALL);
    map.setTile(30, 18, TileType::WALL);
    map.setTile(50, 10, TileType::WALL);
    map.setTile(50, 18, TileType::WALL);
    
    // Place stairs
    map.setTile(58, 21, TileType::STAIRS_DOWN);
}

void MapGenerator::generateStressTest(Map& map) {
    // Generate many rooms for performance testing
    map.fill(TileType::VOID);
    
    std::vector<RoomDef> rooms;
    std::mt19937 rng(12345); // Fixed seed for reproducibility
    std::uniform_int_distribution<int> room_size(4, 12);
    std::uniform_int_distribution<int> x_dist(2, map.getWidth() - 15);
    std::uniform_int_distribution<int> y_dist(2, map.getHeight() - 15);
    
    // Try to place many rooms
    for (int i = 0; i < 50; i++) {
        int w = room_size(rng);
        int h = room_size(rng);
        int x = x_dist(rng);
        int y = y_dist(rng);
        
        // Check if room fits
        if (x + w < map.getWidth() - 1 && y + h < map.getHeight() - 1) {
            RoomDef new_room(x, y, w, h);
            
            // Check for overlaps
            bool overlaps = false;
            for (const auto& room : rooms) {
                if (new_room.overlaps(room)) {
                    overlaps = true;
                    break;
                }
            }
            
            if (!overlaps) {
                carveRoom(map, x, y, w, h);
                rooms.push_back(new_room);
            }
        }
    }
    
    // Connect all rooms with corridors
    for (size_t i = 1; i < rooms.size(); i++) {
        Point start = rooms[i-1].center();
        Point end = rooms[i].center();
        carveCorridorL(map, start, end);
    }
    
    // Place stairs in last room
    if (!rooms.empty()) {
        const auto& last_room = rooms.back();
        map.setTile(last_room.center().x, last_room.center().y, TileType::STAIRS_DOWN);
    }
}

void MapGenerator::generate(Map& map, MapType type) {
    switch (type) {
        case MapType::TEST_ROOM:
            generateTestRoom(map);
            break;
        case MapType::TEST_DUNGEON:
            generateTestDungeon(map);
            break;
        case MapType::CORRIDOR_TEST:
            generateCorridorTest(map);
            break;
        case MapType::COMBAT_ARENA:
            generateCombatArena(map);
            break;
        case MapType::STRESS_TEST:
            generateStressTest(map);
            break;
        case MapType::PROCEDURAL:
            generateProceduralDungeon(map);
            break;
    }
}

Point MapGenerator::findSafeSpawnPoint(const Map& map) {
    // Find first walkable floor tile
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            if (map.getTile(x, y) == TileType::FLOOR) {
                return Point(x, y);
            }
        }
    }
    // Fallback to center
    return Point(map.getWidth() / 2, map.getHeight() / 2);
}

Point MapGenerator::findStairsLocation(const Map& map) {
    // Find stairs position
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            if (map.getTile(x, y) == TileType::STAIRS_DOWN ||
                map.getTile(x, y) == TileType::STAIRS_UP) {
                return Point(x, y);
            }
        }
    }
    return Point(-1, -1);
}

void MapGenerator::placeStairs(Map& map, const Point& position) {
    if (map.inBounds(position)) {
        map.setTile(position.x, position.y, TileType::STAIRS_DOWN);
    }
}

Point MapGenerator::getDefaultSpawnPoint(MapType type) {
    // Calculate spawn points based on actual room layouts
    switch (type) {
        case MapType::TEST_ROOM:
            // TEST_ROOM: centered 20x20 room in 198x66 map
            // Room at (89, 23) with size 20x20, center at (99, 33)
            return Point(99, 33);  // Center of room
        case MapType::TEST_DUNGEON:
            // Central room is at (22, 10, 16, 12), floor is at (23-37, 11-21)
            return Point(30, 16);  // Center of central room floor
        case MapType::CORRIDOR_TEST:
            // First room is at (5, 5, 10, 10), floor is at (6-14, 6-14)
            return Point(10, 10);  // Center of first room floor
        case MapType::COMBAT_ARENA:
            // Arena is at (20, 5, 40, 18), floor is at (21-59, 6-22)
            return Point(40, 14);  // Center of arena floor
        case MapType::STRESS_TEST:
        case MapType::PROCEDURAL:
            // Random/procedural maps need to find a safe spawn point
            return Point(99, 33);  // Center of map, will be validated
        default:
            return Point(99, 33);  // Center of map
    }
}

Point MapGenerator::getDefaultSpawnPoint(const Map& map, MapType type) {
    // For random/procedural maps, find a safe spawn point
    if (type == MapType::STRESS_TEST || type == MapType::PROCEDURAL) {
        return findSafeSpawnPoint(map);
    }
    
    // For fixed maps, verify the default point is walkable, otherwise find safe one
    Point defaultPoint = getDefaultSpawnPoint(type);
    if (map.inBounds(defaultPoint)) {
        auto props = Map::getTileProperties(map.getTile(defaultPoint.x, defaultPoint.y));
        if (props.walkable) {
            return defaultPoint;
        }
    }
    
    // Fallback to safe spawn point
    return findSafeSpawnPoint(map);
}

bool MapGenerator::canPlaceRoom(const Map& map, int x, int y, int w, int h) {
    // Check bounds
    if (x < 1 || y < 1 || x + w >= map.getWidth() - 1 || y + h >= map.getHeight() - 1) {
        return false;
    }
    
    // Check for existing tiles (with 1 tile buffer)
    for (int py = y - 1; py < y + h + 1; py++) {
        for (int px = x - 1; px < x + w + 1; px++) {
            if (map.getTile(px, py) != TileType::VOID) {
                return false;
            }
        }
    }
    
    return true;
}

void MapGenerator::carveRoom(Map& map, int x, int y, int w, int h) {
    // Create walls and floor
    for (int py = y; py < y + h; py++) {
        for (int px = x; px < x + w; px++) {
            if (px == x || px == x + w - 1 || py == y || py == y + h - 1) {
                // Walls on perimeter
                map.setTile(px, py, TileType::WALL);
            } else {
                // Floor inside
                map.setTile(px, py, TileType::FLOOR);
            }
        }
    }
}

void MapGenerator::carveCorridor(Map& map, const Point& start, const Point& end) {
    // Simple straight corridor (horizontal then vertical)
    carveCorridorL(map, start, end);
}

void MapGenerator::carveCorridorL(Map& map, const Point& start, const Point& end) {
    // L-shaped corridor: horizontal first, then vertical
    int x = start.x;
    int y = start.y;
    
    // Move horizontally
    while (x != end.x) {
        // Place floor
        map.setTile(x, y, TileType::FLOOR);
        
        // Add walls around corridor if they're void
        if (map.getTile(x, y-1) == TileType::VOID) {
            map.setTile(x, y-1, TileType::WALL);
        }
        if (map.getTile(x, y+1) == TileType::VOID) {
            map.setTile(x, y+1, TileType::WALL);
        }
        
        x += (end.x > x) ? 1 : -1;
    }
    
    // Move vertically
    while (y != end.y) {
        // Place floor
        map.setTile(x, y, TileType::FLOOR);
        
        // Add walls around corridor if they're void
        if (map.getTile(x-1, y) == TileType::VOID) {
            map.setTile(x-1, y, TileType::WALL);
        }
        if (map.getTile(x+1, y) == TileType::VOID) {
            map.setTile(x+1, y, TileType::WALL);
        }
        
        y += (end.y > y) ? 1 : -1;
    }
    
    // Ensure end point is floor
    map.setTile(end.x, end.y, TileType::FLOOR);
    
    // Add corner walls where the corridor bends
    int bend_x = end.x;
    int bend_y = start.y;
    
    // Add walls at the corner if void
    if (start.x < end.x) {
        // Moving right then up/down
        if (map.getTile(bend_x+1, bend_y) == TileType::VOID) {
            map.setTile(bend_x+1, bend_y, TileType::WALL);
        }
        if (start.y < end.y) {
            // Moving right then down
            if (map.getTile(bend_x, bend_y-1) == TileType::VOID) {
                map.setTile(bend_x, bend_y-1, TileType::WALL);
            }
            if (map.getTile(bend_x+1, bend_y-1) == TileType::VOID) {
                map.setTile(bend_x+1, bend_y-1, TileType::WALL);
            }
        } else if (start.y > end.y) {
            // Moving right then up
            if (map.getTile(bend_x, bend_y+1) == TileType::VOID) {
                map.setTile(bend_x, bend_y+1, TileType::WALL);
            }
            if (map.getTile(bend_x+1, bend_y+1) == TileType::VOID) {
                map.setTile(bend_x+1, bend_y+1, TileType::WALL);
            }
        }
    } else if (start.x > end.x) {
        // Moving left then up/down
        if (map.getTile(bend_x-1, bend_y) == TileType::VOID) {
            map.setTile(bend_x-1, bend_y, TileType::WALL);
        }
        if (start.y < end.y) {
            // Moving left then down
            if (map.getTile(bend_x, bend_y-1) == TileType::VOID) {
                map.setTile(bend_x, bend_y-1, TileType::WALL);
            }
            if (map.getTile(bend_x-1, bend_y-1) == TileType::VOID) {
                map.setTile(bend_x-1, bend_y-1, TileType::WALL);
            }
        } else if (start.y > end.y) {
            // Moving left then up
            if (map.getTile(bend_x, bend_y+1) == TileType::VOID) {
                map.setTile(bend_x, bend_y+1, TileType::WALL);
            }
            if (map.getTile(bend_x-1, bend_y+1) == TileType::VOID) {
                map.setTile(bend_x-1, bend_y+1, TileType::WALL);
            }
        }
    }
}

std::vector<Room> MapGenerator::generateRandomRooms(Map& map, unsigned int seed) {
    std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
    return generateRandomRooms(map, rng);
}

std::vector<Room> MapGenerator::generateRandomRooms(Map& map, std::mt19937& rng) {
    std::vector<Room> rooms;
    
    // Distribution for room dimensions and positions
    std::uniform_int_distribution<int> room_width(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
    std::uniform_int_distribution<int> room_height(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
    std::uniform_int_distribution<int> x_pos(2, std::max(2, map.getWidth() - MAX_ROOM_SIZE - 2));
    std::uniform_int_distribution<int> y_pos(2, std::max(2, map.getHeight() - MAX_ROOM_SIZE - 2));
    std::uniform_int_distribution<int> room_count(MIN_ROOMS, MAX_ROOMS);
    
    int target_rooms = room_count(rng);
    int attempts = 0;
    
    // Clear map first
    map.fill(TileType::VOID);
    
    while (static_cast<int>(rooms.size()) < target_rooms && attempts < MAX_PLACEMENT_ATTEMPTS) {
        // Generate random room dimensions
        int w = room_width(rng);
        int h = room_height(rng);
        int x = x_pos(rng);
        int y = y_pos(rng);
        
        // Ensure room fits in map bounds
        if (x + w >= map.getWidth() - 1 || y + h >= map.getHeight() - 1) {
            attempts++;
            continue;
        }
        
        Room new_room(x, y, w, h);
        
        // Check for overlaps with existing rooms (with 2 tile padding)
        bool overlaps = false;
        for (const auto& room : rooms) {
            if (new_room.overlaps(room, 2)) {
                overlaps = true;
                break;
            }
        }
        
        if (!overlaps && new_room.isValid()) {
            rooms.push_back(new_room);
            carveRoom(map, new_room);
        }
        
        attempts++;
    }
    
    // Ensure at least one room was created
    if (rooms.empty()) {
        // Force create a room in the center
        int w = MIN_ROOM_SIZE + 2;
        int h = MIN_ROOM_SIZE + 2;
        int x = map.getWidth() / 2 - w / 2;
        int y = map.getHeight() / 2 - h / 2;
        Room emergency_room(x, y, w, h);
        rooms.push_back(emergency_room);
        carveRoom(map, emergency_room);
    }
    
    return rooms;
}

void MapGenerator::generateProceduralDungeon(Map& map, unsigned int seed) {
    // Generate random rooms
    auto rooms = generateRandomRooms(map, seed);
    
    // Connect rooms with corridors (simple approach: connect each room to the next)
    for (size_t i = 1; i < rooms.size(); i++) {
        Point start = rooms[i-1].center();
        Point end = rooms[i].center();
        carveCorridorL(map, start, end);
    }
    
    // Place stairs in the last room
    if (!rooms.empty()) {
        const auto& last_room = rooms.back();
        Point stairs = last_room.center();
        map.setTile(stairs.x, stairs.y, TileType::STAIRS_DOWN);
    }
}

bool MapGenerator::canPlaceRoom(const Map& map, const Room& room) {
    return canPlaceRoom(map, room.x, room.y, room.width, room.height);
}

void MapGenerator::carveRoom(Map& map, const Room& room) {
    carveRoom(map, room.x, room.y, room.width, room.height);
}

void MapGenerator::generateProceduralDungeon(Map& map, unsigned int seed, const CorridorOptions& options) {
    const int MAX_GENERATION_ATTEMPTS = 5;
    
    for (int attempt = 0; attempt < MAX_GENERATION_ATTEMPTS; attempt++) {
        // Generate random rooms
        auto rooms = generateRandomRooms(map, seed + attempt);
        
        // Connect rooms using specified strategy
        connectRooms(map, rooms, options);
        
        // Place stairs in the last room
        if (!rooms.empty()) {
            const auto& last_room = rooms.back();
            Point stairs = last_room.center();
            map.setTile(stairs.x, stairs.y, TileType::STAIRS_DOWN);
        }
        
        // Validate and fix the map
        if (MapValidator::validateAndFix(map)) {
            // Map is valid!
            return;
        }
        
        // Map invalid, try again with different seed
        map.fill(TileType::VOID);
    }
    
    // Failed to generate valid map after attempts, use fallback
    generateTestDungeon(map);
}

void MapGenerator::connectRooms(Map& map, std::vector<Room>& rooms, const CorridorOptions& options) {
    if (rooms.size() < 2) return;
    
    // Get connections based on strategy
    std::vector<std::pair<int, int>> connections;
    switch (options.strategy) {
        case ConnectionStrategy::MST:
            connections = getMSTConnections(rooms);
            break;
        case ConnectionStrategy::NEAREST:
            connections = getNearestConnections(rooms);
            break;
        case ConnectionStrategy::SEQUENTIAL:
            connections = getSequentialConnections(rooms);
            break;
        case ConnectionStrategy::RANDOM:
            // For now, use sequential as fallback
            connections = getSequentialConnections(rooms);
            break;
    }
    
    // Create corridors for each connection
    for (const auto& [from, to] : connections) {
        Point start = rooms[from].center();
        Point end = rooms[to].center();
        carveCorridor(map, start, end, options.style, options.width);
        
        // Place doors if requested
        if (options.placeDoors) {
            // Find where corridor meets rooms and place doors
            // This is simplified - proper implementation would find exact intersection
            placeDoorAtIntersection(map, start);
            placeDoorAtIntersection(map, end);
        }
    }
}

void MapGenerator::carveCorridorStraight(Map& map, const Point& start, const Point& end, int width) {
    // First pass: carve the corridor floors
    // Use Bresenham's line algorithm for straight corridors
    int x1 = start.x, y1 = start.y;
    int x2 = end.x, y2 = end.y;
    
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    // First carve all floors
    int tx = x1, ty = y1;
    while (true) {
        // Carve floor with specified width
        for (int w = 0; w < width; w++) {
            for (int h = 0; h < width; h++) {
                if (map.inBounds(tx + w, ty + h)) {
                    map.setTile(tx + w, ty + h, TileType::FLOOR);
                }
            }
        }
        
        if (tx == x2 && ty == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            tx += sx;
        }
        if (e2 < dx) {
            err += dx;
            ty += sy;
        }
    }
    
    // Second pass: add walls around the corridor
    x1 = start.x; y1 = start.y;
    err = dx - dy;
    
    while (true) {
        // Add walls around the corridor (only if void)
        for (int w = -1; w <= width; w++) {
            for (int h = -1; h <= width; h++) {
                if (w == -1 || w == width || h == -1 || h == width) {
                    if (map.inBounds(x1 + w, y1 + h)) {
                        if (map.getTile(x1 + w, y1 + h) == TileType::VOID) {
                            map.setTile(x1 + w, y1 + h, TileType::WALL);
                        }
                    }
                }
            }
        }
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void MapGenerator::carveCorridorS(Map& map, const Point& start, const Point& end, int width) {
    // S-shaped corridor with two bends
    // Calculate two intermediate points for the S-shape
    int mid_x = (start.x + end.x) / 2;
    
    Point bend1(mid_x, start.y);
    Point bend2(mid_x, end.y);
    
    // Carve three segments
    carveCorridorStraight(map, start, bend1, width);
    carveCorridorStraight(map, bend1, bend2, width);
    carveCorridorStraight(map, bend2, end, width);
}

void MapGenerator::carveCorridor(Map& map, const Point& start, const Point& end, 
                                CorridorStyle style, int width) {
    switch (style) {
        case CorridorStyle::STRAIGHT:
            carveCorridorStraight(map, start, end, width);
            break;
        case CorridorStyle::L_SHAPED:
            if (width == 1) {
                carveCorridorL(map, start, end);
            } else {
                // For wider corridors, use straight segments
                Point bend(end.x, start.y);
                carveCorridorStraight(map, start, bend, width);
                carveCorridorStraight(map, bend, end, width);
            }
            break;
        case CorridorStyle::S_SHAPED:
            carveCorridorS(map, start, end, width);
            break;
        case CorridorStyle::ORGANIC:
            // For now, use L-shaped as fallback
            carveCorridor(map, start, end, CorridorStyle::L_SHAPED, width);
            break;
    }
}

std::vector<std::pair<int, int>> MapGenerator::getMSTConnections(const std::vector<Room>& rooms) {
    std::vector<std::pair<int, int>> connections;
    if (rooms.size() < 2) return connections;
    
    // Simple MST using Prim's algorithm
    std::vector<bool> visited(rooms.size(), false);
    visited[0] = true;
    
    while (connections.size() < rooms.size() - 1) {
        int min_dist = INT_MAX;
        int from_idx = -1, to_idx = -1;
        
        // Find minimum edge from visited to unvisited
        for (size_t i = 0; i < rooms.size(); i++) {
            if (!visited[i]) continue;
            
            for (size_t j = 0; j < rooms.size(); j++) {
                if (visited[j]) continue;
                
                Point p1 = rooms[i].center();
                Point p2 = rooms[j].center();
                int dist = abs(p2.x - p1.x) + abs(p2.y - p1.y);  // Manhattan distance
                
                if (dist < min_dist) {
                    min_dist = dist;
                    from_idx = i;
                    to_idx = j;
                }
            }
        }
        
        if (to_idx != -1) {
            connections.push_back({from_idx, to_idx});
            visited[to_idx] = true;
        } else {
            break;  // No more connections possible
        }
    }
    
    return connections;
}

std::vector<std::pair<int, int>> MapGenerator::getNearestConnections(const std::vector<Room>& rooms) {
    std::vector<std::pair<int, int>> connections;
    std::vector<bool> connected(rooms.size(), false);
    
    if (rooms.empty()) return connections;
    connected[0] = true;
    
    // Connect each room to its nearest unconnected neighbor
    for (size_t i = 0; i < rooms.size() - 1; i++) {
        int min_dist = INT_MAX;
        int nearest = -1;
        
        for (size_t j = 0; j < rooms.size(); j++) {
            if (connected[j]) continue;
            
            // Find nearest unconnected room to any connected room
            for (size_t k = 0; k < rooms.size(); k++) {
                if (!connected[k]) continue;
                
                Point p1 = rooms[k].center();
                Point p2 = rooms[j].center();
                int dist = abs(p2.x - p1.x) + abs(p2.y - p1.y);
                
                if (dist < min_dist) {
                    min_dist = dist;
                    nearest = j;
                    connections.push_back({k, j});
                }
            }
        }
        
        if (nearest != -1) {
            connected[nearest] = true;
        }
    }
    
    return connections;
}

std::vector<std::pair<int, int>> MapGenerator::getSequentialConnections(const std::vector<Room>& rooms) {
    std::vector<std::pair<int, int>> connections;
    
    for (size_t i = 1; i < rooms.size(); i++) {
        connections.push_back({i - 1, i});
    }
    
    return connections;
}

void MapGenerator::placeDoorAtIntersection(Map& map, const Point& pos) {
    // Simple door placement - check if position is at room boundary
    // Look for floor adjacent to wall pattern
    bool hasFloor = false;
    bool hasWall = false;
    
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            
            int x = pos.x + dx;
            int y = pos.y + dy;
            
            if (map.inBounds(x, y)) {
                TileType tile = map.getTile(x, y);
                if (tile == TileType::FLOOR) hasFloor = true;
                if (tile == TileType::WALL) hasWall = true;
            }
        }
    }
    
    // Place door if we're at a transition point
    if (hasFloor && hasWall && map.getTile(pos.x, pos.y) == TileType::FLOOR) {
        map.setTile(pos.x, pos.y, TileType::DOOR_CLOSED);
    }
}

std::vector<Point> MapGenerator::findCorridorRoomIntersections(const Map& map, const std::vector<Room>& rooms) {
    std::vector<Point> intersections;
    
    // Find points where corridors meet room boundaries
    for (const auto& room : rooms) {
        // Check room perimeter
        for (int x = room.x; x < room.x + room.width; x++) {
            // Top and bottom edges
            if (map.getTile(x, room.y - 1) == TileType::FLOOR) {
                intersections.push_back(Point(x, room.y));
            }
            if (map.getTile(x, room.y + room.height) == TileType::FLOOR) {
                intersections.push_back(Point(x, room.y + room.height - 1));
            }
        }
        
        for (int y = room.y; y < room.y + room.height; y++) {
            // Left and right edges
            if (map.getTile(room.x - 1, y) == TileType::FLOOR) {
                intersections.push_back(Point(room.x, y));
            }
            if (map.getTile(room.x + room.width, y) == TileType::FLOOR) {
                intersections.push_back(Point(room.x + room.width - 1, y));
            }
        }
    }
    
    return intersections;
}