/**
 * @file combat_component.h
 * @brief Combat statistics and abilities component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include <string>

namespace ecs {

/**
 * @class CombatComponent
 * @brief Manages combat statistics and abilities
 *
 * This component stores all combat-related data including
 * attack/defense bonuses, damage ranges, and combat flags.
 */
class CombatComponent : public Component<CombatComponent> {
public:
    /**
     * @brief Construct combat component with basic stats
     * @param base_damage Base damage dealt
     * @param attack_bonus Bonus to attack rolls
     * @param defense_bonus Bonus to defense/AC
     */
    CombatComponent(int base_damage = 1, int attack_bonus = 0, int defense_bonus = 0)
        : base_damage(base_damage),
          min_damage(base_damage),
          max_damage(base_damage),
          attack_bonus(attack_bonus),
          defense_bonus(defense_bonus) {}

    ComponentType getType() const override {
        return ComponentType::COMBAT;
    }

    std::string getTypeName() const override {
        return "CombatComponent";
    }

    /**
     * @brief Set damage range for attacks
     * @param min Minimum damage
     * @param max Maximum damage
     */
    void setDamageRange(int min, int max) {
        min_damage = min;
        max_damage = max;
        base_damage = (min + max) / 2;  // Average for base
    }

    /**
     * @brief Get total attack bonus
     * @return Attack bonus including modifiers
     */
    int getTotalAttackBonus() const {
        return attack_bonus + attack_modifier;
    }

    /**
     * @brief Get total defense bonus
     * @return Defense bonus including modifiers
     */
    int getTotalDefenseBonus() const {
        return defense_bonus + defense_modifier;
    }

    /**
     * @brief Check if entity can attack
     * @return true if entity is capable of attacking
     */
    bool canAttack() const {
        return can_attack && !is_stunned && !is_paralyzed;
    }

    /**
     * @brief Check if entity can defend
     * @return true if entity can defend against attacks
     */
    bool canDefend() const {
        return !is_stunned && !is_paralyzed && !is_sleeping;
    }

public:
    // Core combat stats
    int base_damage;       ///< Base damage for attacks
    int min_damage;        ///< Minimum damage roll
    int max_damage;        ///< Maximum damage roll
    int attack_bonus;      ///< Bonus to attack rolls
    int defense_bonus;     ///< Bonus to defense/AC

    // Temporary modifiers
    int attack_modifier = 0;   ///< Temporary attack bonus/penalty
    int defense_modifier = 0;  ///< Temporary defense bonus/penalty
    int damage_modifier = 0;   ///< Temporary damage bonus/penalty

    // Combat flags
    bool can_attack = true;    ///< Entity is capable of attacking
    bool can_crit = true;      ///< Can score critical hits
    bool can_dodge = true;     ///< Can dodge attacks
    bool can_block = false;    ///< Can block attacks
    bool can_parry = false;    ///< Can parry attacks

    // Status effects
    bool is_stunned = false;   ///< Cannot act
    bool is_paralyzed = false; ///< Cannot move or act
    bool is_sleeping = false;  ///< Asleep (awakens on damage)
    bool is_confused = false;  ///< Random actions
    bool is_berserk = false;   ///< +damage, -defense

    // Combat state
    int combo_counter = 0;     ///< For combo attacks
    int dodge_charges = 0;     ///< Number of dodges available
    int block_charges = 0;     ///< Number of blocks available

    // Combat text
    std::string combat_name;   ///< Name shown in combat messages
    std::string attack_verb = "attacks";  ///< Verb for attack messages
    std::string damage_type = "physical"; ///< Type of damage dealt
};

} // namespace ecs