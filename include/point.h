#pragma once

#include <vector>
#include <cmath>

struct Point {
    int x;
    int y;
    
    // Constructors
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}
    
    // Operators
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    
    // Less than operator for use in std::set and std::map
    bool operator<(const Point& other) const {
        if (y != other.y) return y < other.y;
        return x < other.x;
    }
    
    Point operator+(const Point& other) const {
        return Point(x + other.x, y + other.y);
    }
    
    Point operator-(const Point& other) const {
        return Point(x - other.x, y - other.y);
    }
    
    // Utilities
    int distance(const Point& other) const {
        // Manhattan distance
        return std::abs(x - other.x) + std::abs(y - other.y);
    }
    
    double euclidean(const Point& other) const {
        // Euclidean distance
        int dx = x - other.x;
        int dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    std::vector<Point> neighbors() const;
    std::vector<Point> neighbors8() const;
    
    bool inBounds(int width, int height) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
};

// Common direction vectors
namespace Direction {
    const Point NORTH(0, -1);
    const Point SOUTH(0, 1);
    const Point EAST(1, 0);
    const Point WEST(-1, 0);
    const Point NORTHEAST(1, -1);
    const Point NORTHWEST(-1, -1);
    const Point SOUTHEAST(1, 1);
    const Point SOUTHWEST(-1, 1);
    
    const std::vector<Point> CARDINAL = {NORTH, SOUTH, EAST, WEST};
    const std::vector<Point> DIAGONAL = {NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST};
    const std::vector<Point> ALL = {NORTH, SOUTH, EAST, WEST, 
                                    NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST};
}