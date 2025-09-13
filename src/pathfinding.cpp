#include "pathfinding.h"
#include "map.h"
#include <algorithm>
#include <cmath>

const Point Pathfinding::DIRECTIONS_8[8] = {
    {0, -1},   // N
    {1, -1},   // NE
    {1, 0},    // E
    {1, 1},    // SE
    {0, 1},    // S
    {-1, 1},   // SW
    {-1, 0},   // W
    {-1, -1}   // NW
};

const Point Pathfinding::DIRECTIONS_4[4] = {
    {0, -1},   // N
    {1, 0},    // E
    {0, 1},    // S
    {-1, 0}    // W
};

std::vector<Point> Pathfinding::findPath(const Point& start, const Point& goal, const Map& map, bool allow_diagonals) {
    if (start == goal) {
        return {goal};
    }

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
    std::map<Point, float> g_scores;
    std::map<Point, Point> came_from;

    open_set.push({start, 0, heuristic(start, goal)});
    g_scores[start] = 0;

    while (!open_set.empty()) {
        Node current = open_set.top();
        open_set.pop();

        if (current.pos == goal) {
            return reconstructPath(came_from, goal);
        }

        if (g_scores.count(current.pos) && current.g_cost > g_scores[current.pos]) {
            continue;
        }

        for (const Point& neighbor : getNeighbors(current.pos, map, allow_diagonals)) {
            float tentative_g = current.g_cost;

            bool is_diagonal = (neighbor.x != current.pos.x) && (neighbor.y != current.pos.y);
            tentative_g += is_diagonal ? 1.41f : 1.0f;

            if (!g_scores.count(neighbor) || tentative_g < g_scores[neighbor]) {
                g_scores[neighbor] = tentative_g;
                came_from[neighbor] = current.pos;
                open_set.push({neighbor, tentative_g, tentative_g + heuristic(neighbor, goal)});
            }
        }
    }

    return {};
}

bool Pathfinding::hasLineOfSight(const Point& from, const Point& to, const Map& map) {
    int dx = std::abs(to.x - from.x);
    int dy = std::abs(to.y - from.y);
    int x = from.x;
    int y = from.y;
    int x_inc = (from.x < to.x) ? 1 : -1;
    int y_inc = (from.y < to.y) ? 1 : -1;
    int error = dx - dy;

    while (x != to.x || y != to.y) {
        if (!map.inBounds(x, y)) {
            return false;
        }

        auto props = Map::getTileProperties(map.getTile(x, y));
        if (!props.transparent) {
            return false;
        }

        int error2 = error * 2;
        if (error2 > -dy) {
            error -= dy;
            x += x_inc;
        }
        if (error2 < dx) {
            error += dx;
            y += y_inc;
        }
    }

    return true;
}

float Pathfinding::getDistance(const Point& a, const Point& b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

float Pathfinding::heuristic(const Point& a, const Point& b) {
    return getDistance(a, b);
}

std::vector<Point> Pathfinding::getNeighbors(const Point& pos, const Map& map, bool allow_diagonals) {
    std::vector<Point> neighbors;

    const Point* directions = allow_diagonals ? DIRECTIONS_8 : DIRECTIONS_4;
    int num_dirs = allow_diagonals ? 8 : 4;

    for (int i = 0; i < num_dirs; ++i) {
        Point new_pos = pos + directions[i];

        if (!map.inBounds(new_pos.x, new_pos.y)) {
            continue;
        }

        auto props = Map::getTileProperties(map.getTile(new_pos.x, new_pos.y));
        if (props.walkable) {
            neighbors.push_back(new_pos);
        }
    }

    return neighbors;
}

std::vector<Point> Pathfinding::reconstructPath(const std::map<Point, Point>& came_from, const Point& current) {
    std::vector<Point> path;
    Point cur = current;

    while (came_from.count(cur)) {
        path.push_back(cur);
        cur = came_from.at(cur);
    }

    std::reverse(path.begin(), path.end());
    return path;
}