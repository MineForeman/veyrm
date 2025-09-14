/**
 * @file fov.h
 * @brief Field of view calculation system for Veyrm
 * @author Veyrm Team
 * @date 2025
 */

#ifndef FOV_H
#define FOV_H

#include "point.h"
#include <vector>
#include <set>

class Map;

/**
 * @class FOV
 * @brief Field of view calculation using symmetric shadowcasting algorithm
 *
 * The FOV class implements symmetric shadowcasting to calculate what tiles
 * are visible from a given point on the map. This system handles line-of-sight
 * calculations for both the player's view and AI visibility checks.
 *
 * The implementation uses the recursive shadowcasting algorithm that divides
 * the field of view into 8 octants and calculates visibility for each.
 * This provides smooth, realistic visibility that respects opaque obstacles.
 *
 * @see Map
 * @see TileType
 * @see Point
 */
class FOV {
public:
    /// Default field of view radius in tiles
    static constexpr int DEFAULT_RADIUS = 10;

    /**
     * @brief Calculate field of view from a point using shadowcasting
     * @param map The map to calculate FOV on
     * @param origin The point from which to calculate visibility
     * @param radius Maximum visibility distance
     * @param visible 2D boolean array to store visibility results
     *
     * This method fills the visible array with true/false values indicating
     * which tiles can be seen from the origin point within the given radius.
     * The array dimensions must match the map dimensions.
     *
     * @see isVisible()
     * @see getVisibleTiles()
     */
    static void calculate(const Map& map, const Point& origin,
                         int radius, std::vector<std::vector<bool>>& visible);

    /**
     * @brief Check if a specific point is visible from an origin
     * @param map The map to check visibility on
     * @param origin The point from which to check visibility
     * @param target The point to check visibility to
     * @param maxDistance Maximum distance to consider (0 = unlimited)
     * @return true if target is visible from origin, false otherwise
     *
     * This method performs a line-of-sight check between two specific points
     * without calculating the entire field of view. Useful for AI checks.
     *
     * @see calculate()
     */
    static bool isVisible(const Map& map, const Point& origin,
                         const Point& target, int maxDistance);

    /**
     * @brief Get all visible tiles from an origin point
     * @param map The map to calculate visibility on
     * @param origin The point from which to calculate visibility
     * @param radius Maximum visibility distance
     * @return Set of all points visible from the origin
     *
     * This method returns a set containing all tiles that are visible
     * from the given origin point within the specified radius.
     *
     * @see calculate()
     * @see isVisible()
     */
    static std::set<Point> getVisibleTiles(const Map& map, const Point& origin, int radius);
    
private:
    /**
     * @brief Recursive shadowcasting implementation for one octant
     * @param map The map to cast shadows on
     * @param origin The center point of the field of view
     * @param radius Maximum casting distance
     * @param row Current row being processed (distance from origin)
     * @param start Start slope of the current shadow
     * @param end End slope of the current shadow
     * @param xx X component of octant transform matrix
     * @param xy Y component of octant transform matrix
     * @param yx X component of octant transform matrix
     * @param yy Y component of octant transform matrix
     * @param visible 2D array to mark visible tiles
     *
     * This is the core recursive shadowcasting algorithm that processes
     * one octant of the field of view. The transform matrix parameters
     * allow the same algorithm to work for all 8 octants.
     */
    static void castLight(const Map& map, const Point& origin, int radius,
                         int row, float start, float end,
                         int xx, int xy, int yx, int yy,
                         std::vector<std::vector<bool>>& visible);

    /**
     * @brief Check if a tile blocks line of sight
     * @param map The map to check
     * @param x X coordinate of the tile
     * @param y Y coordinate of the tile
     * @return true if the tile is opaque (blocks vision), false if transparent
     *
     * This method determines whether a tile blocks vision based on its type.
     * Walls and other solid obstacles are opaque, while floors are transparent.
     */
    static bool isOpaque(const Map& map, int x, int y);
};

#endif // FOV_H