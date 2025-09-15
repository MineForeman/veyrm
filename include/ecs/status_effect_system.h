/**
 * @file status_effect_system.h
 * @brief System for managing status effects and buffs/debuffs
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "system.h"
#include "entity.h"
#include "effects_component.h"
#include "health_component.h"
#include "stats_component.h"
#include "combat_component.h"
#include "logger_interface.h"
#include <memory>
#include <string>

namespace ecs {

/**
 * @class StatusEffectSystem
 * @brief Manages status effects on entities
 */
class StatusEffectSystem : public System<StatusEffectSystem> {
public:
    StatusEffectSystem(ILogger* logger = nullptr);
    ~StatusEffectSystem() = default;

    void update(const std::vector<std::unique_ptr<Entity>>& entities, double delta_time) override;

    /**
     * @brief Apply a status effect to an entity
     * @param entity Target entity
     * @param effect Effect to apply
     * @return true if applied successfully
     */
    bool applyEffect(Entity* entity, const StatusEffect& effect);

    /**
     * @brief Remove a status effect from an entity
     * @param entity Target entity
     * @param type Effect type to remove
     * @return true if removed successfully
     */
    bool removeEffect(Entity* entity, EffectType type);

    /**
     * @brief Clear all effects from an entity
     * @param entity Target entity
     */
    void clearEffects(Entity* entity);

    /**
     * @brief Create a common status effect
     * @param type Effect type
     * @param duration Duration in turns
     * @param power Effect power
     * @return Status effect
     */
    static StatusEffect createEffect(EffectType type, int duration, int power);

    int getPriority() const override { return 25; }

    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<EffectsComponent>();
    }

private:
    ILogger* logger;

    /**
     * @brief Process damage over time effects
     * @param entity Entity with effects
     * @param effects Effects component
     */
    void processDamageOverTime(Entity* entity, EffectsComponent* effects);

    /**
     * @brief Apply stat modifiers from effects
     * @param entity Entity with effects
     * @param effects Effects component
     */
    void applyStatModifiers(Entity* entity, EffectsComponent* effects);

    /**
     * @brief Process special effect behaviors
     * @param entity Entity with effects
     * @param effect Individual effect
     */
    void processSpecialEffect(Entity* entity, const StatusEffect& effect);
};

} // namespace ecs