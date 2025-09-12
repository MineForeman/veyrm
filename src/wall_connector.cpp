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

char WallConnector::getASCIIWall(bool /*n*/, bool /*s*/, bool /*e*/, bool /*w*/) {
    // Always return '#' for ASCII mode (can't use Unicode in char)
    return '#';
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

std::string WallConnector::getWallChar(const Map& map, int x, int y) {
    if (map.getTile(x, y) != TileType::WALL) {
        return "█";  // Not a wall but return block anyway
    }
    
    // Always return the block character for walls
    return "█";
}

std::string WallConnector::getWallString(const Map& map, int x, int y) {
    if (map.getTile(x, y) != TileType::WALL) {
        return "█";  // Not a wall but return block anyway
    }
    
    // Always return the block character for walls
    return "█";
}