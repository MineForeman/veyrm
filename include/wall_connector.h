#pragma once

#include "point.h"
#include <string>

class Map;

// Wall connection system for better visual appearance
class WallConnector {
public:
    // Get the appropriate wall character based on neighboring walls
    static std::string getWallChar(const Map& map, int x, int y);
    static std::string getWallString(const Map& map, int x, int y);
    
    // Check if using Unicode mode
    static bool isUnicodeEnabled();
    static void setUnicodeEnabled(bool enabled);
    
private:
    static bool unicode_enabled;
    
    // Neighbor checking
    static bool hasWallNorth(const Map& map, int x, int y);
    static bool hasWallSouth(const Map& map, int x, int y);
    static bool hasWallEast(const Map& map, int x, int y);
    static bool hasWallWest(const Map& map, int x, int y);
    
    // Get ASCII wall character
    static char getASCIIWall(bool n, bool s, bool e, bool w);
    
    // Get Unicode wall character
    static std::string getUnicodeWall(bool n, bool s, bool e, bool w);
};