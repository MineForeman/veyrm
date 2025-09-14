/**
 * @file renderer_bridge.h
 * @brief Bridge to allow Renderer to work with ECS RenderableComponent
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "entity.h"
#include "position_component.h"
#include "renderable_component.h"
#include "entity_manager_bridge.h"
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <string>

namespace ecs {

/**
 * @class RendererBridge
 * @brief Allows the existing renderer to work with RenderableComponent
 *
 * This class provides methods to render entities using their
 * RenderableComponent and PositionComponent instead of direct
 * entity properties.
 */
class RendererBridge {
public:
    RendererBridge(EntityManagerBridge* entity_bridge);
    ~RendererBridge() = default;

    /**
     * @brief Get the glyph for an entity
     * @param entity ECS entity
     * @return Character to display
     */
    std::string getEntityGlyph(std::shared_ptr<ecs::Entity> entity) const;

    /**
     * @brief Get the color for an entity
     * @param entity ECS entity
     * @return FTXUI color
     */
    ftxui::Color getEntityColor(std::shared_ptr<ecs::Entity> entity) const;

    /**
     * @brief Check if entity is visible
     * @param entity ECS entity
     * @return true if entity should be rendered
     */
    bool isEntityVisible(std::shared_ptr<ecs::Entity> entity) const;

    /**
     * @brief Get entity position
     * @param entity ECS entity
     * @return Position or (-1, -1) if no position
     */
    Point getEntityPosition(std::shared_ptr<ecs::Entity> entity) const;

    /**
     * @brief Render all entities at position
     * @param x X coordinate
     * @param y Y coordinate
     * @param fov Field of view grid
     * @return FTXUI element for rendering
     */
    ftxui::Element renderEntitiesAt(int x, int y,
                                    const std::vector<std::vector<bool>>& fov) const;

    /**
     * @brief Get top entity at position (highest render priority)
     * @param x X coordinate
     * @param y Y coordinate
     * @param fov Field of view grid
     * @return Top entity or nullptr
     */
    std::shared_ptr<ecs::Entity> getTopEntityAt(int x, int y,
                                                const std::vector<std::vector<bool>>& fov) const;

    /**
     * @brief Sort entities by render priority
     * @param entities Entities to sort
     * @return Sorted vector (highest priority first)
     */
    std::vector<std::shared_ptr<ecs::Entity>> sortByRenderPriority(
        std::vector<std::shared_ptr<ecs::Entity>> entities) const;

    /**
     * @brief Check if entity blocks sight
     * @param entity ECS entity
     * @return true if entity blocks line of sight
     */
    bool doesEntityBlockSight(std::shared_ptr<ecs::Entity> entity) const;

    /**
     * @brief Set visibility for entity
     * @param entity ECS entity
     * @param visible New visibility state
     */
    void setEntityVisibility(std::shared_ptr<ecs::Entity> entity, bool visible);

    /**
     * @brief Update entity visibility based on FOV
     * @param fov Field of view grid
     */
    void updateEntitiesVisibility(const std::vector<std::vector<bool>>& fov);

    /**
     * @brief Create render element for entity
     * @param entity ECS entity
     * @return FTXUI element
     */
    ftxui::Element createEntityElement(std::shared_ptr<ecs::Entity> entity) const;

    /**
     * @brief Get all renderable entities in view
     * @param view_x View top-left X
     * @param view_y View top-left Y
     * @param view_width View width
     * @param view_height View height
     * @param fov Field of view grid
     * @return Entities in view
     */
    std::vector<std::shared_ptr<ecs::Entity>> getEntitiesInView(
        int view_x, int view_y, int view_width, int view_height,
        const std::vector<std::vector<bool>>& fov) const;

private:
    EntityManagerBridge* entity_bridge;  ///< Entity manager bridge

    /**
     * @brief Check if position is in FOV
     * @param x X coordinate
     * @param y Y coordinate
     * @param fov Field of view grid
     * @return true if position is visible
     */
    bool isInFOV(int x, int y, const std::vector<std::vector<bool>>& fov) const;
};

} // namespace ecs