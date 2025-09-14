/**
 * @file combat_system.h
 * @brief Native ECS combat system
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "system.h"
#include "entity.h"
#include "combat_component.h"
#include "health_component.h"
#include "position_component.h"
#include "../message_log.h"
#include <memory>
#include <random>
#include <string>

namespace ecs {

/**
 * @struct CombatResult
 * @brief Result of a combat action
 */
struct CombatResult {
    bool hit = false;           ///< Whether the attack hit
    int damage = 0;              ///< Damage dealt
    bool defender_died = false;  ///< Whether defender died
    std::string message;         ///< Combat message for log
};

/**
 * @class CombatSystem
 * @brief Handles combat between entities with combat components
 */
class CombatSystem : public System<CombatSystem> {
public:
    /**
     * @brief Construct combat system
     * @param message_log Message log for combat messages
     */
    explicit CombatSystem(MessageLog* message_log);

    ~CombatSystem() = default;

    /**
     * @brief Update combat system (process pending attacks)
     * @param entities All entities in the world
     * @param delta_time Time since last update
     */
    void update(const std::vector<std::unique_ptr<Entity>>& entities, double delta_time) override;

    /**
     * @brief Check if system should process an entity
     * @param entity Entity to check
     * @return true if entity has combat component
     */
    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<CombatComponent>();
    }

    /**
     * @brief Process an attack between two entities
     * @param attacker Attacking entity
     * @param defender Defending entity
     * @return Combat result
     */
    CombatResult processAttack(std::shared_ptr<Entity> attacker,
                               std::shared_ptr<Entity> defender);

    /**
     * @brief Queue an attack to be processed
     * @param attacker_id ID of attacking entity
     * @param defender_id ID of defending entity
     */
    void queueAttack(EntityID attacker_id, EntityID defender_id);

    /**
     * @brief Check if entity can attack
     * @param entity Entity to check
     * @return true if entity has combat and health components
     */
    bool canAttack(std::shared_ptr<Entity> entity) const;

    /**
     * @brief Check if entities are adjacent
     * @param e1 First entity
     * @param e2 Second entity
     * @return true if entities are in melee range
     */
    bool areAdjacent(std::shared_ptr<Entity> e1, std::shared_ptr<Entity> e2) const;

    /**
     * @brief Apply damage to an entity
     * @param entity Entity to damage
     * @param damage Amount of damage
     * @return Actual damage dealt after defense
     */
    int applyDamage(std::shared_ptr<Entity> entity, int damage);

    /**
     * @brief Get entity name for messages
     * @param entity Entity to get name from
     * @return Entity name or "unknown"
     */
    std::string getEntityName(std::shared_ptr<Entity> entity) const;

    /**
     * @brief Get system priority
     * @return Priority value (lower = earlier execution)
     */
    int getPriority() const override { return 50; } // Mid priority

private:
    MessageLog* message_log;                     ///< Message log for combat messages
    std::mt19937 rng;                           ///< Random number generator

    // Pending attacks to process
    struct PendingAttack {
        EntityID attacker_id;
        EntityID defender_id;
    };
    std::vector<PendingAttack> pending_attacks;

    /**
     * @brief Calculate if attack hits
     * @param attacker_combat Attacker's combat component
     * @param defender_combat Defender's combat component
     * @return true if hit
     */
    bool calculateHit(const CombatComponent& attacker_combat,
                     const CombatComponent& defender_combat);

    /**
     * @brief Calculate damage from attack
     * @param combat Attacker's combat component
     * @return Damage value
     */
    int calculateDamage(const CombatComponent& combat);

    /**
     * @brief Roll dice
     * @param sides Number of sides on die
     * @return Random value from 1 to sides
     */
    int rollDice(int sides);

    /**
     * @brief Find entity by ID
     * @param entities All entities
     * @param id Entity ID to find
     * @return Entity or nullptr
     */
    std::shared_ptr<Entity> findEntity(const std::vector<std::unique_ptr<Entity>>& entities,
                                       EntityID id) const;
};

} // namespace ecs