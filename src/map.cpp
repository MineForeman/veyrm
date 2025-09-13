#include "map.h"
#include "color_scheme.h"
#include <algorithm>

// Define tile properties for each tile type
const std::map<TileType, TileProperties> Map::tileProperties = {
    {TileType::FLOOR,       {"·", Color::White,     Color::Black, true,  true,  false, "Stone Floor"}},
    {TileType::WALL,        {"█", Color::Yellow,    Color::Black, false, false, true,  "Stone Wall"}},
    {TileType::STAIRS_DOWN, {"▼", Color::Yellow,    Color::Black, true,  true,  false, "Stairs Down"}},
    {TileType::STAIRS_UP,   {"▲", Color::Yellow,    Color::Black, true,  true,  false, "Stairs Up"}},
    {TileType::DOOR_CLOSED, {"▦", Color::Yellow,    Color::Black, false, false, true,  "Closed Door"}},
    {TileType::DOOR_OPEN,   {"▢", Color::Yellow,    Color::Black, true,  true,  false, "Open Door"}},
    {TileType::WATER,       {"≈", Color::Cyan,      Color::Black, false, true,  false, "Water"}},
    {TileType::LAVA,        {"≈", Color::Red,       Color::Black, false, true,  false, "Lava"}},
    {TileType::VOID,        {" ", Color::Black,     Color::Black, false, false, false, "Void"}},
    {TileType::UNKNOWN,     {"?", Color::GrayDark,  Color::Black, false, false, false, "Unknown"}},
};

Map::Map(int w, int h) : width(w), height(h) {
    // Initialize tile grid
    tiles.resize(height);
    visible.resize(height);
    explored.resize(height);
    
    for (int y = 0; y < height; y++) {
        tiles[y].resize(width, TileType::VOID);
        visible[y].resize(width, false);
        explored[y].resize(width, false);
    }
}

TileType Map::getTile(int x, int y) const {
    if (!inBounds(x, y)) {
        return TileType::VOID;
    }
    return tiles[y][x];
}

TileType Map::getTile(const Point& pos) const {
    return getTile(pos.x, pos.y);
}

void Map::setTile(int x, int y, TileType type) {
    if (inBounds(x, y)) {
        tiles[y][x] = type;
    }
}

void Map::setTile(const Point& pos, TileType type) {
    setTile(pos.x, pos.y, type);
}

bool Map::isWalkable(int x, int y) const {
    if (!inBounds(x, y)) {
        return false;
    }
    TileType tile = getTile(x, y);
    auto it = tileProperties.find(tile);
    if (it != tileProperties.end()) {
        return it->second.walkable;
    }
    return false;
}

bool Map::isWalkable(const Point& pos) const {
    return isWalkable(pos.x, pos.y);
}

bool Map::isTransparent(int x, int y) const {
    if (!inBounds(x, y)) {
        return false;
    }
    TileType tile = getTile(x, y);
    auto it = tileProperties.find(tile);
    if (it != tileProperties.end()) {
        return it->second.transparent;
    }
    return false;
}

bool Map::isTransparent(const Point& pos) const {
    return isTransparent(pos.x, pos.y);
}

bool Map::inBounds(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool Map::inBounds(const Point& pos) const {
    return inBounds(pos.x, pos.y);
}

bool Map::isVisible(int x, int y) const {
    if (!inBounds(x, y)) {
        return false;
    }
    return visible[y][x];
}

void Map::setVisible(int x, int y, bool vis) {
    if (inBounds(x, y)) {
        visible[y][x] = vis;
        // If setting visible, also mark as explored
        if (vis) {
            explored[y][x] = true;
        }
    }
}

bool Map::isExplored(int x, int y) const {
    if (!inBounds(x, y)) {
        return false;
    }
    return explored[y][x];
}

void Map::setExplored(int x, int y, bool exp) {
    if (inBounds(x, y)) {
        explored[y][x] = exp;
    }
}

std::string Map::getGlyph(int x, int y) const {
    TileType tile = getTile(x, y);
    auto it = tileProperties.find(tile);
    if (it != tileProperties.end()) {
        return it->second.glyph;
    }
    return "?";
}

Color Map::getForeground(int x, int y) const {
    TileType tile = getTile(x, y);
    const auto& colors = ColorScheme::getCurrentColors();
    
    // Use color scheme instead of static properties
    switch (tile) {
        case TileType::WALL:
            return isVisible(x, y) ? colors.wall : colors.wall_memory;
        case TileType::FLOOR:
            return isVisible(x, y) ? colors.floor : colors.floor_memory;
        case TileType::VOID:
            return colors.void_tile;
        case TileType::STAIRS_DOWN:
        case TileType::STAIRS_UP:
            return Color::Yellow;  // Keep stairs always yellow
        case TileType::DOOR_CLOSED:
        case TileType::DOOR_OPEN:
            return Color::Yellow;  // Keep doors yellow
        default:
            return Color::White;
    }
}

Color Map::getBackground(int x, int y) const {
    TileType tile = getTile(x, y);
    auto it = tileProperties.find(tile);
    if (it != tileProperties.end()) {
        return it->second.background;
    }
    return Color::Black;
}

void Map::fill(TileType type) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tiles[y][x] = type;
        }
    }
}

void Map::createRoom(int x, int y, int w, int h) {
    // Create walls
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (inBounds(px, py)) {
                // Walls on edges, floor inside
                if (dx == 0 || dx == w - 1 || dy == 0 || dy == h - 1) {
                    setTile(px, py, TileType::WALL);
                } else {
                    setTile(px, py, TileType::FLOOR);
                }
            }
        }
    }
}

TileProperties Map::getTileProperties(TileType type) {
    auto it = tileProperties.find(type);
    if (it != tileProperties.end()) {
        return it->second;
    }
    // Return default properties for unknown tile
    return {" ", Color::White, Color::Black, false, false, false, "Unknown"};
}

void Map::createCorridor(const Point& start, const Point& end) {
    // Simple L-shaped corridor
    Point current = start;
    
    // Move horizontally first
    while (current.x != end.x) {
        setTile(current, TileType::FLOOR);
        current.x += (end.x > current.x) ? 1 : -1;
    }
    
    // Then move vertically
    while (current.y != end.y) {
        setTile(current, TileType::FLOOR);
        current.y += (end.y > current.y) ? 1 : -1;
    }
    
    // Set the end point
    setTile(end, TileType::FLOOR);
}

void Map::addRoom(const Room& room) {
    rooms.push_back(room);
}

Room* Map::getRoomAt(int x, int y) {
    for (auto& room : rooms) {
        if (room.contains(x, y)) {
            return &room;
        }
    }
    return nullptr;
}

const Room* Map::getRoomAt(int x, int y) const {
    for (const auto& room : rooms) {
        if (room.contains(x, y)) {
            return &room;
        }
    }
    return nullptr;
}