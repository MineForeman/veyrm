/**
 * @file player_component.h
 * @brief Player-specific component for ECS
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"

namespace ecs {

/**
 * @class PlayerComponent
 * @brief Marks an entity as the player and stores player-specific data
 */
class PlayerComponent : public Component<PlayerComponent> {
public:
    // Player-specific stats
    int level = 1;
    int experience = 0;
    int gold = 0;

    // Experience thresholds
    int exp_to_next_level = 100;

    PlayerComponent() = default;

    /**
     * @brief Award experience points
     * @param amount Experience to add
     * @return true if leveled up
     */
    bool gainExperience(int amount) {
        experience += amount;
        bool leveled = false;

        while (experience >= exp_to_next_level) {
            experience -= exp_to_next_level;
            level++;
            exp_to_next_level = calculateExpForLevel(level);
            leveled = true;
        }

        return leveled;
    }

    /**
     * @brief Add gold
     * @param amount Gold to add
     */
    void addGold(int amount) {
        gold += amount;
        if (gold < 0) gold = 0;
    }

    /**
     * @brief Calculate experience required for a level
     * @param lvl Level to calculate for
     * @return Experience required
     */
    static int calculateExpForLevel(int lvl) {
        return 100 * lvl * lvl;  // Quadratic scaling
    }

    std::string getTypeName() const override { return "PlayerComponent"; }
    ComponentType getType() const override { return ComponentType::PLAYER; }
};

} // namespace ecs