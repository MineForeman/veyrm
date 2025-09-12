#pragma once

#include "tile.h"
#include "point.h"
#include <vector>
#include <map>

class Map {
public:
    static constexpr int DEFAULT_WIDTH = 80;
    static constexpr int DEFAULT_HEIGHT = 24;
    
    Map(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);
    ~Map() = default;
    
    // Tile access
    TileType getTile(int x, int y) const;
    TileType getTile(const Point& pos) const;
    void setTile(int x, int y, TileType type);
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
    char getGlyph(int x, int y) const;
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
    
private:
    int width;
    int height;
    std::vector<std::vector<TileType>> tiles;
    std::vector<std::vector<bool>> visible;
    std::vector<std::vector<bool>> explored;
    
    // Tile properties lookup
    static const std::map<TileType, TileProperties> tileProperties;
};