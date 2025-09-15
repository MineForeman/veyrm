/**
 * @file experience_component.h
 * @brief Experience and leveling component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include <cmath>

namespace ecs {

/**
 * @class ExperienceComponent
 * @brief Manages entity experience and leveling
 */
class ExperienceComponent : public Component<ExperienceComponent> {
public:
    int level = 1;                  ///< Current level
    int experience = 0;             ///< Current experience points
    int experience_to_next = 100;  ///< XP needed for next level
    int total_experience = 0;       ///< Total XP earned (lifetime)
    int skill_points = 0;           ///< Unspent skill points
    int stat_points = 0;            ///< Unspent stat points

    // Level progression settings
    float xp_multiplier = 1.5f;     ///< XP requirement multiplier per level
    int base_xp_required = 100;    ///< Base XP for level 2

    ExperienceComponent() = default;

    /**
     * @brief Add experience points
     * @param xp Amount of XP to add
     * @return Number of levels gained
     */
    int addExperience(int xp) {
        int levels_gained = 0;
        experience += xp;
        total_experience += xp;

        // Check for level up
        while (experience >= experience_to_next) {
            experience -= experience_to_next;
            level++;
            levels_gained++;

            // Calculate XP for next level
            experience_to_next = calculateXPRequired(level + 1);

            // Award points for leveling
            skill_points += getSkillPointsPerLevel();
            stat_points += getStatPointsPerLevel();
        }

        return levels_gained;
    }

    /**
     * @brief Calculate XP required for a specific level
     * @param target_level Target level
     * @return XP required
     */
    int calculateXPRequired(int target_level) const {
        if (target_level <= 1) return 0;
        return static_cast<int>(base_xp_required * std::pow(xp_multiplier, target_level - 2));
    }

    /**
     * @brief Get total XP required from level 1 to current level
     * @return Total XP
     */
    int getTotalXPToCurrentLevel() const {
        int total = 0;
        for (int i = 2; i <= level; i++) {
            total += calculateXPRequired(i);
        }
        return total;
    }

    /**
     * @brief Get progress to next level as percentage
     * @return Progress percentage (0-100)
     */
    float getLevelProgress() const {
        if (experience_to_next <= 0) return 100.0f;
        return (static_cast<float>(experience) / experience_to_next) * 100.0f;
    }

    /**
     * @brief Get skill points awarded per level
     * @return Skill points (can vary by level ranges)
     */
    int getSkillPointsPerLevel() const {
        if (level < 10) return 1;
        if (level < 20) return 2;
        if (level < 30) return 3;
        return 4;
    }

    /**
     * @brief Get stat points awarded per level
     * @return Stat points
     */
    int getStatPointsPerLevel() const {
        return (level % 5 == 0) ? 3 : 1;  // Bonus points every 5 levels
    }

    /**
     * @brief Spend skill points
     * @param amount Amount to spend
     * @return true if successful
     */
    bool spendSkillPoints(int amount) {
        if (skill_points >= amount) {
            skill_points -= amount;
            return true;
        }
        return false;
    }

    /**
     * @brief Spend stat points
     * @param amount Amount to spend
     * @return true if successful
     */
    bool spendStatPoints(int amount) {
        if (stat_points >= amount) {
            stat_points -= amount;
            return true;
        }
        return false;
    }

    std::string getTypeName() const override { return "ExperienceComponent"; }
    ComponentType getType() const override { return ComponentType::CUSTOM; }
};

} // namespace ecs