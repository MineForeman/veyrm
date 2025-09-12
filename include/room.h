#pragma once

#include "point.h"
#include <vector>

class Room {
public:
    int x, y;          // Top-left corner
    int width, height; // Dimensions
    
    // Room types for future expansion
    enum class RoomType {
        NORMAL,
        ENTRANCE,    // Starting room
        TREASURE,    // Loot room
        BOSS,        // Boss encounter
        CORRIDOR,    // Hallway connection
        SPECIAL      // Puzzle/special event
    };
    
    RoomType type = RoomType::NORMAL;
    
    // Constructor
    Room(int x, int y, int w, int h, RoomType t = RoomType::NORMAL);
    
    // Get room center for corridor connections
    Point center() const;
    
    // Check if this room overlaps with another
    bool overlaps(const Room& other, int padding = 0) const;
    
    // Check if a point is inside this room
    bool contains(int px, int py) const;
    bool contains(const Point& p) const { return contains(p.x, p.y); }
    
    // Get room boundaries
    int left() const { return x; }
    int right() const { return x + width - 1; }
    int top() const { return y; }
    int bottom() const { return y + height - 1; }
    
    // Get room area
    int area() const { return width * height; }
    
    // Get perimeter points (for door placement)
    std::vector<Point> getPerimeter() const;
    
    // Get all floor tiles (interior points)
    std::vector<Point> getFloorTiles() const;
    
    // Check if room is valid
    bool isValid() const;
};