#pragma once

#include "point.h"
#include <vector>
#include <queue>
#include <map>
#include <functional>

class Map;

class Pathfinding {
public:
    static std::vector<Point> findPath(const Point& start, const Point& goal, const Map& map, bool allow_diagonals = true);

    static bool hasLineOfSight(const Point& from, const Point& to, const Map& map);

    static float getDistance(const Point& a, const Point& b);

    static const Point DIRECTIONS_8[8];
    static const Point DIRECTIONS_4[4];

private:
    struct Node {
        Point pos;
        float g_cost;
        float f_cost;

        bool operator>(const Node& other) const {
            return f_cost > other.f_cost;
        }
    };

    static float heuristic(const Point& a, const Point& b);
    static std::vector<Point> getNeighbors(const Point& pos, const Map& map, bool allow_diagonals);
    static std::vector<Point> reconstructPath(const std::map<Point, Point>& came_from, const Point& current);
};