/**
 * @file stats_component.h
 * @brief RPG statistics component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include <algorithm>

namespace ecs {

/**
 * @class StatsComponent
 * @brief Manages entity RPG statistics
 */
class StatsComponent : public Component<StatsComponent> {
public:
    // Primary stats
    int strength = 10;
    int dexterity = 10;
    int intelligence = 10;
    int constitution = 10;
    int wisdom = 10;
    int charisma = 10;

    // Derived stats (calculated from primary)
    int accuracy_bonus = 0;    ///< Hit chance modifier
    int damage_bonus = 0;       ///< Damage modifier
    int armor_class = 10;       ///< Defense rating
    int spell_power = 0;        ///< Magic damage modifier
    int resistance = 0;         ///< Magic resistance

    // Resource pools
    int mana = 0;
    int max_mana = 0;
    int stamina = 100;
    int max_stamina = 100;

    StatsComponent() = default;

    StatsComponent(int str, int dex, int intel, int con, int wis, int cha)
        : strength(str), dexterity(dex), intelligence(intel),
          constitution(con), wisdom(wis), charisma(cha) {
        recalculateDerived();
    }

    /**
     * @brief Recalculate derived stats from primary stats
     */
    void recalculateDerived() {
        accuracy_bonus = (dexterity - 10) / 2;
        damage_bonus = (strength - 10) / 2;
        armor_class = 10 + (dexterity - 10) / 2;
        spell_power = (intelligence - 10) / 2;
        resistance = (wisdom - 10) / 2;
        max_mana = intelligence * 3;
        max_stamina = constitution * 10;

        // Ensure current values don't exceed max
        mana = std::min(mana, max_mana);
        stamina = std::min(stamina, max_stamina);
    }

    /**
     * @brief Get modifier for a stat (D&D style)
     */
    static int getModifier(int stat) {
        return (stat - 10) / 2;
    }

    std::string getTypeName() const override { return "StatsComponent"; }
    ComponentType getType() const override { return ComponentType::STATS; }
};

} // namespace ecs