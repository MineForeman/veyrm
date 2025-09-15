#include "renderer.h"
#include "map.h"
#include "game_state.h"
#include "color_scheme.h"
#include "entity_manager.h"
#include "item_manager.h"
#include "item.h"
#include "ecs/game_world.h"
#include "ecs/render_system.h"
#include <ftxui/dom/elements.hpp>
#include <algorithm>

using namespace ftxui;

MapRenderer::MapRenderer(int vw, int vh)
    : viewport_width(vw),
      viewport_height(vh),
      viewport_offset(0, 0),
      highlight_pos(-1, -1),
      show_grid(false),
      show_coordinates(false) {
}

void MapRenderer::setViewport(int width, int height) {
    viewport_width = width;
    viewport_height = height;
}

void MapRenderer::centerOn(int x, int y) {
    viewport_offset.x = x - viewport_width / 2;
    viewport_offset.y = y - viewport_height / 2;
}

void MapRenderer::centerOn(const Point& pos) {
    centerOn(pos.x, pos.y);
}

bool MapRenderer::isInViewport(int x, int y) const {
    return x >= viewport_offset.x && 
           x < viewport_offset.x + viewport_width &&
           y >= viewport_offset.y && 
           y < viewport_offset.y + viewport_height;
}

bool MapRenderer::isInViewport(const Point& pos) const {
    return isInViewport(pos.x, pos.y);
}

Point MapRenderer::mapToScreen(int map_x, int map_y) const {
    return Point(map_x - viewport_offset.x, map_y - viewport_offset.y);
}

Point MapRenderer::screenToMap(int screen_x, int screen_y) const {
    return Point(screen_x + viewport_offset.x, screen_y + viewport_offset.y);
}

Color MapRenderer::getColorVariation(Color base, [[maybe_unused]] int x, [[maybe_unused]] int y) const {
    // Simple color variation based on position
    // This creates subtle variations in tile colors for visual interest
    // int seed = (x * 7 + y * 13) % 100;  // For future use
    
    // For now, just return the base color
    // In future, we could adjust brightness slightly
    return base;
}

std::string MapRenderer::getTileVariant(const std::string& base_glyph, int /*x*/, int /*y*/) const {
    // For walls, always use the block character
    if (base_glyph == "█") {
        return "█";  // Always solid block for walls
    }
    
    // For floors, keep the middle dot consistent
    if (base_glyph == "·") {
        return "·";  // Keep the middle dot consistent
    }
    
    return base_glyph;
}

Element MapRenderer::renderTerrainWithPlayer(const Map& map, const GameManager& game) {
    std::vector<Element> rows;
    Point player_screen = mapToScreen(game.player_x, game.player_y);
    
    // Get entities from ECS RenderSystem
    std::vector<std::vector<std::string>> ecs_entity_grid;
    auto* ecs_world = const_cast<ecs::GameWorld*>(game.getECSWorld());
    if (ecs_world) {
        ecs::RenderSystem* render_system = ecs_world->getRenderSystem();
        if (render_system) {
            ecs_entity_grid = render_system->renderToGrid(viewport_width, viewport_height,
                                                         viewport_offset.x, viewport_offset.y);
        }
    }

    // Get items for rendering
    const auto* item_manager = game.getItemManager();
    std::vector<const Item*> all_items;
    if (item_manager) {
        all_items = item_manager->getAllItems();
    }
    
    for (int screen_y = 0; screen_y < viewport_height; screen_y++) {
        std::vector<Element> row_elements;
        
        for (int screen_x = 0; screen_x < viewport_width; screen_x++) {
            Point map_pos = screenToMap(screen_x, screen_y);
            
            // Check if this is the player position
            if (screen_x == player_screen.x && screen_y == player_screen.y && 
                isInViewport(game.player_x, game.player_y)) {
                const auto& colors = ColorScheme::getCurrentColors();
                row_elements.push_back(text("@") | color(colors.player) | bold);
                continue;
            }
            
            // Check for ECS entities at this position
            bool entity_rendered = false;
            if (map.isVisible(map_pos.x, map_pos.y) && !ecs_entity_grid.empty()) {
                if (screen_y < (int)ecs_entity_grid.size() &&
                    screen_x < (int)ecs_entity_grid[screen_y].size() &&
                    ecs_entity_grid[screen_y][screen_x] != " ") {

                    const std::string& entity_glyph = ecs_entity_grid[screen_y][screen_x];
                    // Use default color for ECS entities for now
                    row_elements.push_back(text(entity_glyph) | color(Color::White) | bold);
                    entity_rendered = true;
                }
            }

            if (entity_rendered) {
                continue;
            }

            // Check for items at this position (only if visible)
            bool item_rendered = false;
            if (map.isVisible(map_pos.x, map_pos.y)) {
                for (const auto& item : all_items) {
                    if (item && item->x == map_pos.x && item->y == map_pos.y) {
                        // Convert item color string to ftxui Color
                        Color item_color = Color::White;  // Default
                        if (item->color == "red") item_color = Color::Red;
                        else if (item->color == "bright_red") item_color = Color::RedLight;
                        else if (item->color == "blue") item_color = Color::Blue;
                        else if (item->color == "yellow") item_color = Color::Yellow;
                        else if (item->color == "brown") item_color = Color::RGB(139, 69, 19);  // Brown
                        else if (item->color == "cyan") item_color = Color::Cyan;
                        else if (item->color == "magenta") item_color = Color::Magenta;
                        else if (item->color == "grey") item_color = Color::GrayDark;
                        else if (item->color == "white") item_color = Color::White;

                        // Render the item
                        std::string symbol(1, item->symbol);
                        row_elements.push_back(text(symbol) | color(item_color));
                        item_rendered = true;
                        break;
                    }
                }
            }

            if (item_rendered) {
                continue;
            }
            
            // Check if position is within map bounds
            if (!map.inBounds(map_pos)) {
                row_elements.push_back(text(" "));
                continue;
            }
            
            // Get tile properties
            std::string glyph = map.getGlyph(map_pos.x, map_pos.y);
            Color fg = map.getForeground(map_pos.x, map_pos.y);
            Color bg = map.getBackground(map_pos.x, map_pos.y);
            
            // Apply variations for visual interest (disabled for debugging)
            // glyph = getTileVariant(glyph, map_pos.x, map_pos.y);
            // fg = getColorVariation(fg, map_pos.x, map_pos.y);
            
            // Check visibility
            if (map.isVisible(map_pos.x, map_pos.y)) {
                // Fully visible
                Element tile = text(glyph) | color(fg);
                
                // Add highlight if this tile is selected
                if (highlight_pos == map_pos) {
                    tile = tile | bgcolor(Color::Yellow) | bold;
                } else if (bg != Color::Black) {
                    tile = tile | bgcolor(bg);
                }
                
                row_elements.push_back(tile);
            } else if (map.isExplored(map_pos.x, map_pos.y)) {
                // Previously seen but not currently visible
                // Check if this tile is in a lit room that we've explored
                const Room* room = game.getMap()->getRoomAt(map_pos);
                if (room && room->isLit()) {
                    // Lit rooms remain bright when explored (Angband-style)
                    // Use normal colors, not dimmed
                    Color lit_memory_color = (glyph == "█") ? Color::Yellow : Color::White;
                    row_elements.push_back(
                        text(glyph) | color(lit_memory_color)
                    );
                } else {
                    // Normal memory - use darker colors
                    Color memory_color = (glyph == "█") ? Color::Magenta : Color::GrayDark;
                    row_elements.push_back(
                        text(glyph) | color(memory_color) | dim
                    );
                }
            } else {
                // Never seen - should show as blank
                row_elements.push_back(text(" "));
            }
        }
        
        rows.push_back(hbox(row_elements));
    }
    
    return vbox(rows);
}

