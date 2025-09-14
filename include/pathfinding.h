/**
 * @file pathfinding.h
 * @brief A* pathfinding and line-of-sight algorithms
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "point.h"
#include <vector>
#include <queue>
#include <map>
#include <functional>

class Map;

/**
 * @class Pathfinding
 * @brief Static utilities for pathfinding and spatial analysis
 *
 * The Pathfinding class provides A* pathfinding algorithms and line-of-sight
 * calculations for AI movement and game mechanics. It supports both 4-directional
 * and 8-directional movement with obstacle avoidance.
 *
 * Algorithms implemented:
 * - A* pathfinding with configurable diagonal movement
 * - Bresenham line-of-sight calculation
 * - Distance calculations (Manhattan and Euclidean)
 * - Neighbor enumeration for movement options
 *
 * @see MonsterAI
 * @see FOV
 * @see Point
 */
class Pathfinding {
public:
    /**
     * @brief Find optimal path using A* algorithm
     * @param start Starting position
     * @param goal Target position
     * @param map Map to navigate
     * @param allow_diagonals Allow diagonal movement (default: true)
     * @return Vector of points forming path (empty if no path found)
     */
    static std::vector<Point> findPath(const Point& start, const Point& goal, const Map& map, bool allow_diagonals = true);

    /**
     * @brief Check if there's unobstructed line of sight between points
     * @param from Source position
     * @param to Target position
     * @param map Map to check against
     * @return true if line of sight exists
     */
    static bool hasLineOfSight(const Point& from, const Point& to, const Map& map);

    /**
     * @brief Calculate Euclidean distance between points
     * @param a First point
     * @param b Second point
     * @return Straight-line distance
     */
    static float getDistance(const Point& a, const Point& b);

    /// Eight-directional movement vectors (includes diagonals)
    static const Point DIRECTIONS_8[8];
    /// Four-directional movement vectors (cardinal only)
    static const Point DIRECTIONS_4[4];

private:
    /**
     * @struct Node
     * @brief A* algorithm node for pathfinding
     */
    struct Node {
        Point pos;        ///< Position of this node
        float g_cost;     ///< Distance from start to this node
        float f_cost;     ///< Total estimated cost (g_cost + heuristic)

        /** @brief Priority queue comparison (lower f_cost has higher priority) */
        bool operator>(const Node& other) const {
            return f_cost > other.f_cost;
        }
    };

    /**
     * @brief Calculate heuristic distance for A*
     * @param a Starting point
     * @param b Target point
     * @return Heuristic cost estimate
     */
    static float heuristic(const Point& a, const Point& b);

    /**
     * @brief Get walkable neighbor positions
     * @param pos Current position
     * @param map Map to check walkability
     * @param allow_diagonals Include diagonal neighbors
     * @return Vector of walkable neighbor positions
     */
    static std::vector<Point> getNeighbors(const Point& pos, const Map& map, bool allow_diagonals);

    /**
     * @brief Reconstruct path from A* came_from map
     * @param came_from Map of position -> previous position
     * @param current Final position reached
     * @return Complete path from start to goal
     */
    static std::vector<Point> reconstructPath(const std::map<Point, Point>& came_from, const Point& current);
};