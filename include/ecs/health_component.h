/**
 * @file health_component.h
 * @brief Health and damage tracking component for entities
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include <algorithm>

namespace ecs {

/**
 * @class HealthComponent
 * @brief Manages entity health, damage, and death state
 *
 * This component tracks current and maximum health,
 * handles damage/healing, and determines death state.
 */
class HealthComponent : public Component<HealthComponent> {
public:
    /**
     * @brief Construct health component
     * @param max_hp Maximum hit points
     * @param current_hp Current hit points (defaults to max)
     */
    explicit HealthComponent(int max_hp = 10, int current_hp = -1)
        : hp(current_hp < 0 ? max_hp : current_hp), max_hp(max_hp) {}

    ComponentType getType() const override {
        return ComponentType::HEALTH;
    }

    std::string getTypeName() const override {
        return "HealthComponent";
    }

    /**
     * @brief Apply damage to entity
     * @param amount Damage amount to apply
     * @return Actual damage dealt (after clamping)
     */
    int takeDamage(int amount) {
        int old_hp = hp;
        hp = std::max(0, hp - amount);
        return old_hp - hp;
    }

    /**
     * @brief Heal entity
     * @param amount Healing amount to apply
     * @return Actual healing done (after clamping)
     */
    int heal(int amount) {
        int old_hp = hp;
        hp = std::min(max_hp, hp + amount);
        return hp - old_hp;
    }

    /**
     * @brief Set current health directly
     * @param new_hp New health value (clamped to valid range)
     */
    void setHealth(int new_hp) {
        hp = std::clamp(new_hp, 0, max_hp);
    }

    /**
     * @brief Set maximum health
     * @param new_max New maximum health
     * @param heal_to_max If true, also set current HP to new max
     */
    void setMaxHealth(int new_max, bool heal_to_max = false) {
        max_hp = std::max(1, new_max);
        if (heal_to_max) {
            hp = max_hp;
        } else {
            hp = std::min(hp, max_hp);
        }
    }

    /**
     * @brief Check if entity is alive
     * @return true if HP > 0
     */
    bool isAlive() const { return hp > 0; }

    /**
     * @brief Check if entity is dead
     * @return true if HP <= 0
     */
    bool isDead() const { return hp <= 0; }

    /**
     * @brief Check if entity is at full health
     * @return true if HP == max HP
     */
    bool isFullHealth() const { return hp >= max_hp; }

    /**
     * @brief Get health percentage
     * @return Health as percentage (0-100)
     */
    int getHealthPercent() const {
        return max_hp > 0 ? (hp * 100 / max_hp) : 0;
    }

    /**
     * @brief Get current health
     * @return Current hit points
     */
    int getHealth() const { return hp; }

    /**
     * @brief Get maximum health
     * @return Maximum hit points
     */
    int getMaxHealth() const { return max_hp; }

public:
    // Public data for easy access
    int hp;        ///< Current hit points
    int max_hp;    ///< Maximum hit points

    // Optional health modifiers
    int temp_hp = 0;           ///< Temporary hit points
    int damage_reduction = 0;  ///< Flat damage reduction
    float damage_resistance = 0.0f; ///< Percentage damage reduction (0.0-1.0)
    bool invulnerable = false; ///< Cannot take damage
};

} // namespace ecs