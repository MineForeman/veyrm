/**
 * @file effects_component.h
 * @brief Status effects and buffs/debuffs component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

namespace ecs {

/**
 * @enum EffectType
 * @brief Types of status effects
 */
enum class EffectType {
    BUFF,
    DEBUFF,
    POISON,
    BURN,
    FREEZE,
    STUN,
    BLIND,
    SLOW,
    HASTE,
    REGENERATION,
    SHIELD,
    INVISIBLE,
    CONFUSED
};

/**
 * @struct StatusEffect
 * @brief Individual status effect
 */
struct StatusEffect {
    EffectType type;
    std::string name;
    int duration;           ///< Turns remaining (-1 for permanent)
    int power;              ///< Effect strength
    int tick_damage = 0;    ///< Damage per turn (for DoT effects)

    // Stat modifiers
    int strength_mod = 0;
    int dexterity_mod = 0;
    int armor_mod = 0;
    int speed_mod = 0;

    StatusEffect(EffectType t, const std::string& n, int dur, int pow)
        : type(t), name(n), duration(dur), power(pow) {}

    bool isExpired() const { return duration == 0; }
    void tick() { if (duration > 0) duration--; }
};

/**
 * @class EffectsComponent
 * @brief Manages status effects on an entity
 */
class EffectsComponent : public Component<EffectsComponent> {
public:
    std::vector<StatusEffect> active_effects;

    // Immunity flags
    bool immune_to_poison = false;
    bool immune_to_stun = false;
    bool immune_to_slow = false;

    EffectsComponent() = default;

    /**
     * @brief Add a status effect
     */
    void addEffect(const StatusEffect& effect) {
        // Check immunities
        if ((effect.type == EffectType::POISON && immune_to_poison) ||
            (effect.type == EffectType::STUN && immune_to_stun) ||
            (effect.type == EffectType::SLOW && immune_to_slow)) {
            return;
        }

        // Check if effect already exists and refresh duration
        for (auto& e : active_effects) {
            if (e.type == effect.type) {
                e.duration = std::max(e.duration, effect.duration);
                return;
            }
        }

        active_effects.push_back(effect);
    }

    /**
     * @brief Remove an effect by type
     */
    void removeEffect(EffectType type) {
        active_effects.erase(
            std::remove_if(active_effects.begin(), active_effects.end(),
                [type](const StatusEffect& e) { return e.type == type; }),
            active_effects.end()
        );
    }

    /**
     * @brief Check if entity has a specific effect
     */
    bool hasEffect(EffectType type) const {
        return std::any_of(active_effects.begin(), active_effects.end(),
            [type](const StatusEffect& e) { return e.type == type; });
    }

    /**
     * @brief Update all effects (call each turn)
     */
    void updateEffects() {
        for (auto& effect : active_effects) {
            effect.tick();
        }

        // Remove expired effects
        active_effects.erase(
            std::remove_if(active_effects.begin(), active_effects.end(),
                [](const StatusEffect& e) { return e.isExpired(); }),
            active_effects.end()
        );
    }

    /**
     * @brief Get total stat modifier from all effects
     */
    int getTotalStatModifier(const std::string& stat) const {
        int total = 0;
        for (const auto& effect : active_effects) {
            if (stat == "strength") total += effect.strength_mod;
            else if (stat == "dexterity") total += effect.dexterity_mod;
            else if (stat == "armor") total += effect.armor_mod;
            else if (stat == "speed") total += effect.speed_mod;
        }
        return total;
    }

    std::string getTypeName() const override { return "EffectsComponent"; }
    ComponentType getType() const override { return ComponentType::CUSTOM; }
};

} // namespace ecs