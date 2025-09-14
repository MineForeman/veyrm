/**
 * @file tile.h
 * @brief Tile type definitions and properties
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/screen/color.hpp>
#include <string>

using Color = ftxui::Color;

/**
 * @enum TileType
 * @brief Types of tiles that can exist in the game world
 *
 * Defines all possible tile types for map generation and world representation.
 * Each tile type has associated properties that determine gameplay mechanics
 * such as movement, visibility, and interaction possibilities.
 */
enum class TileType {
    // Basic terrain tiles
    FLOOR,          ///< '.' - Walkable floor surface
    WALL,           ///< '#' - Solid wall that blocks movement and sight
    STAIRS_DOWN,    ///< '>' - Stairs leading to deeper level
    STAIRS_UP,      ///< '<' - Stairs leading to shallower level

    // Door tiles
    DOOR_CLOSED,    ///< '+' - Closed door (blocks movement, not sight)
    DOOR_OPEN,      ///< '/' - Open door (allows movement and sight)

    // Special terrain tiles
    WATER,          ///< '~' - Water tile (affects movement)
    LAVA,           ///< '~' - Lava tile (damages entities)

    // Meta tiles
    VOID,           ///< ' ' - Out of bounds or ungenerated space
    UNKNOWN         ///< '?' - Unexplored tile (for saved memory)
};

/**
 * @struct TileProperties
 * @brief Properties that define tile appearance and behavior
 *
 * Contains all the data needed to render and simulate a tile type,
 * including visual representation, movement rules, and gameplay
 * mechanics.
 *
 * @see Map::getTileProperties()
 * @see ColorScheme
 */
struct TileProperties {
    std::string glyph;    ///< Display character (supports Unicode)
    Color foreground;     ///< Text/glyph color
    Color background;     ///< Background fill color
    bool walkable;        ///< True if entities can move through
    bool transparent;     ///< True if sight passes through (FOV)
    bool destructible;    ///< True if tile can be modified/destroyed
    std::string name;     ///< Human-readable name ("Stone Wall", etc.)
};