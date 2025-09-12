#pragma once

#include <ftxui/screen/color.hpp>
#include <string>

using Color = ftxui::Color;

enum class TileType {
    // Basic tiles
    FLOOR,          // '.' - Walkable floor
    WALL,           // '#' - Solid wall
    STAIRS_DOWN,    // '>' - Stairs going down
    STAIRS_UP,      // '<' - Stairs going up
    
    // Doors (future)
    DOOR_CLOSED,    // '+' - Closed door
    DOOR_OPEN,      // '/' - Open door
    
    // Special tiles (future)
    WATER,          // '~' - Water tile
    LAVA,           // '~' - Lava tile (different color)
    
    // Meta tiles
    VOID,           // ' ' - Out of bounds/ungenerated
    UNKNOWN         // '?' - Not yet revealed
};

struct TileProperties {
    std::string glyph;    // Display character (supports Unicode)
    Color foreground;     // Text color
    Color background;     // Background color
    bool walkable;        // Can entities move through
    bool transparent;     // Can see through (for FOV)
    bool destructible;    // Can be destroyed/modified
    std::string name;     // Display name ("Stone Wall")
};