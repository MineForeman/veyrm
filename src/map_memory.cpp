#include "map_memory.h"

MapMemory::MapMemory(int width, int height) 
    : width(width), height(height) {
    explored.resize(height, std::vector<bool>(width, false));
    remembered.resize(height, std::vector<TileType>(width, TileType::VOID));
    currentlyVisible.resize(height, std::vector<bool>(width, false));
}

void MapMemory::updateVisibility(const Map& map, const std::vector<std::vector<bool>>& fov) {
    // Update currently visible
    currentlyVisible = fov;
    
    // Update explored and remembered for visible tiles
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (fov[y][x]) {
                explored[y][x] = true;
                remembered[y][x] = map.getTile(x, y);
            }
        }
    }
}

bool MapMemory::isExplored(int x, int y) const {
    if (!inBounds(x, y)) return false;
    return explored[y][x];
}

bool MapMemory::isVisible(int x, int y) const {
    if (!inBounds(x, y)) return false;
    return currentlyVisible[y][x];
}

TileType MapMemory::getRemembered(int x, int y) const {
    if (!inBounds(x, y)) return TileType::VOID;
    return remembered[y][x];
}

MapMemory::VisibilityState MapMemory::getVisibility(int x, int y) const {
    if (!inBounds(x, y)) return VisibilityState::UNKNOWN;
    
    if (currentlyVisible[y][x]) {
        return VisibilityState::VISIBLE;
    } else if (explored[y][x]) {
        return VisibilityState::REMEMBERED;
    } else {
        return VisibilityState::UNKNOWN;
    }
}

void MapMemory::forgetAll() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            explored[y][x] = false;
            remembered[y][x] = TileType::VOID;
            currentlyVisible[y][x] = false;
        }
    }
}

bool MapMemory::inBounds(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}