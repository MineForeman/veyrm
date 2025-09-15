/**
 * @file combat_system_bridge.h
 * @brief Bridge to allow CombatSystem to work with ECS CombatComponent
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "../combat_system.h"
#include "combat_system.h"
#include "entity.h"
#include "combat_component.h"
#include "health_component.h"
#include "entity_manager_bridge.h"

namespace ecs {

/**
 * @class CombatSystemBridge
 * @brief Extends CombatSystem to support ECS CombatComponent
 *
 * This class allows the existing CombatSystem to work with entities
 * that use CombatComponent and HealthComponent instead of direct
 * entity properties.
 */
class CombatSystemBridge {
public:
    CombatSystemBridge(::CombatSystem* legacy_system, EntityManagerBridge* entity_bridge);
    ~CombatSystemBridge() = default;

    /**
     * @brief Process attack using CombatComponents
     * @param attacker ECS entity making the attack
     * @param defender ECS entity being attacked
     * @return CombatResult from the attack
     */
    ecs::CombatResult processComponentAttack(
        std::shared_ptr<ecs::Entity> attacker,
        std::shared_ptr<ecs::Entity> defender);

    /**
     * @brief Calculate hit using CombatComponents
     * @param attacker_combat Attacker's combat component
     * @param defender_combat Defender's combat component
     * @return true if attack hits
     */
    bool calculateComponentHit(const CombatComponent& attacker_combat,
                              const CombatComponent& defender_combat) const;

    /**
     * @brief Calculate damage using CombatComponent
     * @param combat Combat component of attacker
     * @return Damage value
     */
    int calculateComponentDamage(const CombatComponent& combat) const;

    /**
     * @brief Apply damage to entity with HealthComponent
     * @param entity Entity to damage
     * @param damage Amount of damage
     * @return Actual damage dealt
     */
    int applyComponentDamage(std::shared_ptr<ecs::Entity> entity, int damage);

    /**
     * @brief Get defense value from CombatComponent
     * @param combat Combat component
     * @return Total defense value
     */
    int getComponentDefenseValue(const CombatComponent& combat) const;

    /**
     * @brief Get attack bonus from CombatComponent
     * @param combat Combat component
     * @return Total attack bonus
     */
    int getComponentAttackBonus(const CombatComponent& combat) const;

    /**
     * @brief Check if entity can attack
     * @param entity Entity to check
     * @return true if entity has CombatComponent and can attack
     */
    bool canAttack(std::shared_ptr<ecs::Entity> entity) const;

    /**
     * @brief Check if entity can defend
     * @param entity Entity to check
     * @return true if entity has CombatComponent and can defend
     */
    bool canDefend(std::shared_ptr<ecs::Entity> entity) const;

    /**
     * @brief Sync combat results back to legacy entities
     * @param attacker ECS attacker entity
     * @param defender ECS defender entity
     */
    void syncCombatResults(std::shared_ptr<ecs::Entity> attacker,
                           std::shared_ptr<ecs::Entity> defender);

private:
    [[maybe_unused]] ::CombatSystem* legacy_system;  ///< The legacy combat system (for future use)
    EntityManagerBridge* entity_bridge;            ///< Entity manager bridge

    /**
     * @brief Roll a d20
     * @return Random value 1-20
     */
    int rollD20() const;

    /**
     * @brief Get entity name for combat messages
     * @param entity ECS entity
     * @return Entity name
     */
    std::string getEntityName(std::shared_ptr<ecs::Entity> entity) const;
};

} // namespace ecs