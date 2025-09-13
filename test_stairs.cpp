#include <iostream>
#include "map.h"
#include "map_generator.h"

int main() {
    Map map(198, 66);
    MapGenerator::generateProceduralDungeon(map, 42);
    
    bool has_stairs = false;
    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            if (map.getTile(x, y) == TileType::STAIRS_DOWN) {
                has_stairs = true;
                std::cout << "Found stairs at: " << x << ", " << y << std::endl;
            }
        }
    }
    
    if (!has_stairs) {
        std::cout << "No stairs found!" << std::endl;
    }
    
    return 0;
}
