#ifndef FOV_H
#define FOV_H

#include "point.h"
#include <vector>
#include <set>

class Map;

class FOV {
public:
    static constexpr int DEFAULT_RADIUS = 10;
    
    // Calculate FOV from a point
    static void calculate(const Map& map, const Point& origin, 
                         int radius, std::vector<std::vector<bool>>& visible);
    
    // Check if point is visible from origin
    static bool isVisible(const Map& map, const Point& origin, 
                         const Point& target, int maxDistance);
    
    // Get all visible points from origin
    static std::set<Point> getVisibleTiles(const Map& map, const Point& origin, int radius);
    
private:
    // Shadowcasting implementation for one octant
    static void castLight(const Map& map, const Point& origin, int radius,
                         int row, float start, float end,
                         int xx, int xy, int yx, int yy,
                         std::vector<std::vector<bool>>& visible);
    
    // Check if a tile blocks vision
    static bool isOpaque(const Map& map, int x, int y);
};

#endif // FOV_H