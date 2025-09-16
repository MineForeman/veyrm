/**
 * @file render_system.h
 * @brief System for rendering entities to the game view
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "system.h"
#include "position_component.h"
#include "renderable_component.h"
#include "../map.h"
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <string>

namespace ecs {

/**
 * @struct RenderData
 * @brief Cached render information for an entity
 */
struct RenderData {
    Point position;           ///< Entity position
    std::string glyph;       ///< Display character
    ftxui::Color color;      ///< Display color
    int priority;            ///< Render priority (higher = on top)
    bool always_visible;     ///< Ignore FOV
};

/**
 * @class RenderSystem
 * @brief Handles rendering of entities with position and renderable components
 *
 * The RenderSystem collects all visible entities and provides methods
 * to render them to various outputs (terminal, map grid, etc).
 */
class RenderSystem : public System<RenderSystem> {
public:
    /**
     * @brief Construct render system
     * @param map Game map for visibility checking
     */
    explicit RenderSystem(Map* map = nullptr) : game_map(map) {}

    /**
     * @brief Update render data cache
     * @param entities All entities in the game
     * @param delta_time Time since last update
     */
    void update(const std::vector<std::unique_ptr<Entity>>& entities,
                double delta_time) override;

    /**
     * @brief Check if entity can be rendered
     * @param entity Entity to check
     * @return true if entity has position and renderable components
     */
    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<PositionComponent>() &&
               entity.hasComponent<RenderableComponent>();
    }

    /**
     * @brief Get entity at position
     * @param x X coordinate
     * @param y Y coordinate
     * @return Render data for top entity at position, or nullptr
     */
    const RenderData* getEntityAt(int x, int y) const;

    /**
     * @brief Get all entities at position
     * @param x X coordinate
     * @param y Y coordinate
     * @return Vector of render data, sorted by priority
     */
    std::vector<const RenderData*> getEntitiesAt(int x, int y) const;

    /**
     * @brief Render entities to a 2D character grid
     * @param width Grid width
     * @param height Grid height
     * @param view_x Top-left X of view
     * @param view_y Top-left Y of view
     * @return 2D grid of characters
     */
    std::vector<std::vector<std::string>> renderToGrid(
        int width, int height, int view_x = 0, int view_y = 0) const;

    /**
     * @brief Create FTXUI element for entity display
     * @param x X coordinate
     * @param y Y coordinate
     * @return FTXUI element for the entity at position
     */
    ftxui::Element renderEntityElement(int x, int y) const;

    /**
     * @brief Check if position is visible
     * @param x X coordinate
     * @param y Y coordinate
     * @return true if position should be rendered
     */
    bool isVisible(int x, int y) const;

    /**
     * @brief Set field of view for rendering
     * @param fov 2D visibility grid
     */
    void setFOV(const std::vector<std::vector<bool>>& fov) {
        field_of_view = fov;
    }

    /**
     * @brief Clear cached render data
     */
    void clearCache() {
        render_cache.clear();
    }

    /**
     * @brief Get system name
     * @return "RenderSystem"
     */
    std::string getName() const override { return "RenderSystem"; }

    /**
     * @brief Get execution priority
     * @return Priority value (90 = late)
     */
    int getPriority() const override { return 90; }

    /**
     * @brief Set the game map
     * @param map New map reference
     */
    void setMap(Map* map) { game_map = map; }

    /**
     * @brief Get all cached render data
     * @return Vector of all render data
     */
    const std::vector<RenderData>& getRenderData() const { return render_cache; }

private:
    Map* game_map;                                ///< Map for bounds checking
    std::vector<RenderData> render_cache;         ///< Cached render data
    std::vector<std::vector<bool>> field_of_view; ///< Current FOV

    /**
     * @brief Sort render cache by position and priority
     */
    void sortRenderCache();
};

} // namespace ecs