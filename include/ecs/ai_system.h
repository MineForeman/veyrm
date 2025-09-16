/**
 * @file ai_system.h
 * @brief Native ECS AI system for monster behavior
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <memory>
#include <random>
#include <vector>
#include <deque>

#include "system.h"
#include "entity.h"
#include "position_component.h"
#include "logger_interface.h"
#include "../map.h"

namespace ecs {

// Forward declarations
class MovementSystem;
class CombatSystem;
class ILogger;

/**
 * @enum AIBehavior
 * @brief Different AI behavior types
 */
enum class AIBehavior {
    PASSIVE,      ///< Doesn't move or attack
    WANDERING,    ///< Moves randomly
    AGGRESSIVE,   ///< Seeks and attacks player
    DEFENSIVE,    ///< Attacks when threatened, retreats when hurt
    PATROL,       ///< Follows a patrol route
    FLEEING,      ///< Running away from threats
    SUPPORT       ///< Healing/helping allies
};

/**
 * @class AIComponent
 * @brief Component for AI-controlled entities
 */
class AIComponent : public Component<AIComponent> {
public:
    AIBehavior behavior = AIBehavior::WANDERING;  ///< AI behavior type
    int vision_range = 6;                          ///< How far the AI can see
    int aggro_range = 4;                           ///< Range at which AI becomes hostile
    EntityID target_id = 0;                        ///< Current target entity ID
    std::deque<Point> path;                        ///< Current pathfinding path
    bool has_seen_player = false;                  ///< Whether AI has spotted player
    int turns_since_player_seen = 0;               ///< Turns since last saw player
    Point last_player_position{-1, -1};            ///< Last known player position

    // Patrol behavior
    std::vector<Point> patrol_points;              ///< Points to patrol
    size_t current_patrol_index = 0;               ///< Current patrol point index

    AIComponent() = default;
    AIComponent(const AIComponent&) = default;
    AIComponent& operator=(const AIComponent&) = default;
    virtual ~AIComponent() = default;

    std::string getTypeName() const override { return "AIComponent"; }
    ComponentType getType() const override { return ComponentType::AI; }
};

/**
 * @class AISystem
 * @brief Handles AI behavior for non-player entities
 */
class AISystem : public System<AISystem> {
public:
    /**
     * @brief Construct AI system
     * @param map Game map for pathfinding
     * @param movement_system Movement system for issuing move commands
     * @param combat_system Combat system for issuing attacks
     * @param logger Logger for AI messages and debug output
     */
    AISystem(Map* map,
             MovementSystem* movement_system,
             CombatSystem* combat_system,
             ILogger* logger);

    ~AISystem() = default;

    /**
     * @brief Update AI for all entities
     * @param entities All entities in the world
     * @param delta_time Time since last update
     */
    void update(const std::vector<std::unique_ptr<Entity>>& entities, double delta_time) override;

    /**
     * @brief Get system priority
     * @return Priority value (lower = earlier execution)
     */
    int getPriority() const override { return 30; } // After input, before movement

    /**
     * @brief Check if system should process an entity
     * @param entity Entity to check
     * @return true if entity has AI component
     */
    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<AIComponent>();
    }

    /**
     * @brief Set the player entity for AI targeting
     * @param id ID of the player entity
     */
    void setPlayerId(EntityID id) { this->player_id = id; }

private:
    Map* map;                           ///< Game map
    MovementSystem* movement_system;    ///< Movement system
    CombatSystem* combat_system;       ///< Combat system
    ILogger* logger;                                    ///< Logger for messages and debug output
    EntityID player_id = 0;             ///< Player entity ID
    std::mt19937 rng;                  ///< Random number generator

    /**
     * @brief Process AI for a single entity
     * @param entity Entity with AI component
     * @param entities All entities (for targeting)
     */
    void processEntityAI(std::shared_ptr<Entity> entity,
                        const std::vector<std::unique_ptr<Entity>>& entities);

    /**
     * @brief Handle passive behavior
     * @param entity AI entity
     */
    void handlePassiveBehavior(std::shared_ptr<Entity> entity);

    /**
     * @brief Handle wandering behavior
     * @param entity AI entity
     */
    void handleWanderingBehavior(std::shared_ptr<Entity> entity);

    /**
     * @brief Handle aggressive behavior
     * @param entity AI entity
     * @param player Player entity
     */
    void handleAggressiveBehavior(std::shared_ptr<Entity> entity,
                                 std::shared_ptr<Entity> player);

    /**
     * @brief Handle defensive behavior
     * @param entity AI entity
     * @param player Player entity
     */
    void handleDefensiveBehavior(std::shared_ptr<Entity> entity,
                                std::shared_ptr<Entity> player);

    /**
     * @brief Handle patrol behavior
     * @param entity AI entity
     */
    void handlePatrolBehavior(std::shared_ptr<Entity> entity);

    /**
     * @brief Handle fleeing behavior
     * @param entity AI entity
     * @param threat Threat to flee from
     */
    void handleFleeingBehavior(std::shared_ptr<Entity> entity,
                               std::shared_ptr<Entity> threat);

    /**
     * @brief Handle support behavior (healing/helping allies)
     * @param entity AI entity
     * @param player Player entity
     */
    void handleSupportBehavior(std::shared_ptr<Entity> entity,
                              std::shared_ptr<Entity> player);

    /**
     * @brief Check if entity can see target
     * @param entity Observer entity
     * @param target Target entity
     * @return true if target is visible
     */
    bool canSeeEntity(std::shared_ptr<Entity> entity,
                     std::shared_ptr<Entity> target) const;

    /**
     * @brief Calculate distance between entities
     * @param e1 First entity
     * @param e2 Second entity
     * @return Manhattan distance
     */
    int getDistance(std::shared_ptr<Entity> e1,
                   std::shared_ptr<Entity> e2) const;

    /**
     * @brief Find path to target using BFS
     * @param from Starting position
     * @param to Target position
     * @return Path as deque of points
     */
    std::deque<Point> findPath(const Point& from, const Point& to) const;

    /**
     * @brief Move entity towards target
     * @param entity AI entity
     * @param target Target position
     * @return true if moved
     */
    bool moveTowards(std::shared_ptr<Entity> entity, const Point& target);

    /**
     * @brief Move entity away from target
     * @param entity AI entity
     * @param threat Threat position to flee from
     * @return true if moved
     */
    bool moveAway(std::shared_ptr<Entity> entity, const Point& threat);

    /**
     * @brief Try to attack target if adjacent
     * @param entity Attacking entity
     * @param target Target entity
     * @return true if attack was made
     */
    bool tryAttack(std::shared_ptr<Entity> entity,
                  std::shared_ptr<Entity> target);

    /**
     * @brief Get random adjacent position
     * @param pos Current position
     * @return Random adjacent position
     */
    Point getRandomAdjacentPosition(const Point& pos) const;

    /**
     * @brief Find entity by ID
     * @param entities All entities
     * @param id Entity ID
     * @return Entity or nullptr
     */
    std::shared_ptr<Entity> findEntity(
        const std::vector<std::unique_ptr<Entity>>& entities,
        EntityID id) const;
};

} // namespace ecs