/**
 * @file room.h
 * @brief Room structure for dungeon map generation
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "point.h"
#include <vector>

/**
 * @class Room
 * @brief Represents a rectangular room in the dungeon
 *
 * The Room class defines rectangular areas within the dungeon map that serve
 * as the primary spaces for gameplay. Rooms are connected by corridors and
 * can have different types and properties that affect gameplay mechanics.
 *
 * Room features:
 * - Rectangular geometry with position and dimensions
 * - Type system for different room purposes
 * - Lighting system (Angband-style lit rooms)
 * - Overlap detection for generation
 * - Door placement on perimeter
 * - Interior floor tile enumeration
 *
 * @see MapGenerator
 * @see Map
 * @see Point
 */
class Room {
public:
    int x, y;          ///< Top-left corner coordinates
    int width, height; ///< Room dimensions

    /**
     * @enum RoomType
     * @brief Different types of rooms with specific purposes
     */
    enum class RoomType {
        NORMAL,      ///< Standard room with random contents
        ENTRANCE,    ///< Starting room where player begins
        TREASURE,    ///< Room with increased loot generation
        BOSS,        ///< Room designed for boss encounters
        CORRIDOR,    ///< Wide hallway or junction
        SPECIAL      ///< Puzzle rooms or special events
    };

    RoomType type = RoomType::NORMAL; ///< Room's functional type
    bool lit = false;                 ///< Angband-style lighting (entire room visible when entered)
    
    /**
     * @brief Construct a new room
     * @param x Left edge X coordinate
     * @param y Top edge Y coordinate
     * @param w Room width
     * @param h Room height
     * @param t Room type (default: NORMAL)
     * @param isLit Whether room is lit (default: false)
     */
    Room(int x, int y, int w, int h, RoomType t = RoomType::NORMAL, bool isLit = false);

    /**
     * @brief Get room center point
     * @return Point at the geometric center of the room
     * @note Used for corridor connections and pathfinding
     */
    Point center() const;

    /**
     * @brief Check if this room overlaps with another
     * @param other Room to check overlap with
     * @param padding Minimum separation distance (default: 0)
     * @return true if rooms overlap (considering padding)
     */
    bool overlaps(const Room& other, int padding = 0) const;

    /**
     * @brief Check if coordinates are inside this room
     * @param px X coordinate to test
     * @param py Y coordinate to test
     * @return true if point is inside room boundaries
     */
    bool contains(int px, int py) const;

    /** @brief Check if point is inside this room */
    bool contains(const Point& p) const { return contains(p.x, p.y); }
    
    // Room boundary accessors

    /** @brief Get left boundary X coordinate @return Left edge X */
    int left() const { return x; }

    /** @brief Get right boundary X coordinate @return Right edge X */
    int right() const { return x + width - 1; }

    /** @brief Get top boundary Y coordinate @return Top edge Y */
    int top() const { return y; }

    /** @brief Get bottom boundary Y coordinate @return Bottom edge Y */
    int bottom() const { return y + height - 1; }

    /** @brief Get room area @return Total area in tiles */
    int area() const { return width * height; }

    /**
     * @brief Get room perimeter points
     * @return Vector of points on room border (for door placement)
     */
    std::vector<Point> getPerimeter() const;

    /**
     * @brief Get all interior floor points
     * @return Vector of all floor tiles within the room
     */
    std::vector<Point> getFloorTiles() const;

    /**
     * @brief Check if room has valid dimensions
     * @return true if room has positive width and height
     */
    bool isValid() const;

    // Lighting accessors

    /** @brief Check if room is lit @return true if room has lighting */
    bool isLit() const { return lit; }

    /** @brief Set room lighting state @param value New lighting state */
    void setLit(bool value) { lit = value; }
};