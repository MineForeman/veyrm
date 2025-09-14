/**
 * @file player.h
 * @brief Player character class
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "entity.h"
#include "inventory.h"
#include <vector>
#include <memory>

class Map;
class EntityManager;

/**
 * @class Player
 * @brief Represents the player character
 *
 * The Player class extends Entity to provide player-specific functionality
 * including inventory management, experience/leveling, gold tracking, and
 * player-specific combat mechanics. The player is controlled by user input
 * and interacts with the game world through movement and actions.
 *
 * @see Entity
 * @see Inventory
 * @see InputHandler
 */
class Player : public Entity {
public:
    /**
     * @brief Construct a new Player
     * @param x Initial X coordinate
     * @param y Initial Y coordinate
     */
    Player(int x, int y);

    // Stats (hp and max_hp are inherited from Entity)
    int attack;      ///< Attack power for damage calculations
    int defense;     ///< Defense value for damage reduction
    int level;       ///< Current character level
    int experience;  ///< Total experience points earned
    int gold;        ///< Currency for purchasing items
    
    /**
     * @brief Attempt to move player in direction
     * @param map Current game map
     * @param entity_manager Manager for entity interactions
     * @param dx X-axis movement delta
     * @param dy Y-axis movement delta
     * @return true if movement succeeded, false if blocked
     * @note Handles collision with walls, entities, and triggers combat
     */
    bool tryMove(Map& map, EntityManager* entity_manager, int dx, int dy);

    /**
     * @brief Apply damage to player
     * @param amount Damage to apply (before defense)
     * @warning Damage cannot reduce HP below 0
     */
    void takeDamage(int amount);

    /**
     * @brief Restore player health
     * @param amount HP to restore
     * @note Cannot exceed max_hp
     */
    void heal(int amount);

    /**
     * @brief Check if player is dead
     * @return true if HP <= 0
     */
    bool isDead() const { return hp <= 0; }
    
    /**
     * @brief Award experience points
     * @param amount Experience to add
     * @note Automatically triggers levelUp() when threshold reached
     */
    void gainExperience(int amount);

    /**
     * @brief Increase player level
     * @note Increases max HP and updates stats
     */
    void levelUp();

    /// Player's inventory system
    std::unique_ptr<Inventory> inventory;

    /**
     * @brief Pick up an item
     * @param item Item to add to inventory
     * @return true if pickup succeeded, false if inventory full
     */
    bool pickupItem(std::unique_ptr<Item> item);

    /**
     * @brief Drop item from inventory
     * @param slot Inventory slot index
     * @return true if drop succeeded
     */
    bool dropItem(int slot);

    /**
     * @brief Check if player has specific item
     * @param item_id Item identifier to search for
     * @return true if item exists in inventory
     */
    bool hasItem(const std::string& item_id) const;

    /**
     * @brief Count items of specific type
     * @param item_id Item identifier to count
     * @return Number of matching items in inventory
     */
    int countItems(const std::string& item_id) const;

    /**
     * @brief Check if player can pick up items
     * @return true if inventory has space
     */
    bool canPickUp() const;

    /**
     * @brief Check if player can attack
     * @return true if attack > 0
     */
    bool canAttack() const { return attack > 0; }
    
    /**
     * @brief Handle player death
     * @note Called when HP reaches 0
     */
    virtual void onDeath() override;

    /**
     * @brief Update player state
     * @param delta_time Time since last update (seconds)
     */
    virtual void update(double delta_time) override;

    // Combat interface overrides
    /// @brief Get attack bonus for combat rolls
    int getAttackBonus() const override { return attack; }
    /// @brief Get defense bonus for damage reduction
    int getDefenseBonus() const override { return defense; }
    /// @brief Get base damage dealt in combat
    int getBaseDamage() const override { return attack; }
    /// @brief Get name for combat messages
    std::string getCombatName() const override { return "You"; }
    
private:
    /**
     * @brief Calculate level from experience
     * @return Level based on current experience
     */
    int calculateLevel() const;

    // Stats per level
    static constexpr int BASE_HP = 10;       ///< Starting hit points
    static constexpr int HP_PER_LEVEL = 5;   ///< HP gained per level
    static constexpr int BASE_ATTACK = 1;    ///< Starting attack power
    static constexpr int BASE_DEFENSE = 0;   ///< Starting defense value
};