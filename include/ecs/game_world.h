/**
 * @file game_world.h
 * @brief ECS-based game world management
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

// All includes at global scope
#include <memory>
#include <vector>
#include <string>

#include "system_manager.h"
// Bridge classes removed - no longer needed in full ECS mode
#include "../game_state.h"

// Forward declarations at global scope
class MessageLog;
class CombatSystem;
class Map;

// Only now open the namespace
namespace ecs {

// Forward declare ECS classes
class ILogger;
class MovementSystem;
class RenderSystem;
class CombatSystem;
class AISystem;

/**
 * @class GameWorld
 * @brief Manages the ECS-based game world
 *
 * This class integrates the ECS architecture into the game,
 * providing a modern component-based approach while maintaining
 * compatibility with legacy systems through bridges.
 */
class GameWorld {
public:
    /**
     * @brief Construct GameWorld with necessary systems
     * @param message_log Message logging system
     * @param game_map Game map
     */
    GameWorld(MessageLog* message_log,
              Map* game_map);

    ~GameWorld();

    /**
     * @brief Initialize ECS world
     * @param migrate_existing If true, migrate existing entities to ECS
     */
    void initialize(bool migrate_existing = true);

    /**
     * @brief Update all ECS systems
     * @param delta_time Time since last update (seconds)
     */
    void update(double delta_time);

    /**
     * @brief Create player entity using ECS
     * @param x Initial X position
     * @param y Initial Y position
     * @return Player entity ID
     */
    EntityID createPlayer(int x, int y);

    /**
     * @brief Create monster entity using ECS
     * @param type Monster type
     * @param x Initial X position
     * @param y Initial Y position
     * @return Monster entity ID
     */
    EntityID createMonster(const std::string& type, int x, int y);

    /**
     * @brief Create item entity using ECS
     * @param type Item type
     * @param x Initial X position
     * @param y Initial Y position
     * @return Item entity ID
     */
    EntityID createItem(const std::string& type, int x, int y);

    /**
     * @brief Get entity by ID
     * @param id Entity ID
     * @return Entity pointer or nullptr
     */
    Entity* getEntity(EntityID id);

    /**
     * @brief Remove entity by ID
     * @param id Entity ID to remove
     * @return true if entity was removed
     */
    bool removeEntity(EntityID id);

    /**
     * @brief Get all entities at position
     * @param x X coordinate
     * @param y Y coordinate
     * @return Vector of entities at position
     */
    std::vector<Entity*> getEntitiesAt(int x, int y);

    /**
     * @brief Process player action
     * @param action Action to process
     * @param dx X direction for movement
     * @param dy Y direction for movement
     * @return Action speed
     */
    ActionSpeed processPlayerAction(int action, int dx = 0, int dy = 0);

    /**
     * @brief Process AI for all monsters
     */
    void processMonsterAI();

    /**
     * @brief Update field of view
     * @param fov FOV grid
     */
    void updateFOV(const std::vector<std::vector<bool>>& fov);

    /**
     * @brief Sync ECS state back to legacy systems
     */
    void syncToLegacy();

    /**
     * @brief Sync legacy state to ECS
     */
    void syncFromLegacy();

    /**
     * @brief Get the ECS world container
     * @return Reference to world
     */
    World& getWorld() { return world; }

    /**
     * @brief Get movement system
     * @return Movement system pointer
     */
    MovementSystem* getMovementSystem();

    /**
     * @brief Get render system
     * @return Render system pointer
     */
    RenderSystem* getRenderSystem();

    // Entity bridge removed - no longer needed in full ECS mode

    /**
     * @brief Get native combat system
     * @return Combat system pointer
     */
    CombatSystem* getCombatSystem() { return native_combat_system; }

    /**
     * @brief Get native AI system
     * @return AI system pointer
     */
    AISystem* getAISystem() { return native_ai_system; }

    // Renderer bridge removed - no longer needed in full ECS mode

    /**
     * @brief Check if position is blocked
     * @param x X coordinate
     * @param y Y coordinate
     * @return true if position is blocked
     */
    bool isPositionBlocked(int x, int y) const;

    /**
     * @brief Remove dead entities
     */
    void removeDeadEntities();

    /**
     * @brief Get entity count
     * @return Number of entities
     */
    size_t getEntityCount() const { return world.getEntityCount(); }

    /**
     * @brief Clear all entities
     */
    void clearEntities() { world.clearEntities(); }

    /**
     * @brief Get player entity ID
     * @return Player entity ID or 0 if no player
     */
    EntityID getPlayerID() const { return player_id; }

    /**
     * @brief Get player entity
     * @return Player entity pointer or nullptr if no player
     */
    Entity* getPlayerEntity() { return getEntity(player_id); }

private:
    World world;                                          ///< ECS world container
    // Bridge members removed - no longer needed in full ECS mode

    // Native ECS systems
    CombatSystem* native_combat_system = nullptr;        ///< Native ECS combat system
    AISystem* native_ai_system = nullptr;                ///< Native ECS AI system

    // Legacy systems (for migration)
    MessageLog* message_log;         ///< Message log
    std::unique_ptr<ILogger> logger; ///< Logger interface adapter
    Map* game_map;                   ///< Game map

    EntityID player_id = 0;  ///< Player entity ID

    /**
     * @brief Initialize ECS systems
     */
    void initializeSystems();

    /**
     * @brief Migrate existing entities to ECS
     */
    void migrateExistingEntities();
};

} // namespace ecs