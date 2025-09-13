#include "combat_system.h"
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "message_log.h"
#include "log.h"
#include <random>
#include <algorithm>
#include <sstream>

// Define static constants
const int CombatSystem::CRITICAL_HIT_THRESHOLD;
const int CombatSystem::CRITICAL_MISS_THRESHOLD;
const int CombatSystem::BASE_DEFENSE;
const int CombatSystem::MIN_DAMAGE;

CombatSystem::CombatSystem() : message_log(nullptr) {}

CombatSystem::CombatSystem(MessageLog* log) : message_log(log) {}

CombatSystem::CombatResult CombatSystem::processAttack(Entity& attacker, Entity& defender) {
    CombatResult result;

    std::string attacker_name = getEntityName(attacker);
    std::string defender_name = getEntityName(defender);

    LOG_COMBAT("=== COMBAT START ===");
    LOG_COMBAT("Attacker: " + attacker_name + " (HP: " + std::to_string(attacker.hp) + "/" + std::to_string(attacker.max_hp) + ")");
    LOG_COMBAT("Defender: " + defender_name + " (HP: " + std::to_string(defender.hp) + "/" + std::to_string(defender.max_hp) + ")");

    // Roll d20 once for the entire attack
    int raw_d20 = rollD20();
    int attack_bonus = attacker.getAttackBonus();
    int attack_roll = raw_d20 + attack_bonus;
    int defense_value = getDefenseValue(defender);

    LOG_COMBAT("Raw d20 roll: " + std::to_string(raw_d20));
    LOG_COMBAT("Attack bonus: " + std::to_string(attack_bonus));
    LOG_COMBAT("Total attack roll: " + std::to_string(attack_roll));
    LOG_COMBAT("Defense value: " + std::to_string(defense_value));

    // Check for critical hit/miss first (based on raw d20)
    result.critical = isCriticalHit(raw_d20);
    bool critical_miss = isCriticalMiss(raw_d20);

    LOG_COMBAT("Critical hit check (raw d20 == 20): " + std::string(result.critical ? "YES" : "NO"));
    LOG_COMBAT("Critical miss check (raw d20 == 1): " + std::string(critical_miss ? "YES" : "NO"));

    // Determine hit
    if (result.critical) {
        result.hit = true; // Critical hit always hits
        LOG_COMBAT("Hit result: CRITICAL HIT (auto-hit)");
    } else if (critical_miss) {
        result.hit = false; // Critical miss always misses
        LOG_COMBAT("Hit result: CRITICAL MISS (auto-miss)");
    } else {
        result.hit = (attack_roll >= defense_value); // Standard hit calculation
        LOG_COMBAT("Hit result: " + std::string(result.hit ? "HIT" : "MISS") +
                  " (" + std::to_string(attack_roll) + " vs " + std::to_string(defense_value) + ")");
    }

    if (result.hit) {
        // Calculate damage
        int base_damage = calculateDamage(attacker);
        result.damage = base_damage;
        LOG_COMBAT("Base damage calculated: " + std::to_string(base_damage));

        if (result.critical) {
            result.damage *= 2; // Double damage on critical
            LOG_COMBAT("Critical hit - damage doubled: " + std::to_string(result.damage));
        }

        // Apply damage
        int defender_hp_before = defender.hp;
        applyDamage(defender, result.damage);
        int actual_damage_dealt = defender_hp_before - defender.hp;
        result.fatal = (defender.hp <= 0);

        LOG_COMBAT("Damage to apply: " + std::to_string(result.damage));
        LOG_COMBAT("Actual damage dealt: " + std::to_string(actual_damage_dealt));
        LOG_COMBAT("Defender HP after: " + std::to_string(defender.hp) + "/" + std::to_string(defender.max_hp));
        LOG_COMBAT("Fatal: " + std::string(result.fatal ? "YES" : "NO"));

        // Generate messages
        result.attack_message = generateAttackMessage(attacker, defender, true, result.critical);
        result.damage_message = generateDamageMessage(defender, result.damage, result.fatal);

        if (result.fatal) {
            result.result_message = getEntityName(defender) + (isPlayer(defender) ? " die!" : " dies!");
            LOG_COMBAT("Death message: " + result.result_message);
        }
    } else {
        // Miss
        result.attack_message = generateAttackMessage(attacker, defender, false, false);
        LOG_COMBAT("Attack missed - no damage dealt");
    }

    LOG_COMBAT("Attack message: " + result.attack_message);
    if (!result.damage_message.empty()) {
        LOG_COMBAT("Damage message: " + result.damage_message);
    }

    // Log the result
    if (message_log) {
        logCombatResult(result);
        LOG_COMBAT("Combat result logged to message log");
    } else {
        LOG_COMBAT("No message log available - combat result not logged");
    }

    LOG_COMBAT("=== COMBAT END ===");
    return result;
}

