/**
 * @file movement_system.cpp
 * @brief Implementation of movement system
 */

#include "../../include/ecs/movement_system.h"
#include "../../include/ecs/renderable_component.h"
#include "../../include/ecs/combat_component.h"
#include <iostream>

namespace ecs {

void MovementSystem::update(const std::vector<std::unique_ptr<Entity>>& entities,
                            [[maybe_unused]] double delta_time) {
    // Process queued movements
    processQueue(entities);
}

void MovementSystem::processQueue(const std::vector<std::unique_ptr<Entity>>& entities) {
    while (!move_queue.empty()) {
        MoveCommand cmd = move_queue.front();
        move_queue.pop();

        Entity* entity = findEntity(entities, cmd.entity_id);
        if (entity && entity->hasComponent<PositionComponent>()) {
            moveEntity(*entity, cmd.dx, cmd.dy, cmd.forced);
        }
    }
}

bool MovementSystem::moveEntity(Entity& entity, int dx, int dy, bool forced) {
    auto* pos = entity.getComponent<PositionComponent>();
    if (!pos) return false;

    int new_x = pos->position.x + dx;
    int new_y = pos->position.y + dy;


    return moveEntityTo(entity, new_x, new_y, forced);
}

bool MovementSystem::moveEntityTo(Entity& entity, int x, int y, bool forced) {
    auto* pos = entity.getComponent<PositionComponent>();
    if (!pos) return false;

    // Check if movement is valid
    if (!forced) {
        // Check map bounds and walkability
        if (!game_map || !game_map->inBounds(x, y) || !game_map->isWalkable(x, y)) {
            return false;
        }
    }

    // Update position
    pos->moveTo(x, y);
    return true;
}

bool MovementSystem::isValidPosition(int x, int y,
                                    const std::vector<std::unique_ptr<Entity>>& entities,
                                    const Entity* moving_entity) const {
    // Check map bounds and walkability
    if (!game_map || !game_map->inBounds(x, y) || !game_map->isWalkable(x, y)) {
        return false;
    }

    // Check for blocking entities
    Entity* blocker = getBlockingEntity(x, y, entities, moving_entity);
    return blocker == nullptr;
}

Entity* MovementSystem::getBlockingEntity(int x, int y,
                                         const std::vector<std::unique_ptr<Entity>>& entities,
                                         const Entity* ignore) const {
    for (const auto& entity : entities) {
        if (entity.get() == ignore) continue;

        auto* pos = entity->getComponent<PositionComponent>();
        if (!pos || !pos->isAt(x, y)) continue;

        // Check if entity blocks movement
        // An entity blocks if it has a combat component (creature) or
        // if its renderable component indicates it blocks
        auto* combat = entity->getComponent<CombatComponent>();
        auto* render = entity->getComponent<RenderableComponent>();

        if (combat || (render && render->blocks_sight)) {
            return entity.get();
        }
    }

    return nullptr;
}

Entity* MovementSystem::findEntity(const std::vector<std::unique_ptr<Entity>>& entities,
                                  EntityID id) const {
    for (const auto& entity : entities) {
        if (entity->getID() == id) {
            return entity.get();
        }
    }
    return nullptr;
}

} // namespace ecs