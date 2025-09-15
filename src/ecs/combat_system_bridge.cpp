/**
 * @file combat_system_bridge.cpp
 * @brief Implementation of CombatSystem ECS bridge
 */

#include "../../include/ecs/combat_system_bridge.h"
#include "../../include/message_log.h"
#include <random>
#include <algorithm>

namespace ecs {

CombatSystemBridge::CombatSystemBridge(::CombatSystem* system, EntityManagerBridge* bridge)
    : legacy_system(system), entity_bridge(bridge) {
}

ecs::CombatResult CombatSystemBridge::processComponentAttack(
    std::shared_ptr<ecs::Entity> attacker,
    std::shared_ptr<ecs::Entity> defender) {

    ecs::CombatResult result;

    // Verify both entities have combat components
    auto* attacker_combat = attacker->getComponent<CombatComponent>();
    auto* defender_combat = defender->getComponent<CombatComponent>();

    if (!attacker_combat || !defender_combat) {
        result.hit = false;
        result.message = "Invalid combat entities";
        return result;
    }

    // Check if entities can participate in combat
    if (!attacker_combat->canAttack()) {
        result.hit = false;
        result.message = getEntityName(attacker) + " cannot attack!";
        return result;
    }

    if (!defender_combat->canDefend()) {
        // Defender is stunned or otherwise can't defend - auto hit
        result.hit = true;
    } else {
        // Calculate hit
        result.hit = calculateComponentHit(*attacker_combat, *defender_combat);
    }

    std::string attacker_name = getEntityName(attacker);
    std::string defender_name = getEntityName(defender);

    // Roll for critical (simplified - ECS CombatResult doesn't have critical field)
    int d20_roll = rollD20();
    bool critical = (d20_roll == 20);
    bool critical_miss = (d20_roll == 1);

    if (critical_miss) {
        result.hit = false;
        result.message = attacker_name + " critically misses!";
        return result;
    }

    if (critical) {
        result.hit = true; // Critical always hits
        result.message = attacker_name + " scores a CRITICAL HIT!";
    }

    if (result.hit) {
        // Calculate damage
        result.damage = calculateComponentDamage(*attacker_combat);

        if (critical) {
            result.damage *= 2;
        }

        // Apply damage
        int actual_damage = applyComponentDamage(defender, result.damage);
        result.damage = actual_damage;

        // Check if fatal
        auto* defender_health = defender->getComponent<HealthComponent>();
        if (defender_health) {
            result.defender_died = defender_health->isDead();
        }

        // Build messages
        if (!critical) {
            result.message = attacker_name + " hits " + defender_name + " for " + std::to_string(result.damage) + " damage.";
        }

        if (result.defender_died) {
            result.message += " " + defender_name + " dies!";
        }
    } else {
        result.message = attacker_name + " misses " + defender_name + ".";
    }

    // Sync results back to legacy entities if needed
    syncCombatResults(attacker, defender);

    return result;
}

bool CombatSystemBridge::calculateComponentHit(const CombatComponent& attacker_combat,
                                               const CombatComponent& defender_combat) const {
    // Use legacy system's logic if available, otherwise use our own
    int attack_roll = rollD20() + getComponentAttackBonus(attacker_combat);
    int defense_value = getComponentDefenseValue(defender_combat);
    return attack_roll >= defense_value;
}

int CombatSystemBridge::calculateComponentDamage(const CombatComponent& combat) const {
    // Use damage range if specified
    if (combat.min_damage > 0 && combat.max_damage > combat.min_damage) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(combat.min_damage, combat.max_damage);
        return dis(gen);
    }

    // Otherwise use base damage
    return std::max(1, combat.base_damage);
}

int CombatSystemBridge::applyComponentDamage(std::shared_ptr<ecs::Entity> entity, int damage) {
    auto* health = entity->getComponent<HealthComponent>();
    if (!health) return 0;

    return health->takeDamage(damage);
}

int CombatSystemBridge::getComponentDefenseValue(const CombatComponent& combat) const {
    return 10 + combat.getTotalDefenseBonus();
}

int CombatSystemBridge::getComponentAttackBonus(const CombatComponent& combat) const {
    return combat.getTotalAttackBonus();
}

bool CombatSystemBridge::canAttack(std::shared_ptr<ecs::Entity> entity) const {
    auto* combat = entity->getComponent<CombatComponent>();
    return combat && combat->canAttack();
}

bool CombatSystemBridge::canDefend(std::shared_ptr<ecs::Entity> entity) const {
    auto* combat = entity->getComponent<CombatComponent>();
    return combat && combat->canDefend();
}

void CombatSystemBridge::syncCombatResults(std::shared_ptr<ecs::Entity> attacker,
                                           std::shared_ptr<ecs::Entity> defender) {
    // Sync health back to legacy entities
    if (entity_bridge) {
        auto legacy_attacker = entity_bridge->getLegacyEntity(attacker);
        auto legacy_defender = entity_bridge->getLegacyEntity(defender);

        if (legacy_attacker) {
            auto* health = attacker->getComponent<HealthComponent>();
            if (health) {
                legacy_attacker->hp = health->getHealth();
                legacy_attacker->max_hp = health->getMaxHealth();
            }
        }

        if (legacy_defender) {
            auto* health = defender->getComponent<HealthComponent>();
            if (health) {
                legacy_defender->hp = health->getHealth();
                legacy_defender->max_hp = health->getMaxHealth();
            }
        }
    }
}

int CombatSystemBridge::rollD20() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1, 20);
    return dis(gen);
}

std::string CombatSystemBridge::getEntityName(std::shared_ptr<ecs::Entity> entity) const {
    auto* combat = entity->getComponent<CombatComponent>();
    if (combat && !combat->combat_name.empty()) {
        return combat->combat_name;
    }

    // Fall back to legacy entity name if available
    if (entity_bridge) {
        auto legacy = entity_bridge->getLegacyEntity(entity);
        if (legacy && !legacy->name.empty()) {
            return legacy->name;
        }
    }

    return "Unknown";
}

} // namespace ecs