bool CombatSystem::calculateHit(const Entity& attacker, const Entity& defender) {
    // Roll d20 and calculate attack
    int raw_d20 = rollD20();
    int attack_roll = raw_d20 + attacker.getAttackBonus();
    int defense_value = getDefenseValue(defender);

    // Critical hit always hits
    if (isCriticalHit(raw_d20)) {
        return true;
    }

    // Critical miss always misses
    if (isCriticalMiss(raw_d20)) {
        return false;
    }

    // Standard hit calculation
    return attack_roll >= defense_value;
}

int CombatSystem::calculateDamage(const Entity& attacker) {
    // Get base damage from entity
    int base_damage = attacker.getBaseDamage();

    // Add some randomness (1 to base_damage)
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> damage_roll(1, std::max(1, base_damage));

    return std::max(MIN_DAMAGE, damage_roll(rng));
}

void CombatSystem::applyDamage(Entity& target, int amount) {
    // Apply defense reduction
    int defense_bonus = target.getDefenseBonus();
    int actual_damage = std::max(MIN_DAMAGE, amount - defense_bonus);

    // Apply damage
    target.hp = std::max(0, target.hp - actual_damage);
}

int CombatSystem::getAttackRoll(const Entity& attacker) {
    int d20_roll = rollD20();
    int attack_bonus = attacker.getAttackBonus();
    return d20_roll + attack_bonus;
}

int CombatSystem::getDefenseValue(const Entity& defender) {
    return BASE_DEFENSE + defender.getDefenseBonus();
}

bool CombatSystem::isCriticalHit(int roll) {
    // Only natural 20s are critical hits
    return roll == 20;
}

bool CombatSystem::isCriticalMiss(int roll) {
    // Only natural 1s are critical misses
    return roll == 1;
}

void CombatSystem::logCombatResult(const CombatResult& result) {
    if (!message_log) return;

    // Log attack message
    if (!result.attack_message.empty()) {
        message_log->addCombatMessage(result.attack_message);
    }

    // Log damage message
    if (result.hit && !result.damage_message.empty()) {
        message_log->addCombatMessage(result.damage_message);
    }

    // Log result message (death, etc.)
    if (!result.result_message.empty()) {
        message_log->addCombatMessage(result.result_message);
    }
}

std::string CombatSystem::generateAttackMessage(const Entity& attacker, const Entity& defender, bool hit, bool critical) {
    std::ostringstream msg;

    std::string attacker_name = getEntityName(attacker);
    std::string defender_name = getEntityName(defender);

    if (hit) {
        if (critical) {
            msg << attacker_name << " critically hit" << (isPlayer(attacker) ? "" : "s") << " " << defender_name << "!";
        } else {
            msg << attacker_name << " hit" << (isPlayer(attacker) ? "" : "s") << " " << defender_name << ".";
        }
    } else {
        msg << attacker_name << " miss" << (isPlayer(attacker) ? "" : "es") << " " << defender_name << ".";
    }

    return msg.str();
}

std::string CombatSystem::generateDamageMessage(const Entity& defender, int damage, bool fatal) {
    std::ostringstream msg;

    if (isPlayer(defender)) {
        msg << "You take " << damage << " damage";
        if (fatal) {
            msg << " and die!";
        } else {
            msg << ".";
        }
    } else {
        msg << getEntityName(defender) << " takes " << damage << " damage";
        if (fatal) {
            msg << " and dies!";
        } else {
            msg << ".";
        }
    }

    return msg.str();
}

int CombatSystem::rollD20() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> d20(1, 20);
    return d20(rng);
}

bool CombatSystem::isPlayer(const Entity& entity) {
    return entity.is_player;
}

bool CombatSystem::isMonster(const Entity& entity) {
    return entity.is_monster;
}

std::string CombatSystem::getEntityName(const Entity& entity) {
    if (isPlayer(entity)) {
        return "You";
    } else if (isMonster(entity)) {
        // Try to cast to Monster to get the name
        const Monster* monster = dynamic_cast<const Monster*>(&entity);
        if (monster) {
            return monster->name;
        }
        return "Monster";
    }
    return "Entity";
}