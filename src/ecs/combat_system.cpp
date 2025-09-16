/**
 * @file combat_system.cpp
 * @brief Native ECS combat system implementation
 */

#include "ecs/combat_system.h"
#include "ecs/renderable_component.h"
#include "ecs/event.h"
#include <algorithm>
#include <cmath>

namespace ecs {

CombatSystem::CombatSystem(ILogger* logger)
    : logger(logger)
    , rng(std::random_device{}()) {
}

void CombatSystem::update(const std::vector<std::unique_ptr<Entity>>& entities, double) {
    // Process all pending attacks
    for (const auto& attack : pending_attacks) {
        auto attacker = findEntity(entities, attack.attacker_id);
        auto defender = findEntity(entities, attack.defender_id);

        if (attacker && defender) {
            auto result = processAttack(attacker, defender);
            // Result message is already logged in processAttack
        }
    }

    pending_attacks.clear();
}

CombatResult CombatSystem::processAttack(std::shared_ptr<Entity> attacker,
                                         std::shared_ptr<Entity> defender) {
    CombatResult result;

    // Verify both entities can participate in combat
    if (!canAttack(attacker) || !canAttack(defender)) {
        result.message = "Invalid combat participants";
        return result;
    }

    auto* attacker_combat = attacker->getComponent<CombatComponent>();
    auto* defender_combat = defender->getComponent<CombatComponent>();

    if (!attacker_combat || !defender_combat) {
        result.message = "Missing combat components";
        return result;
    }

    // Check if entities are in range
    if (!areAdjacent(attacker, defender)) {
        result.message = "Target out of range";
        return result;
    }

    // Fire attack event
    EventSystem::getInstance().emit(
        AttackEvent(attacker->getID(), defender->getID(), "melee")
    );

    // Calculate hit
    result.hit = calculateHit(*attacker_combat, *defender_combat);

    if (logger) {
        logger->logCombat("Attack from " + std::to_string(attacker->getID()) +
                         " to " + std::to_string(defender->getID()) +
                         " - Hit: " + (result.hit ? "true" : "false"));
    }

    if (result.hit) {
        // Calculate and apply damage
        int base_damage = calculateDamage(*attacker_combat);
        result.damage = applyDamage(defender, base_damage);

        if (logger) {
            logger->logCombat("Damage dealt: " + std::to_string(result.damage));
        }

        // Fire damage event
        EventSystem::getInstance().emit(
            DamageEvent(attacker->getID(), defender->getID(), result.damage)
        );

        // Check if defender died
        auto* defender_health = defender->getComponent<HealthComponent>();
        if (defender_health && defender_health->hp <= 0) {
            result.defender_died = true;

            // Fire death event
            EventSystem::getInstance().emit(
                DeathEvent(defender->getID(), attacker->getID(), "combat")
            );
        }

        // Create combat message
        std::string attacker_name = getEntityName(attacker);
        std::string defender_name = getEntityName(defender);

        if (result.defender_died) {
            result.message = attacker_name + " kills " + defender_name + "!";
        } else {
            result.message = attacker_name + " hits " + defender_name +
                           " for " + std::to_string(result.damage) + " damage.";
        }
    } else {
        // Miss message
        std::string attacker_name = getEntityName(attacker);
        std::string defender_name = getEntityName(defender);
        result.message = attacker_name + " misses " + defender_name + ".";
    }

    // Log the final message once
    if (logger && !result.message.empty()) {
        logger->logCombat(result.message);
    }

    return result;
}

void CombatSystem::queueAttack(EntityID attacker_id, EntityID defender_id) {
    pending_attacks.push_back({attacker_id, defender_id});
}

bool CombatSystem::canAttack(std::shared_ptr<Entity> entity) const {
    if (!entity) return false;

    // Entity needs both combat and health components to participate in combat
    return entity->hasComponent<CombatComponent>() &&
           entity->hasComponent<HealthComponent>();
}

bool CombatSystem::areAdjacent(std::shared_ptr<Entity> e1, std::shared_ptr<Entity> e2) const {
    auto* pos1 = e1->getComponent<PositionComponent>();
    auto* pos2 = e2->getComponent<PositionComponent>();

    if (!pos1 || !pos2) return false;

    // Calculate Manhattan distance (for 4-directional movement)
    int dx = std::abs(pos1->position.x - pos2->position.x);
    int dy = std::abs(pos1->position.y - pos2->position.y);

    // Adjacent means distance of 1 in cardinal directions
    // or sqrt(2) for diagonal (if diagonal movement is allowed)
    return (dx <= 1 && dy <= 1) && (dx + dy > 0);
}

int CombatSystem::applyDamage(std::shared_ptr<Entity> entity, int damage) {
    auto* health = entity->getComponent<HealthComponent>();
    auto* combat = entity->getComponent<CombatComponent>();

    if (!health || !combat) return 0;

    // Apply defense reduction
    int actual_damage = std::max(1, damage - combat->defense_bonus);

    // Apply damage to health
    health->hp -= actual_damage;

    // Mark as dead if hp <= 0
    if (health->hp <= 0) {
        health->hp = 0;
        // Could set a "dead" flag here if needed
    }

    return actual_damage;
}

std::string CombatSystem::getEntityName(std::shared_ptr<Entity> entity) const {
    if (!entity) return "unknown";

    // Try to get name from renderable component
    auto* renderable = entity->getComponent<RenderableComponent>();
    if (renderable && !renderable->name.empty()) {
        return renderable->name;
    }

    // Fall back to entity ID
    return "entity_" + std::to_string(entity->getID());
}

bool CombatSystem::calculateHit(const CombatComponent& attacker_combat,
                                const CombatComponent& defender_combat) {
    // D20 system: roll + attack bonus vs 10 + defense
    int attack_roll = rollDice(20) + attacker_combat.attack_bonus;
    int defense_value = 10 + defender_combat.defense_bonus;

    return attack_roll >= defense_value;
}

int CombatSystem::calculateDamage(const CombatComponent& combat) {
    // Roll damage based on damage range
    int min_damage = combat.min_damage;
    int max_damage = combat.max_damage;

    if (min_damage >= max_damage) {
        return min_damage;
    }

    std::uniform_int_distribution<int> dist(min_damage, max_damage);
    return dist(rng);
}

int CombatSystem::rollDice(int sides) {
    std::uniform_int_distribution<int> dist(1, sides);
    return dist(rng);
}

std::shared_ptr<Entity> CombatSystem::findEntity(
    const std::vector<std::unique_ptr<Entity>>& entities,
    EntityID id) const {

    auto it = std::find_if(entities.begin(), entities.end(),
        [id](const std::unique_ptr<Entity>& e) {
            return e->getID() == id;
        });

    // Create shared_ptr that doesn't own the entity
    return (it != entities.end()) ? std::shared_ptr<Entity>(it->get(), [](Entity*){}) : nullptr;
}

} // namespace ecs