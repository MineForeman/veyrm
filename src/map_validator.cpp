#include "map_validator.h"
#include "map.h"
#include <queue>

MapValidator::ValidationResult MapValidator::validate(const Map& map) {
    ValidationResult result;
    
    // Count tiles
    result.walkable_tiles = countWalkableTiles(map);
    result.wall_tiles = countWallTiles(map);
    
    // Calculate void tiles
    int total_tiles = map.getWidth() * map.getHeight();
    result.void_tiles = total_tiles - result.walkable_tiles - result.wall_tiles;
    
    // Check for walkable tiles
    if (result.walkable_tiles == 0) {
        result.addError("Map has no walkable tiles");
    } else if (result.walkable_tiles < 20) {
        result.addWarning("Map has very few walkable tiles (" + 
                         std::to_string(result.walkable_tiles) + ")");
    }
    
    // Check for spawn point
    Point spawn = findWalkableTile(map);
    result.has_spawn_point = (spawn.x != -1 && spawn.y != -1);
    if (!result.has_spawn_point) {
        result.addError("Map has no valid spawn point");
    }
    
    // Check for stairs
    result.has_stairs_down = false;
    result.has_stairs_up = false;
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            TileType tile = map.getTile(x, y);
            if (tile == TileType::STAIRS_DOWN) {
                result.has_stairs_down = true;
            } else if (tile == TileType::STAIRS_UP) {
                result.has_stairs_up = true;
            }
        }
    }
    
    if (!result.has_stairs_down && !result.has_stairs_up) {
        result.addWarning("Map has no stairs");
    }
    
    // Check connectivity
    result.is_connected = checkConnectivity(map);
    if (!result.is_connected && result.walkable_tiles > 0) {
        result.addError("Map has disconnected areas");
    }
    
    // Count rooms (approximate by finding separated floor areas)
    result.room_count = countRooms(map);
    if (result.room_count == 0 && result.walkable_tiles > 0) {
        result.addWarning("Could not identify distinct rooms");
    }
    
    // Check for reasonable wall ratio
    float wall_ratio = (float)result.wall_tiles / (float)total_tiles;
    if (wall_ratio < 0.1f) {
        result.addWarning("Very few walls in map");
    } else if (wall_ratio > 0.8f) {
        result.addWarning("Map is mostly walls");
    }
    
    return result;
}

bool MapValidator::checkConnectivity(const Map& map) {
    // Find first walkable tile
    Point start = findWalkableTile(map);
    if (start.x == -1 || start.y == -1) {
        return false;  // No walkable tiles
    }
    
    // Flood fill from that tile
    std::vector<std::vector<bool>> visited(map.getHeight(), 
                                          std::vector<bool>(map.getWidth(), false));
    floodFill(map, start.x, start.y, visited);
    
    // Check if all walkable tiles were visited
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            if (isWalkable(map, x, y) && !visited[y][x]) {
                return false;  // Found unreachable walkable tile
            }
        }
    }
    
    return true;
}

bool MapValidator::hasWalkableTiles(const Map& map) {
    return countWalkableTiles(map) > 0;
}

int MapValidator::countWalkableTiles(const Map& map) {
    int count = 0;
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            if (isWalkable(map, x, y)) {
                count++;
            }
        }
    }
    return count;
}

int MapValidator::countWallTiles(const Map& map) {
    int count = 0;
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            TileType tile = map.getTile(x, y);
            if (tile == TileType::WALL || tile == TileType::DOOR_CLOSED) {
                count++;
            }
        }
    }
    return count;
}

int MapValidator::countRooms(const Map& map) {
    // Simple room counting using flood fill
    std::vector<std::vector<bool>> visited(map.getHeight(), 
                                          std::vector<bool>(map.getWidth(), false));
    int room_count = 0;
    
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            if (isWalkable(map, x, y) && !visited[y][x]) {
                // Found new room/area
                floodFill(map, x, y, visited);
                room_count++;
            }
        }
    }
    
    return room_count;
}

bool MapValidator::hasStairs(const Map& map) {
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            TileType tile = map.getTile(x, y);
            if (tile == TileType::STAIRS_DOWN || tile == TileType::STAIRS_UP) {
                return true;
            }
        }
    }
    return false;
}

Point MapValidator::findWalkableTile(const Map& map) {
    // Find first walkable tile
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            if (isWalkable(map, x, y)) {
                return Point(x, y);
            }
        }
    }
    return Point(-1, -1);
}

void MapValidator::floodFill(const Map& map, int start_x, int start_y,
                            std::vector<std::vector<bool>>& visited) {
    // BFS flood fill
    std::queue<Point> queue;
    queue.push(Point(start_x, start_y));
    visited[start_y][start_x] = true;
    
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    
    while (!queue.empty()) {
        Point current = queue.front();
        queue.pop();
        
        // Check all 4 neighbors
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            
            // Check bounds
            if (nx >= 0 && nx < map.getWidth() && 
                ny >= 0 && ny < map.getHeight()) {
                // Check if walkable and not visited
                if (isWalkable(map, nx, ny) && !visited[ny][nx]) {
                    visited[ny][nx] = true;
                    queue.push(Point(nx, ny));
                }
            }
        }
    }
}

bool MapValidator::isWalkable(const Map& map, int x, int y) {
    if (!map.inBounds(x, y)) {
        return false;
    }
    
    TileType tile = map.getTile(x, y);
    // Get walkability from tile properties
    auto props = map.getTileProperties(tile);
    return props.walkable;
}