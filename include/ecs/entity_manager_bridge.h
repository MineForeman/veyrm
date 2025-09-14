/**
 * @file entity_manager_bridge.h
 * @brief Bridge to allow EntityManager to work with ECS components
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "../entity_manager.h"
#include "entity.h"
#include "position_component.h"
#include "renderable_component.h"
#include "health_component.h"
#include "combat_component.h"
#include "entity_adapter.h"
#include <unordered_map>

namespace ecs {

/**
 * @class EntityManagerBridge
 * @brief Extends EntityManager to support ECS components
 *
 * This class provides methods to query entities based on their components
 * rather than type flags. It maintains a mapping between legacy entities
 * and ECS entities to enable gradual migration.
 */
class EntityManagerBridge {
public:
    EntityManagerBridge(EntityManager* legacy_manager);
    ~EntityManagerBridge() = default;

    /**
     * @brief Sync an ECS entity with a legacy entity
     * @param legacy_entity The legacy entity
     * @param ecs_entity The ECS entity
     */
    void syncEntity(std::shared_ptr<::Entity> legacy_entity,
                    std::shared_ptr<ecs::Entity> ecs_entity);

    /**
     * @brief Get entities at position using PositionComponent
     * @param x X coordinate
     * @param y Y coordinate
     * @return Vector of ECS entities at position
     */
    std::vector<std::shared_ptr<ecs::Entity>> getEntitiesAtPosition(int x, int y) const;

    /**
     * @brief Get entities with CombatComponent
     * @return Vector of entities that can engage in combat
     */
    std::vector<std::shared_ptr<ecs::Entity>> getCombatEntities() const;

    /**
     * @brief Get entities with RenderableComponent
     * @return Vector of entities that can be rendered
     */
    std::vector<std::shared_ptr<ecs::Entity>> getRenderableEntities() const;

    /**
     * @brief Get visible entities with RenderableComponent
     * @param fov Field of view grid
     * @return Vector of visible renderable entities
     */
    std::vector<std::shared_ptr<ecs::Entity>> getVisibleRenderableEntities(
        const std::vector<std::vector<bool>>& fov) const;

    /**
     * @brief Check if position is blocked by entity with CombatComponent
     * @param x X coordinate
     * @param y Y coordinate
     * @return true if blocked by combat entity
     */
    bool isPositionBlockedByCombatEntity(int x, int y) const;

    /**
     * @brief Update entity positions from PositionComponent
     */
    void updatePositionsFromComponents();

    /**
     * @brief Update entity health from HealthComponent
     */
    void updateHealthFromComponents();

    /**
     * @brief Get ECS entity for legacy entity
     * @param legacy_entity Legacy entity
     * @return ECS entity or nullptr
     */
    std::shared_ptr<ecs::Entity> getECSEntity(std::shared_ptr<::Entity> legacy_entity) const;

    /**
     * @brief Get legacy entity for ECS entity
     * @param ecs_entity ECS entity
     * @return Legacy entity or nullptr
     */
    std::shared_ptr<::Entity> getLegacyEntity(std::shared_ptr<ecs::Entity> ecs_entity) const;

    /**
     * @brief Create ECS components for all legacy entities
     */
    void createComponentsForLegacyEntities();

    /**
     * @brief Remove dead entities based on HealthComponent
     */
    void removeDeadEntitiesFromComponents();

private:
    EntityManager* legacy_manager;  ///< The legacy entity manager

    // Bidirectional mapping between legacy and ECS entities
    std::unordered_map<std::shared_ptr<::Entity>, std::shared_ptr<ecs::Entity>> legacy_to_ecs;
    std::unordered_map<std::shared_ptr<ecs::Entity>, std::shared_ptr<::Entity>> ecs_to_legacy;
};

} // namespace ecs