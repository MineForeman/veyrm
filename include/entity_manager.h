/**
 * @file entity_manager.h
 * @brief Entity lifecycle and management system
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include "entity.h"
#include "player.h"

// Forward declarations
class Map;
class MapRenderer;

/**
 * @class EntityManager
 * @brief Manages all entities in the game world
 *
 * The EntityManager is responsible for creating, destroying, and managing
 * all entities (player, monsters, items) in the game. It provides methods
 * for querying entities by position or type, updating entity states, and
 * managing entity visibility based on field of view.
 *
 * @note Uses shared_ptr for automatic memory management
 * @see Entity
 * @see Player
 * @see Monster
 */
class EntityManager {
public:
    /// Default constructor
    EntityManager();
    /// Destructor
    ~EntityManager();

    // Entity lifecycle

    /**
     * @brief Create a generic entity
     * @param type Type of entity to create
     * @param x Initial X coordinate
     * @param y Initial Y coordinate
     * @return Shared pointer to created entity
     */
    std::shared_ptr<Entity> createEntity(EntityType type, int x, int y);

    /**
     * @brief Create the player entity
     * @param x Initial X coordinate
     * @param y Initial Y coordinate
     * @return Shared pointer to player
     * @note Only one player can exist at a time
     */
    std::shared_ptr<Player> createPlayer(int x, int y);


    /**
     * @brief Remove entity from manager
     * @param entity Entity to destroy
     */
    void destroyEntity(std::shared_ptr<Entity> entity);

    /**
     * @brief Remove all entities
     */
    void clear();

    /**
     * @brief Clear all entities except the player
     * @note Used when changing levels
     */
    void clearNonPlayerEntities();
    
    // Access

    /**
     * @brief Get player entity
     * @return Shared pointer to player (may be null)
     */
    std::shared_ptr<Player> getPlayer() { return player; }

    /**
     * @brief Get player entity (const)
     * @return Const shared pointer to player
     */
    const std::shared_ptr<Player> getPlayer() const { return player; }
    
    // Query entities at position

    /**
     * @brief Get all entities at position
     * @param x X coordinate
     * @param y Y coordinate
     * @return Vector of entities at position
     */
    std::vector<std::shared_ptr<Entity>> getEntitiesAt(int x, int y) const;

    /**
     * @brief Get blocking entity at position
     * @param x X coordinate
     * @param y Y coordinate
     * @return First blocking entity found, or nullptr
     */
    std::shared_ptr<Entity> getBlockingEntityAt(int x, int y) const;

    /**
     * @brief Get item at position
     * @param x X coordinate
     * @param y Y coordinate
     * @return First item found, or nullptr
     */
    std::shared_ptr<Entity> getItemAt(int x, int y) const;

    
    // Get all entities of type

    /**
     * @brief Get all monster entities
     * @return Vector of all monsters
     */
    std::vector<std::shared_ptr<Entity>> getMonsters() const;

    /**
     * @brief Get all item entities
     * @return Vector of all items
     */
    std::vector<std::shared_ptr<Entity>> getItems() const;

    /**
     * @brief Get all entities
     * @return Vector of all managed entities
     */
    std::vector<std::shared_ptr<Entity>> getAllEntities() const;
    
    // Visibility

    /**
     * @brief Get all visible entities
     * @return Vector of entities marked as visible
     */
    std::vector<std::shared_ptr<Entity>> getVisibleEntities() const;

    /**
     * @brief Get visible monsters
     * @return Vector of visible monster entities
     */
    std::vector<std::shared_ptr<Entity>> getVisibleMonsters() const;

    /**
     * @brief Get visible items
     * @return Vector of visible item entities
     */
    std::vector<std::shared_ptr<Entity>> getVisibleItems() const;

    /**
     * @brief Update entity visibility based on FOV
     * @param fov 2D grid of visibility (true = visible)
     */
    void updateEntityVisibility(const std::vector<std::vector<bool>>& fov);
    
    // Updates

    /**
     * @brief Update all entities
     * @param delta_time Time since last update (seconds)
     */
    void updateAll(double delta_time);

    // Entity count

    /**
     * @brief Get total entity count
     * @return Number of entities
     */
    size_t getEntityCount() const { return entities.size(); }

    /**
     * @brief Get monster count
     * @return Number of monsters
     */
    size_t getMonsterCount() const;

    /**
     * @brief Get item count
     * @return Number of items
     */
    size_t getItemCount() const;

    // Utility

    /**
     * @brief Remove entities with HP <= 0
     * @note Preserves player even if dead
     */
    void removeDeadEntities();

    /**
     * @brief Check if position is blocked by entity
     * @param x X coordinate
     * @param y Y coordinate
     * @return true if blocked by entity
     */
    bool isPositionBlocked(int x, int y) const;
    
private:
    std::shared_ptr<Player> player;  ///< The player entity
    std::vector<std::shared_ptr<Entity>> entities; ///< All managed entities

    /**
     * @brief Add entity to internal list
     * @param entity Entity to add
     */
    void addEntity(std::shared_ptr<Entity> entity);
};