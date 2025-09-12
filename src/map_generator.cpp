#include "map_generator.h"
#include "map.h"
#include <algorithm>
#include <random>

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
    
    // Place stairs in bottom-right room
    map.setTile(55, 25, TileType::STAIRS_DOWN);
}

void MapGenerator::generateCorridorTest(Map& map) {
    // Create a map with long corridors for testing movement
    map.fill(TileType::VOID);
    
    // Create two rooms connected by corridors
    carveRoom(map, 5, 5, 10, 10);
    carveRoom(map, 65, 15, 10, 8);
    
    // Long horizontal corridor with walls
    for (int x = 14; x < 65; x++) {
        map.setTile(x, 10, TileType::FLOOR);
        // Add walls
        if (map.getTile(x, 9) == TileType::VOID) {
            map.setTile(x, 9, TileType::WALL);
        }
        if (map.getTile(x, 11) == TileType::VOID) {
            map.setTile(x, 11, TileType::WALL);
        }
    }
    
    // Vertical corridor with walls
    for (int y = 10; y < 15; y++) {
        map.setTile(65, y, TileType::FLOOR);
        // Add walls
        if (map.getTile(64, y) == TileType::VOID) {
            map.setTile(64, y, TileType::WALL);
        }
        if (map.getTile(66, y) == TileType::VOID) {
            map.setTile(66, y, TileType::WALL);
        }
    }
    
    // Narrow 1-tile corridor with walls
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
    switch (type) {
        case MapType::TEST_ROOM:
            return Point(40, 12);  // Center of centered room
        case MapType::TEST_DUNGEON:
            return Point(30, 15);  // Central room
        case MapType::CORRIDOR_TEST:
            return Point(10, 10);  // First room
        case MapType::COMBAT_ARENA:
            return Point(40, 14);  // Center of arena
        case MapType::STRESS_TEST:
            return Point(40, 12);  // Approximate center
        default:
            return Point(40, 12);
    }
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