/**
 * @file position_component.h
 * @brief Position and movement component for entities
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include "../point.h"

namespace ecs {

/**
 * @class PositionComponent
 * @brief Manages entity position and movement history
 *
 * This component stores an entity's current position and
 * previous position for movement interpolation or undo.
 */
class PositionComponent : public Component<PositionComponent> {
public:
    /**
     * @brief Construct position component at coordinates
     * @param x Initial X coordinate
     * @param y Initial Y coordinate
     */
    PositionComponent(int x = 0, int y = 0)
        : position(x, y), previous_position(x, y) {}

    /**
     * @brief Construct position component from Point
     * @param pos Initial position
     */
    explicit PositionComponent(const Point& pos)
        : position(pos), previous_position(pos) {}

    ComponentType getType() const override {
        return ComponentType::POSITION;
    }

    std::string getTypeName() const override {
        return "PositionComponent";
    }

    /**
     * @brief Move to new position
     * @param new_pos Target position
     */
    void moveTo(const Point& new_pos) {
        previous_position = position;
        position = new_pos;
    }

    /**
     * @brief Move to new coordinates
     * @param x Target X coordinate
     * @param y Target Y coordinate
     */
    void moveTo(int x, int y) {
        moveTo(Point(x, y));
    }

    /**
     * @brief Move by relative offset
     * @param dx X-axis delta
     * @param dy Y-axis delta
     */
    void moveBy(int dx, int dy) {
        moveTo(position.x + dx, position.y + dy);
    }

    /**
     * @brief Get current position
     * @return Current position as Point
     */
    const Point& getPosition() const { return position; }

    /**
     * @brief Get previous position
     * @return Previous position as Point
     */
    const Point& getPreviousPosition() const { return previous_position; }

    /**
     * @brief Check if at specific coordinates
     * @param x X coordinate to check
     * @param y Y coordinate to check
     * @return true if at the specified position
     */
    bool isAt(int x, int y) const {
        return position.x == x && position.y == y;
    }

    /**
     * @brief Calculate distance to another position
     * @param other Target position
     * @return Euclidean distance
     */
    double distanceTo(const Point& other) const {
        return position.distance(other);
    }

public:
    // Public data for easy access (component as data)
    Point position;           ///< Current position
    Point previous_position;  ///< Previous position for undo/animation
};

} // namespace ecs