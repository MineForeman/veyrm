#include "wall_connector.h"
#include "map.h"

bool WallConnector::unicode_enabled = false; // Start with ASCII for compatibility

bool WallConnector::isUnicodeEnabled() {
    return unicode_enabled;
}

void WallConnector::setUnicodeEnabled(bool enabled) {
    unicode_enabled = enabled;
}

bool WallConnector::hasWallNorth(const Map& map, int x, int y) {
    return map.getTile(x, y - 1) == TileType::WALL || 
           map.getTile(x, y - 1) == TileType::DOOR_CLOSED;
}

bool WallConnector::hasWallSouth(const Map& map, int x, int y) {
    return map.getTile(x, y + 1) == TileType::WALL || 
           map.getTile(x, y + 1) == TileType::DOOR_CLOSED;
}

bool WallConnector::hasWallEast(const Map& map, int x, int y) {
    return map.getTile(x + 1, y) == TileType::WALL || 
           map.getTile(x + 1, y) == TileType::DOOR_CLOSED;
}

bool WallConnector::hasWallWest(const Map& map, int x, int y) {
    return map.getTile(x - 1, y) == TileType::WALL || 
           map.getTile(x - 1, y) == TileType::DOOR_CLOSED;
}

char WallConnector::getASCIIWall(bool n, bool s, bool e, bool w) {
    // Count connections
    int connections = (n ? 1 : 0) + (s ? 1 : 0) + (e ? 1 : 0) + (w ? 1 : 0);
    
    // Use different ASCII characters based on connections
    switch (connections) {
        case 0: return '#';  // Isolated wall (pillar)
        case 1: return '#';  // Dead end
        case 2:
            if ((n && s) || (e && w)) return '#';  // Straight wall
            return '#';  // Corner
        case 3: return '#';  // T-junction
        case 4: return '#';  // Cross
        default: return '#';
    }
}

std::string WallConnector::getUnicodeWall(bool n, bool s, bool e, bool w) {
    // Box-drawing characters for walls
    // Using single-line box drawing for now
    
    // All four directions
    if (n && s && e && w) return "┼";
    
    // Three directions (T-junctions)
    if (n && s && e) return "├";
    if (n && s && w) return "┤";
    if (n && e && w) return "┴";
    if (s && e && w) return "┬";
    
    // Two directions (corners and straight)
    if (n && s) return "│";  // Vertical
    if (e && w) return "─";  // Horizontal
    if (n && e) return "└";  // Bottom-left corner
    if (n && w) return "┘";  // Bottom-right corner
    if (s && e) return "┌";  // Top-left corner
    if (s && w) return "┐";  // Top-right corner
    
    // One direction (ends)
    if (n) return "╵";  // Up only
    if (s) return "╷";  // Down only
    if (e) return "╶";  // Right only
    if (w) return "╴";  // Left only
    
    // No connections (pillar)
    return "●";
}

char WallConnector::getWallChar(const Map& map, int x, int y) {
    if (map.getTile(x, y) != TileType::WALL) {
        return '#';  // Not a wall
    }
    
    bool n = hasWallNorth(map, x, y);
    bool s = hasWallSouth(map, x, y);
    bool e = hasWallEast(map, x, y);
    bool w = hasWallWest(map, x, y);
    
    if (unicode_enabled) {
        // For char return, we'll use ASCII fallback since Unicode needs string
        return getASCIIWall(n, s, e, w);
    } else {
        return getASCIIWall(n, s, e, w);
    }
}

std::string WallConnector::getWallString(const Map& map, int x, int y) {
    if (map.getTile(x, y) != TileType::WALL) {
        return "#";  // Not a wall
    }
    
    bool n = hasWallNorth(map, x, y);
    bool s = hasWallSouth(map, x, y);
    bool e = hasWallEast(map, x, y);
    bool w = hasWallWest(map, x, y);
    
    if (unicode_enabled) {
        return getUnicodeWall(n, s, e, w);
    } else {
        return std::string(1, getASCIIWall(n, s, e, w));
    }
}