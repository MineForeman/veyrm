/**
 * @file monster.h
 * @brief Monster entity class
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "entity.h"
#include <string>
#include <ftxui/screen/color.hpp>

/**
 * @class Monster
 * @brief Represents hostile creatures in the game
 *
 * The Monster class extends Entity to provide enemy-specific functionality
 * including combat stats, AI behavior, threat levels, and special abilities.
 * Monsters are controlled by the AI system and provide the main combat
 * challenges for the player.
 *
 * @see Entity
 * @see MonsterFactory
 * @see MonsterAI
 * @see CombatSystem
 */
class Monster : public Entity {
public:
    // Combat stats
    int attack;          ///< Attack power for damage calculations
    int defense;         ///< Defense value for damage reduction
    int speed;           ///< Movement/action speed (100 = normal)
    int xp_value;        ///< Experience awarded when defeated

    // Monster metadata
    std::string species; ///< Monster type identifier (e.g., "goblin")
    std::string name;    ///< Display name (e.g., "Goblin Warrior")
    std::string description; ///< Detailed description for examine
    char threat_level;   ///< Difficulty rating ('a' to 'z', 'a' = easiest)

    // Behavior flags
    bool aggressive = true;      ///< Whether monster attacks on sight
    bool can_open_doors = false; ///< Can open closed doors
    bool can_see_invisible = false; ///< Can detect invisible entities
    
    /**
     * @brief Construct a new Monster
     * @param x Initial X coordinate
     * @param y Initial Y coordinate
     * @param species Monster type identifier
     */
    Monster(int x, int y, const std::string& species);

    /**
     * @brief Set combat statistics
     * @param hp Current hit points
     * @param maxHp Maximum hit points
     * @param atk Attack power
     * @param def Defense value
     * @param spd Movement speed (100 = normal)
     * @param xp Experience value when defeated
     */
    void setStats(int hp, int maxHp, int atk, int def, int spd, int xp);

    /**
     * @brief Set monster display properties
     * @param name Display name
     * @param desc Description text
     * @param glyph Character representation
     * @param color Display color
     * @param threat Threat level ('a' to 'z')
     */
    void setMetadata(const std::string& name, const std::string& desc,
                     const std::string& glyph, ftxui::Color color, char threat);

    /**
     * @brief Set behavior flags
     * @param aggro Whether monster is aggressive
     * @param doors Can open closed doors
     * @param seeInvis Can see invisible entities
     */
    void setFlags(bool aggro, bool doors, bool seeInvis);
    
    // Override Entity methods
    /// @brief Get entity type (always MONSTER)
    EntityType getType() const override { return EntityType::MONSTER; }
    /// @brief Monsters block movement
    bool isBlocking() const override { return true; }
    /// @brief Can act if alive
    bool canAct() const override { return hp > 0; }
    /**
     * @brief Update monster state
     * @param deltaTime Time since last update (seconds)
     */
    void update(double deltaTime) override;
    
    /**
     * @brief Calculate damage for an attack
     * @return Damage value based on attack stat
     */
    int calculateDamage() const;

    /**
     * @brief Apply damage to monster
     * @param amount Damage to apply (before defense)
     */
    void takeDamage(int amount);

    /**
     * @brief Check if monster is dead
     * @return true if HP <= 0
     */
    bool isDead() const { return hp <= 0; }

    /**
     * @brief Roll for attack success
     * @return Attack roll value
     */
    int getAttackRoll() const;

    /**
     * @brief Get defense value for damage reduction
     * @return Defense value
     */
    int getDefenseValue() const;

    // Combat interface overrides
    /// @brief Get attack bonus for combat rolls
    int getAttackBonus() const override { return attack; }
    /// @brief Get defense bonus for damage reduction
    int getDefenseBonus() const override { return defense; }
    /// @brief Get base damage dealt in combat
    int getBaseDamage() const override { return attack; }
    /// @brief Get name for combat messages
    std::string getCombatName() const override { return name; }
    
    /**
     * @enum AIState
     * @brief Monster AI behavior states
     */
    enum class AIState {
        IDLE,       ///< Not aware of threats
        WANDERING,  ///< Moving randomly
        CHASING,    ///< Pursuing target
        FLEEING,    ///< Running from danger
        ATTACKING   ///< Engaged in combat
    };

    AIState ai_state = AIState::IDLE;  ///< Current AI state
    Point last_known_player_pos = Point(-1, -1); ///< Last seen player position
};