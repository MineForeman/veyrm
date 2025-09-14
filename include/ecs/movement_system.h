/**
 * @file movement_system.h
 * @brief System for handling entity movement and collision
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "system.h"
#include "position_component.h"
#include "../map.h"
#include <memory>
#include <queue>

namespace ecs {

/**
 * @struct MoveCommand
 * @brief Queued movement request for an entity
 */
struct MoveCommand {
    EntityID entity_id;  ///< Entity to move
    int dx;              ///< X-axis delta
    int dy;              ///< Y-axis delta
    bool forced;         ///< Ignore collision checks
};

/**
 * @class MovementSystem
 * @brief Handles entity movement, collision detection, and position updates
 *
 * The MovementSystem processes movement commands, checks for collisions,
 * and updates entity positions. It maintains a command queue for
 * deferred movement processing.
 */
class MovementSystem : public System<MovementSystem> {
public:
    /**
     * @brief Construct movement system with map reference
     * @param map Game map for collision checking
     */
    explicit MovementSystem(Map* map) : game_map(map) {}

    /**
     * @brief Process movement commands and update positions
     * @param entities All entities in the game
     * @param delta_time Time since last update
     */
    void update(const std::vector<std::unique_ptr<Entity>>& entities,
                double delta_time) override;

    /**
     * @brief Check if entity has position component
     * @param entity Entity to check
     * @return true if entity can be moved
     */
    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<PositionComponent>();
    }

    /**
     * @brief Queue a movement command
     * @param entity_id Entity to move
     * @param dx X-axis movement
     * @param dy Y-axis movement
     * @param forced Skip collision checks if true
     */
    void queueMove(EntityID entity_id, int dx, int dy, bool forced = false) {
        move_queue.push({entity_id, dx, dy, forced});
    }

    /**
     * @brief Move entity by offset
     * @param entity Entity to move
     * @param dx X-axis movement
     * @param dy Y-axis movement
     * @param forced Skip collision checks if true
     * @return true if movement succeeded
     */
    bool moveEntity(Entity& entity, int dx, int dy, bool forced = false);

    /**
     * @brief Move entity to absolute position
     * @param entity Entity to move
     * @param x Target X coordinate
     * @param y Target Y coordinate
     * @param forced Skip collision checks if true
     * @return true if movement succeeded
     */
    bool moveEntityTo(Entity& entity, int x, int y, bool forced = false);

    /**
     * @brief Check if position is valid for movement
     * @param x X coordinate to check
     * @param y Y coordinate to check
     * @param entities All entities (for collision)
     * @param moving_entity Entity that's moving (can pass through self)
     * @return true if position is valid
     */
    bool isValidPosition(int x, int y,
                         const std::vector<std::unique_ptr<Entity>>& entities,
                         const Entity* moving_entity = nullptr) const;

    /**
     * @brief Check if entity at position blocks movement
     * @param x X coordinate to check
     * @param y Y coordinate to check
     * @param entities All entities
     * @param ignore Entity to ignore (usually the mover)
     * @return Entity that blocks, or nullptr if clear
     */
    Entity* getBlockingEntity(int x, int y,
                             const std::vector<std::unique_ptr<Entity>>& entities,
                             const Entity* ignore = nullptr) const;

    /**
     * @brief Get system name
     * @return "MovementSystem"
     */
    std::string getName() const override { return "MovementSystem"; }

    /**
     * @brief Get execution priority
     * @return Priority value (10 = early)
     */
    int getPriority() const override { return 10; }

    /**
     * @brief Set the game map
     * @param map New map reference
     */
    void setMap(Map* map) { game_map = map; }

    /**
     * @brief Clear all queued movements
     */
    void clearQueue() {
        while (!move_queue.empty()) {
            move_queue.pop();
        }
    }

private:
    Map* game_map;                      ///< Map for collision checking
    std::queue<MoveCommand> move_queue; ///< Pending movement commands

    /**
     * @brief Process all queued movement commands
     * @param entities All entities in the game
     */
    void processQueue(const std::vector<std::unique_ptr<Entity>>& entities);

    /**
     * @brief Find entity by ID
     * @param entities All entities
     * @param id Entity ID to find
     * @return Pointer to entity or nullptr
     */
    Entity* findEntity(const std::vector<std::unique_ptr<Entity>>& entities,
                      EntityID id) const;
};

} // namespace ecs