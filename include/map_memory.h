#ifndef MAP_MEMORY_H
#define MAP_MEMORY_H

#include "map.h"
#include "point.h"
#include <vector>

class MapMemory {
private:
    int width;
    int height;
    std::vector<std::vector<bool>> explored;
    std::vector<std::vector<TileType>> remembered;
    std::vector<std::vector<bool>> currentlyVisible;
    
public:
    MapMemory(int width, int height);
    
    // Update visibility based on FOV calculation
    void updateVisibility(const Map& map, const std::vector<std::vector<bool>>& fov);
    
    // Query methods
    bool isExplored(int x, int y) const;
    bool isVisible(int x, int y) const;
    TileType getRemembered(int x, int y) const;
    
    // Get visibility for rendering
    enum class VisibilityState {
        UNKNOWN,    // Never seen
        REMEMBERED, // Seen before but not currently visible
        VISIBLE     // Currently visible
    };
    VisibilityState getVisibility(int x, int y) const;
    
    // Clear all memory (for new level)
    void forgetAll();
    
    // Bounds checking
    bool inBounds(int x, int y) const;
};

#endif // MAP_MEMORY_H