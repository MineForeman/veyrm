#include "map_validator.h"
#include "map.h"
#include "map_generator.h"
#include <queue>
#include <algorithm>
#include <limits>

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

bool MapValidator::isWalkable(const Map& map, const Point& p) {
    return isWalkable(map, p.x, p.y);
}

bool MapValidator::validateAndFix(Map& map) {
    // Check advanced connectivity
    auto connectivity = checkAdvancedConnectivity(map);
    
    // If not fully connected, try to fix
    if (!connectivity.isFullyConnected && connectivity.numComponents > 1) {
        connectComponents(map, connectivity.components);
        
        // Re-check after fixing
        connectivity = checkAdvancedConnectivity(map);
        if (!connectivity.isFullyConnected) {
            return false; // Could not fix
        }
    }
    
    // Check if map is too small
    if (connectivity.largestComponent.size() < MIN_PLAYABLE_TILES) {
        return false; // Map too small, need to regenerate
    }
    
    // Ensure stairs are reachable
    if (!ensureStairsReachable(map)) {
        return false; // Could not ensure stairs reachable
    }
    
    return true;
}

ConnectivityResult MapValidator::checkAdvancedConnectivity(const Map& map) {
    ConnectivityResult result;
    result.totalFloorTiles = countWalkableTiles(map);
    
    if (result.totalFloorTiles == 0) {
        result.isFullyConnected = false;
        result.numComponents = 0;
        result.reachableFloorTiles = 0;
        return result;
    }
    
    // Find all components
    result.components = findAllComponents(map);
    result.numComponents = result.components.size();
    
    // Find largest component
    if (!result.components.empty()) {
        size_t maxSize = 0;
        size_t maxIdx = 0;
        for (size_t i = 0; i < result.components.size(); i++) {
            if (result.components[i].size() > maxSize) {
                maxSize = result.components[i].size();
                maxIdx = i;
            }
        }
        result.largestComponent = result.components[maxIdx];
        result.reachableFloorTiles = result.largestComponent.size();
    }
    
    // Check if fully connected
    result.isFullyConnected = (result.numComponents == 1);
    
    // Find unreachable tiles
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            if (isWalkable(map, x, y)) {
                Point p(x, y);
                if (result.largestComponent.find(p) == result.largestComponent.end()) {
                    result.unreachableTiles.insert(p);
                }
            }
        }
    }
    
    return result;
}

std::vector<std::set<Point>> MapValidator::findAllComponents(const Map& map) {
    std::vector<std::set<Point>> components;
    std::set<Point> visited;
    
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            Point p(x, y);
            if (isWalkable(map, x, y) && visited.find(p) == visited.end()) {
                // Found new component
                std::set<Point> component = bfsFloodFill(map, p);
                components.push_back(component);
                
                // Mark all as visited
                for (const auto& cp : component) {
                    visited.insert(cp);
                }
            }
        }
    }
    
    return components;
}

std::set<Point> MapValidator::bfsFloodFill(const Map& map, const Point& start) {
    std::set<Point> visited;
    std::queue<Point> queue;
    queue.push(start);
    
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    
    while (!queue.empty()) {
        Point current = queue.front();
        queue.pop();
        
        if (visited.find(current) != visited.end()) continue;
        visited.insert(current);
        
        // Check all 4 neighbors
        for (int i = 0; i < 4; i++) {
            Point next(current.x + dx[i], current.y + dy[i]);
            
            if (isWalkable(map, next) && visited.find(next) == visited.end()) {
                queue.push(next);
            }
        }
    }
    
    return visited;
}

bool MapValidator::isReachable(const Map& map, const Point& from, const Point& to) {
    if (!isWalkable(map, from) || !isWalkable(map, to)) {
        return false;
    }
    
    std::set<Point> reachable = bfsFloodFill(map, from);
    return reachable.find(to) != reachable.end();
}

std::set<Point> MapValidator::getReachableTiles(const Map& map, const Point& start) {
    if (!isWalkable(map, start)) {
        return std::set<Point>();
    }
    return bfsFloodFill(map, start);
}

void MapValidator::connectComponents(Map& map, const std::vector<std::set<Point>>& components) {
    if (components.size() <= 1) return;
    
    // Connect all components to the first (largest) one
    for (size_t i = 1; i < components.size(); i++) {
        Point p1, p2;
        findClosestPoints(components[0], components[i], p1, p2);
        
        // Use L-shaped corridor for better connectivity
        MapGenerator::carveCorridorL(map, p1, p2);
    }
}

void MapValidator::findClosestPoints(const std::set<Point>& comp1, 
                                      const std::set<Point>& comp2,
                                      Point& p1, Point& p2) {
    int minDist = std::numeric_limits<int>::max();
    
    for (const auto& a : comp1) {
        for (const auto& b : comp2) {
            int dist = manhattanDistance(a, b);
            if (dist < minDist) {
                minDist = dist;
                p1 = a;
                p2 = b;
            }
        }
    }
}

int MapValidator::manhattanDistance(const Point& a, const Point& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool MapValidator::ensureStairsReachable(Map& map) {
    Point stairs = findStairs(map);
    if (stairs.x == -1) {
        // No stairs found, that's ok
        return true;
    }
    
    Point start = findFirstFloorTile(map);
    if (start.x == -1) {
        return false; // No floor tiles!
    }
    
    if (!isReachable(map, start, stairs)) {
        // Try to connect stairs to main area
        auto components = findAllComponents(map);
        if (components.empty()) return false;
        
        // Find component containing stairs
        int stairsComp = -1;
        for (size_t i = 0; i < components.size(); i++) {
            if (components[i].find(stairs) != components[i].end()) {
                stairsComp = i;
                break;
            }
        }
        
        if (stairsComp == -1) {
            // Stairs not in any component, place on floor
            map.setTile(stairs.x, stairs.y, TileType::FLOOR);
        }
        
        // Re-validate
        return isReachable(map, start, stairs);
    }
    
    return true;
}

Point MapValidator::findStairs(const Map& map) {
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            TileType tile = map.getTile(x, y);
            if (tile == TileType::STAIRS_DOWN || tile == TileType::STAIRS_UP) {
                return Point(x, y);
            }
        }
    }
    return Point(-1, -1);
}

Point MapValidator::findFirstFloorTile(const Map& map) {
    return findWalkableTile(map);
}