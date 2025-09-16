/**
 * @file experience_system.h
 * @brief System for managing experience and leveling
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "system.h"
#include "entity.h"
#include "experience_component.h"
#include "stats_component.h"
#include "health_component.h"
#include "combat_component.h"
#include "logger_interface.h"
#include <memory>
#include <functional>

namespace ecs {

/**
 * @class ExperienceSystem
 * @brief Manages experience gain and level progression
 */
class ExperienceSystem : public System<ExperienceSystem> {
public:
    ExperienceSystem(ILogger* logger = nullptr);
    ~ExperienceSystem() = default;

    void update(const std::vector<std::unique_ptr<Entity>>& entities, double delta_time) override;

    /**
     * @brief Award experience to an entity
     * @param entity Entity receiving XP
     * @param amount XP amount
     * @return Number of levels gained
     */
    int awardExperience(Entity* entity, int amount);

    /**
     * @brief Calculate XP reward for defeating an enemy
     * @param killer Entity that killed
     * @param victim Entity that was killed
     * @return XP amount
     */
    int calculateKillXP(const Entity* killer, const Entity* victim) const;

    /**
     * @brief Apply level up bonuses
     * @param entity Entity leveling up
     * @param levels Number of levels gained
     */
    void applyLevelUp(Entity* entity, int levels);

    /**
     * @brief Spend skill points on ability
     * @param entity Entity spending points
     * @param skill_id Skill identifier
     * @param points Points to spend
     * @return true if successful
     */
    bool spendSkillPoints(Entity* entity, const std::string& skill_id, int points);

    /**
     * @brief Spend stat points on attribute
     * @param entity Entity spending points
     * @param stat Stat to increase
     * @param points Points to spend
     * @return true if successful
     */
    bool spendStatPoints(Entity* entity, const std::string& stat, int points);

    /**
     * @brief Get experience scaling factor for level difference
     * @param attacker_level Attacker's level
     * @param defender_level Defender's level
     * @return XP multiplier (0.0 - 2.0)
     */
    float getXPScaling(int attacker_level, int defender_level) const;

    /**
     * @brief Award quest experience
     * @param entity Entity completing quest
     * @param quest_level Quest difficulty level
     * @param base_xp Base XP reward
     */
    void awardQuestXP(Entity* entity, int quest_level, int base_xp);

    /**
     * @brief Award exploration experience
     * @param entity Entity exploring
     * @param discovery_value Value of discovery
     */
    void awardExplorationXP(Entity* entity, int discovery_value);

    /**
     * @brief Set level up callback
     * @param callback Function to call on level up
     */
    void setLevelUpCallback(std::function<void(Entity*, int)> callback) {
        level_up_callback = callback;
    }

    int getPriority() const override { return 35; }

    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<ExperienceComponent>();
    }

private:
    ILogger* logger;
    std::function<void(Entity*, int)> level_up_callback;

    /**
     * @brief Apply stat increases for level up
     * @param entity Entity leveling up
     * @param levels Number of levels
     */
    void applyStatIncreases(Entity* entity, int levels);

    /**
     * @brief Apply health/mana increases for level up
     * @param entity Entity leveling up
     * @param levels Number of levels
     */
    void applyResourceIncreases(Entity* entity, int levels);

    /**
     * @brief Unlock abilities at certain levels
     * @param entity Entity leveling up
     * @param new_level New level reached
     */
    void unlockAbilities(Entity* entity, int new_level);

    /**
     * @brief Calculate base XP for enemy
     * @param entity Enemy entity
     * @return Base XP value
     */
    int calculateBaseXP(const Entity* entity) const;
};

} // namespace ecs