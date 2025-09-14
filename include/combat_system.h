/**
 * @file combat_system.h
 * @brief Combat mechanics and damage calculation system for Veyrm
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "point.h"
#include <string>
#include <vector>
#include <memory>

class Entity;
class Player;
class Monster;
class MessageLog;

/**
 * @class CombatSystem
 * @brief Handles all combat mechanics including attacks, damage, and messaging
 *
 * The CombatSystem manages combat interactions between entities (players and monsters).
 * It implements a d20-based combat system with attack rolls, armor class calculations,
 * damage rolls, and critical hit/miss mechanics. The system also generates appropriate
 * combat messages and integrates with the message logging system.
 *
 * Combat follows these rules:
 * - Attack roll: d20 + attacker bonuses vs target defense
 * - Damage roll: Entity-specific damage dice
 * - Critical hits on natural 20, critical misses on natural 1
 * - Minimum damage of 1 point per successful hit
 *
 * @see Entity
 * @see Player
 * @see Monster
 * @see MessageLog
 */
class CombatSystem {
public:
    /**
     * @struct CombatResult
     * @brief Contains the results of a combat attack
     *
     * This structure encapsulates all information about a combat attack,
     * including whether it hit, damage dealt, special conditions, and
     * the messages to display to the player.
     */
    struct CombatResult {
        bool hit = false;              ///< Whether the attack hit the target
        int damage = 0;                ///< Amount of damage dealt
        bool critical = false;         ///< Whether this was a critical hit
        bool fatal = false;            ///< Whether the attack killed the target
        std::string attack_message;    ///< Message describing the attack
        std::string damage_message;    ///< Message describing the damage
        std::string result_message;    ///< Message describing the result
    };

    /**
     * @brief Construct a new CombatSystem without message logging
     */
    CombatSystem();

    /**
     * @brief Construct a new CombatSystem with message logging
     * @param message_log Pointer to the message log system
     */
    explicit CombatSystem(MessageLog* message_log);

    /// Default destructor
    ~CombatSystem() = default;

    // Core combat methods

    /**
     * @brief Process a complete attack between two entities
     * @param attacker The entity making the attack
     * @param defender The entity being attacked
     * @return CombatResult containing all attack information
     *
     * This is the main combat method that handles the complete attack sequence:
     * calculating hit chance, rolling damage, applying effects, and generating
     * appropriate messages. The result contains all information needed for
     * display and game state updates.
     *
     * @see calculateHit()
     * @see calculateDamage()
     * @see applyDamage()
     */
    CombatResult processAttack(Entity& attacker, Entity& defender);

    /**
     * @brief Determine if an attack hits its target
     * @param attacker The entity making the attack
     * @param defender The entity being attacked
     * @return true if the attack hits, false if it misses
     *
     * Uses d20 + attacker modifiers vs defender's armor class.
     * Automatically hits on natural 20, automatically misses on natural 1.
     *
     * @see getAttackRoll()
     * @see getDefenseValue()
     */
    bool calculateHit(const Entity& attacker, const Entity& defender);

    /**
     * @brief Calculate damage dealt by an attacker
     * @param attacker The entity dealing damage
     * @return Amount of damage to deal (minimum 1)
     *
     * Rolls damage dice based on the attacker's weapon/natural attacks.
     * Critical hits may modify this value.
     *
     * @see isCriticalHit()
     */
    int calculateDamage(const Entity& attacker);

    /**
     * @brief Apply damage to a target entity
     * @param target The entity taking damage
     * @param amount The amount of damage to apply
     *
     * Reduces the target's health by the specified amount.
     * Handles death state if health drops to zero or below.
     */
    void applyDamage(Entity& target, int amount);

    // Combat utilities

    /**
     * @brief Get the attack roll for an entity
     * @param attacker The entity making the attack
     * @return Total attack roll value (d20 + modifiers)
     *
     * Rolls a d20 and adds the attacker's relevant bonuses.
     * Used in hit chance calculations.
     *
     * @see calculateHit()
     */
    int getAttackRoll(const Entity& attacker);

    /**
     * @brief Get the defense value for an entity
     * @param defender The entity being attacked
     * @return Total armor class/defense value
     *
     * Calculates the target number that attacks must meet or exceed
     * to hit this entity. Based on armor and natural defenses.
     *
     * @see calculateHit()
     */
    int getDefenseValue(const Entity& defender);

    /**
     * @brief Check if a die roll is a critical hit
     * @param roll The raw die roll (1-20)
     * @return true if this is a critical hit (natural 20)
     *
     * Critical hits automatically hit and may deal extra damage.
     *
     * @see CRITICAL_HIT_THRESHOLD
     */
    bool isCriticalHit(int roll);

    /**
     * @brief Check if a die roll is a critical miss
     * @param roll The raw die roll (1-20)
     * @return true if this is a critical miss (natural 1)
     *
     * Critical misses automatically miss regardless of modifiers.
     *
     * @see CRITICAL_MISS_THRESHOLD
     */
    bool isCriticalMiss(int roll);

    // Message integration

    /**
     * @brief Set the message log for combat message output
     * @param log Pointer to the message log system
     */
    void setMessageLog(MessageLog* log) { message_log = log; }

    /**
     * @brief Log combat result messages to the message system
     * @param result The combat result to log
     *
     * Outputs the attack, damage, and result messages from a
     * CombatResult to the message log if one is configured.
     */
    void logCombatResult(const CombatResult& result);

    // Configuration

    /// Die roll threshold for critical hits (natural 20)
    static const int CRITICAL_HIT_THRESHOLD = 20;
    /// Die roll threshold for critical misses (natural 1)
    static const int CRITICAL_MISS_THRESHOLD = 1;
    /// Base armor class for unarmored entities
    static const int BASE_DEFENSE = 10;
    /// Minimum damage dealt by any successful attack
    static const int MIN_DAMAGE = 1;

private:
    /// Pointer to the message log system for combat messages
    MessageLog* message_log = nullptr;

    // Combat message generation

    /**
     * @brief Generate an attack message based on combat conditions
     * @param attacker The entity making the attack
     * @param defender The entity being attacked
     * @param hit Whether the attack hit
     * @param critical Whether this was a critical hit
     * @return Formatted attack message string
     */
    std::string generateAttackMessage(const Entity& attacker, const Entity& defender, bool hit, bool critical);

    /**
     * @brief Generate a damage message for an attack result
     * @param defender The entity taking damage
     * @param damage The amount of damage dealt
     * @param fatal Whether the damage killed the target
     * @return Formatted damage message string
     */
    std::string generateDamageMessage(const Entity& defender, int damage, bool fatal);

    // Internal helpers

    /**
     * @brief Roll a 20-sided die
     * @return Random value from 1 to 20
     */
    int rollD20();

    /**
     * @brief Check if an entity is a player
     * @param entity The entity to check
     * @return true if the entity is a player
     */
    bool isPlayer(const Entity& entity);

    /**
     * @brief Check if an entity is a monster
     * @param entity The entity to check
     * @return true if the entity is a monster
     */
    bool isMonster(const Entity& entity);

    /**
     * @brief Get the display name of an entity
     * @param entity The entity to get the name for
     * @return Formatted name string for messages
     */
    std::string getEntityName(const Entity& entity);
};