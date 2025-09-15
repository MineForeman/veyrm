/**
 * @file renderer.h
 * @brief Map and entity rendering system
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "point.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <vector>
#include <optional>

// Forward declarations
class Map;
class GameManager;

/**
 * @enum RenderLayer
 * @brief Rendering layer priorities for proper visual stacking
 */
enum class RenderLayer {
    TERRAIN = 0,    ///< Map tiles (floors, walls, doors)
    ITEMS = 1,      ///< Items on the ground
    CREATURES = 2,  ///< Monsters and NPCs
    PLAYER = 3,     ///< Player character (always on top of creatures)
    EFFECTS = 4,    ///< Visual effects and particles
    UI = 5          ///< UI overlays and cursors
};

/**
 * @struct RenderEntity
 * @brief Data for rendering a single entity with layering
 */
struct RenderEntity {
    Point position;             ///< World position
    std::string glyph;          ///< Display character(s)
    ftxui::Color foreground;    ///< Foreground color
    ftxui::Color background;    ///< Background color
    RenderLayer layer;          ///< Rendering layer
    int priority = 0;           ///< Priority within layer (higher = on top)
};

/**
 * @class MapRenderer
 * @brief Handles rendering of the game map and entities
 *
 * The MapRenderer manages the visual representation of the game world,
 * including terrain, entities, items, and effects. It supports viewport
 * management for large maps, layered rendering for proper visual stacking,
 * and various rendering options for debugging and visualization.
 *
 * Features:
 * - Viewport-based rendering for large maps
 * - Multi-layer rendering system
 * - Player-centered view with smooth scrolling
 * - Debug visualization options
 * - Coordinate transformation utilities
 * - Tile highlighting for cursor feedback
 *
 * @see Map
 * @see GameManager
 * @see ColorScheme
 */
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
    ftxui::Element renderItems(const GameManager& game);
    // renderEntities removed - using ECS RenderSystem integration
    ftxui::Element renderPlayer(const GameManager& game);
    ftxui::Element renderEffects();
    
    // Helper methods
    ftxui::Color getColorVariation(ftxui::Color base, int x, int y) const;
    std::string getTileVariant(const std::string& base_glyph, int x, int y) const;
};