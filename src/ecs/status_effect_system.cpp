/**
 * @file status_effect_system.cpp
 * @brief Implementation of status effect system
 */

#include <sstream>
#include "ecs/status_effect_system.h"
#include "ecs/renderable_component.h"

namespace ecs {

StatusEffectSystem::StatusEffectSystem(ILogger* logger)
    : logger(logger) {
}

void StatusEffectSystem::update(const std::vector<std::unique_ptr<Entity>>& entities, double) {
    for (const auto& entity : entities) {
        if (!shouldProcess(*entity)) continue;

        auto* effects = entity->getComponent<EffectsComponent>();
        if (!effects) continue;

        // Process damage over time
        processDamageOverTime(entity.get(), effects);

        // Apply stat modifiers
        applyStatModifiers(entity.get(), effects);

        // Process special effects
        for (const auto& effect : effects->active_effects) {
            processSpecialEffect(entity.get(), effect);
        }

        // Update effect durations
        effects->updateEffects();
    }
}

bool StatusEffectSystem::applyEffect(Entity* entity, const StatusEffect& effect) {
    if (!entity) return false;

    auto* effects = entity->getComponent<EffectsComponent>();
    if (!effects) {
        entity->addComponent<EffectsComponent>();
        effects = entity->getComponent<EffectsComponent>();
    }

    effects->addEffect(effect);

    if (logger) {
        auto* renderable = entity->getComponent<RenderableComponent>();
        std::string name = renderable ? renderable->name : "Entity";
        std::stringstream msg;
        msg << name << " is affected by " << effect.name;
        logger->logSystem(msg.str());
    }

    return true;
}

bool StatusEffectSystem::removeEffect(Entity* entity, EffectType type) {
    if (!entity) return false;

    auto* effects = entity->getComponent<EffectsComponent>();
    if (!effects) return false;

    effects->removeEffect(type);
    return true;
}

void StatusEffectSystem::clearEffects(Entity* entity) {
    if (!entity) return;

    auto* effects = entity->getComponent<EffectsComponent>();
    if (effects) {
        effects->active_effects.clear();
    }
}

StatusEffect StatusEffectSystem::createEffect(EffectType type, int duration, int power) {
    StatusEffect effect(type, "", duration, power);

    switch (type) {
        case EffectType::POISON:
            effect.name = "Poisoned";
            effect.tick_damage = power;
            break;
        case EffectType::BURN:
            effect.name = "Burning";
            effect.tick_damage = power * 2;
            break;
        case EffectType::REGENERATION:
            effect.name = "Regenerating";
            effect.tick_damage = -power;  // Negative for healing
            break;
        case EffectType::HASTE:
            effect.name = "Hasted";
            effect.speed_mod = power;
            break;
        case EffectType::SLOW:
            effect.name = "Slowed";
            effect.speed_mod = -power;
            break;
        case EffectType::BUFF:
            effect.name = "Buffed";
            effect.strength_mod = power;
            effect.dexterity_mod = power;
            break;
        case EffectType::DEBUFF:
            effect.name = "Debuffed";
            effect.strength_mod = -power;
            effect.dexterity_mod = -power;
            break;
        case EffectType::SHIELD:
            effect.name = "Shielded";
            effect.armor_mod = power * 2;
            break;
        default:
            effect.name = "Unknown Effect";
            break;
    }

    return effect;
}

void StatusEffectSystem::processDamageOverTime(Entity* entity, EffectsComponent* effects) {
    auto* health = entity->getComponent<HealthComponent>();
    if (!health) return;

    int total_tick_damage = 0;
    for (const auto& effect : effects->active_effects) {
        total_tick_damage += effect.tick_damage;
    }

    if (total_tick_damage != 0) {
        if (total_tick_damage > 0) {
            // Damage
            health->takeDamage(total_tick_damage);
            if (logger) {
                auto* renderable = entity->getComponent<RenderableComponent>();
                std::string name = renderable ? renderable->name : "Entity";
                std::stringstream msg;
                msg << name << " takes " << total_tick_damage << " damage from effects";
                logger->logCombat(msg.str());
            }
        } else {
            // Healing
            health->heal(-total_tick_damage);
            if (logger) {
                auto* renderable = entity->getComponent<RenderableComponent>();
                std::string name = renderable ? renderable->name : "Entity";
                std::stringstream msg;
                msg << name << " heals " << (-total_tick_damage) << " from regeneration";
                logger->logCombat(msg.str());
            }
        }
    }
}

void StatusEffectSystem::applyStatModifiers(Entity* entity, EffectsComponent* effects) {
    auto* stats = entity->getComponent<StatsComponent>();
    if (!stats) return;

    // Apply temporary modifiers (would need to track base vs modified stats)
    // For now, just recalculate derived stats
    stats->recalculateDerived();

    auto* combat = entity->getComponent<CombatComponent>();
    if (combat) {
        // Apply combat modifiers
        combat->attack_bonus = stats->accuracy_bonus + effects->getTotalStatModifier("strength");
        combat->defense_bonus = stats->armor_class + effects->getTotalStatModifier("armor");
    }
}

void StatusEffectSystem::processSpecialEffect(Entity*, const StatusEffect& effect) {
    switch (effect.type) {
        case EffectType::STUN:
            // Entity can't act (would need to integrate with turn system)
            break;
        case EffectType::BLIND:
            // Reduce vision range (would need FOV component)
            break;
        case EffectType::INVISIBLE:
            // Make entity invisible (would need visibility flag)
            break;
        case EffectType::CONFUSED:
            // Random movement (would need AI modification)
            break;
        default:
            break;
    }
}

} // namespace ecs