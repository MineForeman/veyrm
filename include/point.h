/**
 * @file point.h
 * @brief 2D coordinate and direction utilities
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <vector>
#include <cmath>

/**
 * @struct Point
 * @brief 2D coordinate point with utility functions
 *
 * The Point struct represents a 2D coordinate in the game world and provides
 * common geometric operations including distance calculations, neighbor finding,
 * bounds checking, and operator overloads for mathematical operations.
 *
 * Coordinate system:
 * - X-axis: left to right (0 to width-1)
 * - Y-axis: top to bottom (0 to height-1)
 *
 * @see Direction namespace for common movement vectors
 * @see Map for coordinate validation
 */
struct Point {
    int x; ///< X coordinate (horizontal position)
    int y; ///< Y coordinate (vertical position)

    // Constructors

    /** @brief Default constructor, creates point at origin (0,0) */
    Point() : x(0), y(0) {}

    /** @brief Construct point at specific coordinates */
    Point(int x, int y) : x(x), y(y) {}

    // Comparison operators

    /** @brief Equality comparison */
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    /** @brief Inequality comparison */
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    /** @brief Less than comparison for std::set and std::map usage */
    bool operator<(const Point& other) const {
        if (y != other.y) return y < other.y;
        return x < other.x;
    }

    // Arithmetic operators

    /** @brief Add two points (vector addition) */
    Point operator+(const Point& other) const {
        return Point(x + other.x, y + other.y);
    }

    /** @brief Subtract two points (vector subtraction) */
    Point operator-(const Point& other) const {
        return Point(x - other.x, y - other.y);
    }
    
    // Distance and geometry utilities

    /**
     * @brief Calculate Manhattan distance to another point
     * @param other Target point
     * @return Manhattan distance (sum of coordinate differences)
     */
    int distance(const Point& other) const {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

    /**
     * @brief Calculate Euclidean distance to another point
     * @param other Target point
     * @return Straight-line distance
     */
    double euclidean(const Point& other) const {
        int dx = x - other.x;
        int dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * @brief Get 4-directional neighbors (cardinal directions)
     * @return Vector of neighboring points (north, south, east, west)
     */
    std::vector<Point> neighbors() const;

    /**
     * @brief Get 8-directional neighbors (includes diagonals)
     * @return Vector of all adjacent points
     */
    std::vector<Point> neighbors8() const;

    /**
     * @brief Check if point is within rectangular bounds
     * @param width Rectangle width
     * @param height Rectangle height
     * @return true if point is inside [0, width) x [0, height)
     */
    bool inBounds(int width, int height) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
};

/**
 * @namespace Direction
 * @brief Common direction vectors for movement and pathfinding
 *
 * Provides standard direction constants and collections for game movement.
 * Uses screen coordinate system where Y increases downward.
 */
namespace Direction {
    const Point NORTH(0, -1);      ///< Move up (decrease Y)
    const Point SOUTH(0, 1);       ///< Move down (increase Y)
    const Point EAST(1, 0);        ///< Move right (increase X)
    const Point WEST(-1, 0);       ///< Move left (decrease X)
    const Point NORTHEAST(1, -1);  ///< Move diagonally up-right
    const Point NORTHWEST(-1, -1); ///< Move diagonally up-left
    const Point SOUTHEAST(1, 1);   ///< Move diagonally down-right
    const Point SOUTHWEST(-1, 1);  ///< Move diagonally down-left

    /// Four cardinal directions (no diagonals)
    const std::vector<Point> CARDINAL = {NORTH, SOUTH, EAST, WEST};
    /// Four diagonal directions
    const std::vector<Point> DIAGONAL = {NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST};
    /// All eight directions
    const std::vector<Point> ALL = {NORTH, SOUTH, EAST, WEST,
                                    NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST};
}