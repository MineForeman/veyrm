/**
 * @file map.h
 * @brief Tile-based map system for Veyrm
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "tile.h"
#include "point.h"
#include "room.h"
#include <vector>
#include <map>

/**
 * @class Map
 * @brief Manages the tile-based game map
 *
 * The Map class represents the game world as a 2D grid of tiles.
 * It handles tile storage, visibility tracking, exploration state,
 * and provides methods for querying tile properties.
 *
 * @note Uses classic Angband dimensions (198x66) by default
 * @see TileType
 * @see MapGenerator
 * @see FOV
 */
class Map {
public:
    /// Default map width (classic Angband dimension)
    static constexpr int DEFAULT_WIDTH = 198;
    /// Default map height (classic Angband dimension)
    static constexpr int DEFAULT_HEIGHT = 66;

    /**
     * @brief Construct a new Map
     * @param width Map width in tiles (default: 198)
     * @param height Map height in tiles (default: 66)
     */
    Map(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);

    /// Default destructor
    ~Map() = default;

    // Tile access

    /**
     * @brief Get the tile type at a position
     * @param x X coordinate
     * @param y Y coordinate
     * @return TileType at the position, or WALL if out of bounds
     */
    TileType getTile(int x, int y) const;

    /**
     * @brief Get the tile type at a position
     * @param pos Position to query
     * @return TileType at the position, or WALL if out of bounds
     */
    TileType getTile(const Point& pos) const;

    /**
     * @brief Set the tile type at a position
     * @param x X coordinate
     * @param y Y coordinate
     * @param type New tile type
     * @warning Does nothing if position is out of bounds
     */
    void setTile(int x, int y, TileType type);

    /**
     * @brief Set the tile type at a position
     * @param pos Position to modify
     * @param type New tile type
     * @warning Does nothing if position is out of bounds
     */
    void setTile(const Point& pos, TileType type);
    
    // Properties
    bool isWalkable(int x, int y) const;
    bool isWalkable(const Point& pos) const;
    bool isTransparent(int x, int y) const;
    bool isTransparent(const Point& pos) const;
    bool inBounds(int x, int y) const;
    bool inBounds(const Point& pos) const;
    
    // Visibility (for future FOV)
    bool isVisible(int x, int y) const;
    void setVisible(int x, int y, bool visible);
    bool isExplored(int x, int y) const;
    void setExplored(int x, int y, bool explored);
    
    // Rendering
    std::string getGlyph(int x, int y) const;
    ftxui::Color getForeground(int x, int y) const;
    ftxui::Color getBackground(int x, int y) const;
    
    // Map generation helpers
    void fill(TileType type);
    void createRoom(int x, int y, int width, int height);
    void createCorridor(const Point& start, const Point& end);
    
    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Get tile properties for a given tile type
    static TileProperties getTileProperties(TileType type);
    
    // Room management
    void addRoom(const Room& room);
    Room* getRoomAt(int x, int y);
    const Room* getRoomAt(int x, int y) const;
    Room* getRoomAt(const Point& pos) { return getRoomAt(pos.x, pos.y); }
    const Room* getRoomAt(const Point& pos) const { return getRoomAt(pos.x, pos.y); }
    const std::vector<Room>& getRooms() const { return rooms; }
    void clearRooms() { rooms.clear(); }
    
private:
    int width;
    int height;
    std::vector<std::vector<TileType>> tiles;
    std::vector<std::vector<bool>> visible;
    std::vector<std::vector<bool>> explored;
    std::vector<Room> rooms;  // Store all rooms in the map
    
    // Tile properties lookup
    static const std::map<TileType, TileProperties> tileProperties;
};