Element MapRenderer::renderTerrain([[maybe_unused]] const Map& map) {
    // This function is no longer used, but kept for compatibility
    return text("");
}

// renderEntities method removed - now using ECS RenderSystem integration

Element MapRenderer::renderPlayer(const GameManager& game) {
    // Check if player is in viewport
    if (!isInViewport(game.player_x, game.player_y)) {
        return text("");
    }
    
    // Get screen position
    Point screen_pos = mapToScreen(game.player_x, game.player_y);
    
    // Create player element using a single positioned character
    // This is more efficient than creating a full grid overlay
    std::vector<Element> rows;
    for (int y = 0; y < viewport_height; y++) {
        std::vector<Element> row_elements;
        for (int x = 0; x < viewport_width; x++) {
            if (x == screen_pos.x && y == screen_pos.y) {
                row_elements.push_back(text("@") | color(Color::White) | bold);
            } else {
                row_elements.push_back(text(" "));
            }
        }
        rows.push_back(hbox(row_elements));
    }
    
    return vbox(rows);
}

Element MapRenderer::renderEffects() {
    // Placeholder for visual effects
    // Could include things like spell effects, explosions, etc.
    return text("");
}

Element MapRenderer::render(const Map& map, const GameManager& game) {
    // Center viewport on player if they would go off-screen
    int margin = 5; // Keep player at least 5 tiles from edge
    
    Point player_screen = mapToScreen(game.player_x, game.player_y);
    bool needs_recenter = false;
    
    if (player_screen.x < margin || 
        player_screen.x >= viewport_width - margin ||
        player_screen.y < margin || 
        player_screen.y >= viewport_height - margin) {
        needs_recenter = true;
    }
    
    if (needs_recenter) {
        centerOn(game.player_x, game.player_y);
    }
    
    // Clamp viewport to map bounds
    int max_offset_x = std::max(0, map.getWidth() - viewport_width);
    int max_offset_y = std::max(0, map.getHeight() - viewport_height);
    
    viewport_offset.x = std::clamp(viewport_offset.x, 0, max_offset_x);
    viewport_offset.y = std::clamp(viewport_offset.y, 0, max_offset_y);
    
    // Render the map with the player directly in the terrain layer
    Element composite = renderTerrainWithPlayer(map, game);
    
    // Add debug info if enabled
    if (show_coordinates) {
        std::string coord_text = "Viewport: (" + 
                                std::to_string(viewport_offset.x) + ", " +
                                std::to_string(viewport_offset.y) + ") " +
                                "Player: (" + 
                                std::to_string(game.player_x) + ", " +
                                std::to_string(game.player_y) + ")";
        composite = vbox({
            composite,
            separator(),
            text(coord_text) | color(Color::Green)
        });
    }
    
    return composite;
}