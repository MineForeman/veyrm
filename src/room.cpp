#include "room.h"
#include <algorithm>

Room::Room(int x, int y, int w, int h, RoomType t, bool isLit)
    : x(x), y(y), width(w), height(h), type(t), lit(isLit) {
}

Point Room::center() const {
    return Point(x + width / 2, y + height / 2);
}

bool Room::overlaps(const Room& other, int padding) const {
    // Expand both rooms by padding on all sides
    int left1 = x - padding;
    int right1 = x + width - 1 + padding;
    int top1 = y - padding;
    int bottom1 = y + height - 1 + padding;
    
    int left2 = other.x - padding;
    int right2 = other.x + other.width - 1 + padding;
    int top2 = other.y - padding;
    int bottom2 = other.y + other.height - 1 + padding;
    
    // Check for non-overlap conditions
    // Rooms don't overlap if one is completely to the left, right, above, or below the other
    // Touching at edges/corners is not considered overlapping
    if (left1 > right2 || left2 > right1) return false;
    if (top1 > bottom2 || top2 > bottom1) return false;
    
    // Rooms overlap
    return true;
}

bool Room::contains(int px, int py) const {
    return px >= x && px < x + width &&
           py >= y && py < y + height;
}

std::vector<Point> Room::getPerimeter() const {
    std::vector<Point> perimeter;
    
    // Top and bottom edges
    for (int px = x; px < x + width; ++px) {
        perimeter.push_back(Point(px, y));                    // Top
        perimeter.push_back(Point(px, y + height - 1));       // Bottom
    }
    
    // Left and right edges (excluding corners to avoid duplicates)
    for (int py = y + 1; py < y + height - 1; ++py) {
        perimeter.push_back(Point(x, py));                    // Left
        perimeter.push_back(Point(x + width - 1, py));        // Right
    }
    
    return perimeter;
}

std::vector<Point> Room::getFloorTiles() const {
    std::vector<Point> floor;
    
    // Get all interior points (not including walls)
    for (int py = y + 1; py < y + height - 1; ++py) {
        for (int px = x + 1; px < x + width - 1; ++px) {
            floor.push_back(Point(px, py));
        }
    }
    
    return floor;
}

bool Room::isValid() const {
    // Room must have minimum size of 3x3 to have at least 1 floor tile
    return width >= 3 && height >= 3;
}