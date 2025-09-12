#include "point.h"

std::vector<Point> Point::neighbors() const {
    return {
        Point(x, y - 1),  // North
        Point(x, y + 1),  // South
        Point(x + 1, y),  // East
        Point(x - 1, y)   // West
    };
}

std::vector<Point> Point::neighbors8() const {
    return {
        Point(x, y - 1),      // North
        Point(x, y + 1),      // South
        Point(x + 1, y),      // East
        Point(x - 1, y),      // West
        Point(x + 1, y - 1),  // Northeast
        Point(x - 1, y - 1),  // Northwest
        Point(x + 1, y + 1),  // Southeast
        Point(x - 1, y + 1)   // Southwest
    };
}