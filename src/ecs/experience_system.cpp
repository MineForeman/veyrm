/**
 * @file experience_system.cpp
 * @brief Implementation of experience system
 */

#include <sstream>
#include <cmath>
#include <algorithm>
#include "ecs/experience_system.h"
#include "ecs/renderable_component.h"

namespace ecs {

ExperienceSystem::ExperienceSystem(ILogger* logger)
    : logger(logger) {
}

void ExperienceSystem::update(const std::vector<std::unique_ptr<Entity>>&, double) {
    // Experience system is mostly event-driven
    // Could process XP over time effects here if needed
}

int ExperienceSystem::awardExperience(Entity* entity, int amount) {
    if (!entity || amount <= 0) return 0;

    auto* exp = entity->getComponent<ExperienceComponent>();
    if (!exp) {
        entity->addComponent<ExperienceComponent>();
        exp = entity->getComponent<ExperienceComponent>();
    }

    [[maybe_unused]] int initial_level = exp->level;
    int levels_gained = exp->addExperience(amount);

    if (levels_gained > 0) {
        applyLevelUp(entity, levels_gained);

        if (logger) {
            auto* renderable = entity->getComponent<RenderableComponent>();
            std::string name = renderable ? renderable->name : "Entity";
            std::stringstream msg;
            msg << name << " gained " << levels_gained << " level(s)! Now level " << exp->level;
            logger->logSystem(msg.str());
        }

        // Call level up callback if set
        if (level_up_callback) {
            level_up_callback(entity, exp->level);
        }
    } else if (logger && amount > 0) {
        auto* renderable = entity->getComponent<RenderableComponent>();
        std::string name = renderable ? renderable->name : "Entity";
        std::stringstream msg;
        msg << name << " gained " << amount << " experience ("
            << exp->experience << "/" << exp->experience_to_next << ")";
        logger->logSystem(msg.str());
    }

    return levels_gained;
}

int ExperienceSystem::calculateKillXP(const Entity* killer, const Entity* victim) const {
    if (!killer || !victim) return 0;

    // Get levels
    int killer_level = 1;
    auto* killer_exp = killer->getComponent<ExperienceComponent>();
    if (killer_exp) {
        killer_level = killer_exp->level;
    }

    int victim_level = 1;
    auto* victim_exp = victim->getComponent<ExperienceComponent>();
    if (victim_exp) {
        victim_level = victim_exp->level;
    }

    // Calculate base XP
    int base_xp = calculateBaseXP(victim);

    // Apply level scaling
    float scaling = getXPScaling(killer_level, victim_level);
    int final_xp = static_cast<int>(base_xp * scaling);

    // Minimum XP for any kill
    return std::max(1, final_xp);
}

void ExperienceSystem::applyLevelUp(Entity* entity, int levels) {
    if (!entity || levels <= 0) return;

    // Apply stat increases
    applyStatIncreases(entity, levels);

    // Apply resource increases
    applyResourceIncreases(entity, levels);

    // Check for new abilities
    auto* exp = entity->getComponent<ExperienceComponent>();
    if (exp) {
        unlockAbilities(entity, exp->level);
    }
}

bool ExperienceSystem::spendSkillPoints(Entity* entity, const std::string& skill_id, int points) {
    if (!entity || points <= 0) return false;

    auto* exp = entity->getComponent<ExperienceComponent>();
    if (!exp) return false;

    if (!exp->spendSkillPoints(points)) {
        if (logger) {
            logger->logSystem("Not enough skill points");
        }
        return false;
    }

    // Apply skill upgrade (would need skill system)
    if (logger) {
        auto* renderable = entity->getComponent<RenderableComponent>();
        std::string name = renderable ? renderable->name : "Entity";
        std::stringstream msg;
        msg << name << " upgraded " << skill_id << " with " << points << " points";
        logger->logSystem(msg.str());
    }

    return true;
}

bool ExperienceSystem::spendStatPoints(Entity* entity, const std::string& stat, int points) {
    if (!entity || points <= 0) return false;

    auto* exp = entity->getComponent<ExperienceComponent>();
    if (!exp) return false;

    if (!exp->spendStatPoints(points)) {
        if (logger) {
            logger->logSystem("Not enough stat points");
        }
        return false;
    }

    // Apply stat increase
    auto* stats = entity->getComponent<StatsComponent>();
    if (stats) {
        if (stat == "strength") {
            stats->strength += points;
        } else if (stat == "dexterity") {
            stats->dexterity += points;
        } else if (stat == "intelligence") {
            stats->intelligence += points;
        } else if (stat == "constitution") {
            stats->constitution += points;
        } else if (stat == "wisdom") {
            stats->wisdom += points;
        } else if (stat == "charisma") {
            stats->charisma += points;
        } else if (stat == "luck") {
            // Luck not implemented yet
            // stats->luck += points;
        } else {
            // Invalid stat, refund points
            exp->stat_points += points;
            return false;
        }

        // Recalculate derived stats
        stats->recalculateDerived();

        if (logger) {
            auto* renderable = entity->getComponent<RenderableComponent>();
            std::string name = renderable ? renderable->name : "Entity";
            std::stringstream msg;
            msg << name << " increased " << stat << " by " << points;
            logger->logSystem(msg.str());
        }
    }

    return true;
}

float ExperienceSystem::getXPScaling(int attacker_level, int defender_level) const {
    int level_diff = defender_level - attacker_level;

    // No scaling for equal levels
    if (level_diff == 0) return 1.0f;

    // Bonus XP for fighting higher level enemies
    if (level_diff > 0) {
        // Up to 2x XP for enemies 10+ levels higher
        return std::min(2.0f, 1.0f + (level_diff * 0.1f));
    }

    // Reduced XP for fighting lower level enemies
    // Minimum 10% XP for enemies 10+ levels lower
    float reduction = 1.0f - (std::abs(level_diff) * 0.09f);
    return std::max(0.1f, reduction);
}

void ExperienceSystem::awardQuestXP(Entity* entity, int quest_level, int base_xp) {
    if (!entity) return;

    auto* exp = entity->getComponent<ExperienceComponent>();
    if (!exp) return;

    // Apply level scaling to quest XP
    float scaling = getXPScaling(exp->level, quest_level);
    int final_xp = static_cast<int>(base_xp * scaling);

    awardExperience(entity, final_xp);

    if (logger) {
        auto* renderable = entity->getComponent<RenderableComponent>();
        std::string name = renderable ? renderable->name : "Entity";
        std::stringstream msg;
        msg << name << " completed quest for " << final_xp << " XP";
        logger->logSystem(msg.str());
    }
}

void ExperienceSystem::awardExplorationXP(Entity* entity, int discovery_value) {
    if (!entity) return;

    // Small XP reward for exploration
    int xp = discovery_value * 5;
    awardExperience(entity, xp);

    if (logger) {
        auto* renderable = entity->getComponent<RenderableComponent>();
        std::string name = renderable ? renderable->name : "Entity";
        std::stringstream msg;
        msg << name << " gained " << xp << " XP from exploration";
        logger->logSystem(msg.str());
    }
}

void ExperienceSystem::applyStatIncreases(Entity* entity, int levels) {
    if (!entity || levels <= 0) return;

    auto* stats = entity->getComponent<StatsComponent>();
    if (!stats) return;

    // Every level gives small stat increases
    for (int i = 0; i < levels; i++) {
        // Alternate stat increases or based on class
        stats->strength += 1;
        stats->constitution += 1;

        // Every other level
        if ((stats->strength + stats->constitution) % 2 == 0) {
            stats->dexterity += 1;
        }

        // Every third level
        if ((stats->strength + stats->constitution) % 3 == 0) {
            stats->intelligence += 1;
            stats->wisdom += 1;
        }
    }

    stats->recalculateDerived();
}

void ExperienceSystem::applyResourceIncreases(Entity* entity, int levels) {
    if (!entity || levels <= 0) return;

    auto* health = entity->getComponent<HealthComponent>();
    if (health) {
        auto* stats = entity->getComponent<StatsComponent>();
        int con_bonus = stats ? (stats->constitution - 10) / 2 : 0;

        // Increase max health based on constitution
        int health_gain = levels * (5 + con_bonus);
        health->max_hp += health_gain;
        health->hp += health_gain;  // Heal on level up

        // Increase max mana based on intelligence
        if (stats) {
            [[maybe_unused]] int mana_gain = levels * (3 + (stats->intelligence - 10) / 2);
            // Would need mana component
        }
    }
}

void ExperienceSystem::unlockAbilities(Entity* entity, int new_level) {
    if (!entity) return;

    // Unlock abilities at specific levels
    struct AbilityUnlock {
        int level;
        std::string ability;
    };

    static const AbilityUnlock unlocks[] = {
        {5, "Power Strike"},
        {10, "Whirlwind"},
        {15, "Defensive Stance"},
        {20, "Berserk"},
        {25, "Master Strike"}
    };

    for (const auto& unlock : unlocks) {
        if (new_level == unlock.level) {
            if (logger) {
                auto* renderable = entity->getComponent<RenderableComponent>();
                std::string name = renderable ? renderable->name : "Entity";
                std::stringstream msg;
                msg << name << " learned " << unlock.ability << "!";
                logger->logSystem(msg.str());
            }
            // Would need ability system to actually grant ability
        }
    }
}

int ExperienceSystem::calculateBaseXP(const Entity* entity) const {
    if (!entity) return 0;

    int base_xp = 10;  // Default XP

    // Base on entity level
    auto* exp = entity->getComponent<ExperienceComponent>();
    if (exp) {
        base_xp = 10 * exp->level;
    }

    // Bonus for special enemies
    if (entity->hasTag("boss")) {
        base_xp *= 10;
    } else if (entity->hasTag("elite")) {
        base_xp *= 3;
    } else if (entity->hasTag("rare")) {
        base_xp *= 2;
    }

    // Factor in combat stats
    auto* combat = entity->getComponent<CombatComponent>();
    if (combat) {
        base_xp += combat->damage_modifier * 2;
        base_xp += combat->defense_bonus;
    }

    return base_xp;
}

} // namespace ecs