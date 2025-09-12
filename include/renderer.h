#pragma once

#include "point.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <vector>
#include <optional>

// Forward declarations
class Map;
class GameManager;

enum class RenderLayer {
    TERRAIN = 0,
    ITEMS = 1,
    CREATURES = 2,
    PLAYER = 3,
    EFFECTS = 4,
    UI = 5
};

struct RenderEntity {
    Point position;
    char glyph;
    ftxui::Color foreground;
    ftxui::Color background;
    RenderLayer layer;
    int priority = 0;
};

class MapRenderer {
public:
    MapRenderer(int viewport_width = 80, int viewport_height = 24);
    ~MapRenderer() = default;
    
    // Main render method
    ftxui::Element render(const Map& map, const GameManager& game);
    
    // Viewport management
    void setViewport(int width, int height);
    void centerOn(int x, int y);
    void centerOn(const Point& pos);
    Point getViewportOffset() const { return viewport_offset; }
    
    // Render options
    void setShowGrid(bool show) { show_grid = show; }
    void setShowCoordinates(bool show) { show_coordinates = show; }
    void setHighlightTile(const Point& pos) { highlight_pos = pos; }
    void clearHighlight() { highlight_pos = Point(-1, -1); }
    
    // Viewport queries
    bool isInViewport(int x, int y) const;
    bool isInViewport(const Point& pos) const;
    Point mapToScreen(int map_x, int map_y) const;
    Point screenToMap(int screen_x, int screen_y) const;
    
private:
    int viewport_width;
    int viewport_height;
    Point viewport_offset;
    Point highlight_pos;
    bool show_grid;
    bool show_coordinates;
    
    // Layer rendering
    ftxui::Element renderTerrain(const Map& map);
    ftxui::Element renderTerrainWithPlayer(const Map& map, const GameManager& game);
    ftxui::Element renderEntities(const GameManager& game);
    ftxui::Element renderPlayer(const GameManager& game);
    ftxui::Element renderEffects();
    
    // Helper methods
    ftxui::Color getColorVariation(ftxui::Color base, int x, int y) const;
    char getTileVariant(char base_glyph, int x, int y) const;
};