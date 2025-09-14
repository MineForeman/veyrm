/**
 * @file entity_adapter.h
 * @brief Adapter to bridge old Entity system with new ECS
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "entity.h"
#include "entity_factory.h"
#include "position_component.h"
#include "renderable_component.h"
#include "health_component.h"
#include "combat_component.h"
#include "../entity.h"  // Old entity system
#include "../player.h"
#include "../monster.h"
#include "../item.h"
#include <memory>
#include <algorithm>

namespace ecs {

/**
 * @class EntityAdapter
 * @brief Converts between old Entity system and new ECS
 *
 * Provides bidirectional conversion between the inheritance-based
 * Entity system and the component-based ECS. This allows gradual
 * migration of the codebase.
 */
class EntityAdapter {
public:
    /**
     * @brief Convert old Entity to ECS Entity
     * @param old_entity The legacy entity to convert
     * @return Unique pointer to ECS entity with equivalent components
     */
    static std::unique_ptr<ecs::Entity> fromLegacyEntity(const ::Entity& old_entity) {
        EntityBuilder builder;

        // Position component
        builder.withPosition(old_entity.x, old_entity.y);

        // Renderable component
        builder.withRenderable(old_entity.glyph, old_entity.color);
        if (auto* render = builder.build()->getComponent<RenderableComponent>()) {
            render->setVisible(old_entity.isVisible());
            render->blocks_sight = old_entity.blocks_sight;
        }
        builder.reset();  // Reset for continued building

        // Rebuild with all components
        auto entity = std::make_unique<ecs::Entity>();

        // Position
        entity->addComponent<PositionComponent>(old_entity.x, old_entity.y);

        // Renderable
        auto& render = entity->addComponent<RenderableComponent>(
            old_entity.glyph, old_entity.color, old_entity.isVisible());
        render.blocks_sight = old_entity.blocks_sight;

        // Health (if has HP)
        if (old_entity.max_hp > 0) {
            entity->addComponent<HealthComponent>(old_entity.max_hp, old_entity.hp);
        }

        // Combat (if can fight)
        if (old_entity.is_player || old_entity.is_monster) {
            auto& combat = entity->addComponent<CombatComponent>(
                old_entity.getBaseDamage(),
                old_entity.getAttackBonus(),
                old_entity.getDefenseBonus()
            );
            combat.combat_name = old_entity.getCombatName();
        }

        return entity;
    }

    /**
     * @brief Convert Player to ECS Entity
     * @param player The player to convert
     * @return Unique pointer to ECS entity with player components
     */
    static std::unique_ptr<ecs::Entity> fromPlayer(const Player& player) {
        auto entity = std::make_unique<Entity>();

        // Core components
        entity->addComponent<PositionComponent>(player.x, player.y);
        entity->addComponent<RenderableComponent>(player.glyph, player.color);
        entity->addComponent<HealthComponent>(player.max_hp, player.hp);

        // Combat with player stats
        auto& combat = entity->addComponent<CombatComponent>(
            player.getBaseDamage(),
            player.getAttackBonus(),
            player.getDefenseBonus()
        );
        combat.combat_name = "Player";
        combat.setDamageRange(1, 6);  // Player uses d6

        return entity;
    }

    /**
     * @brief Convert Monster to ECS Entity
     * @param monster The monster to convert
     * @return Unique pointer to ECS entity with monster components
     */
    static std::unique_ptr<ecs::Entity> fromMonster(const Monster& monster) {
        auto entity = std::make_unique<Entity>();

        // Core components
        entity->addComponent<PositionComponent>(monster.x, monster.y);
        entity->addComponent<RenderableComponent>(monster.glyph, monster.color);
        entity->addComponent<HealthComponent>(monster.max_hp, monster.hp);

        // Combat with monster stats
        auto& combat = entity->addComponent<CombatComponent>(
            monster.getBaseDamage(),
            monster.getAttackBonus(),
            monster.getDefenseBonus()
        );
        combat.combat_name = monster.name;
        // Monsters in legacy system use base damage from Entity
        // We'll use a simple range based on attack stat
        int base_dmg = monster.getBaseDamage();
        combat.setDamageRange(std::max(1, base_dmg - 1), base_dmg + 2);

        return entity;
    }

    /**
     * @brief Convert Item to ECS Entity
     * @param item The item to convert
     * @return Unique pointer to ECS entity with item components
     */
    static std::unique_ptr<ecs::Entity> fromItem(const Item& item) {
        auto entity = std::make_unique<ecs::Entity>();

        // Items typically just need position and rendering
        entity->addComponent<PositionComponent>(item.x, item.y);
        // Item has symbol and color string (need to convert)
        std::string glyph(1, item.symbol);  // Convert char to string
        entity->addComponent<RenderableComponent>(glyph, ftxui::Color::White);

        // Items don't usually have health or combat
        // Item-specific data would go in an ItemComponent (future)

        return entity;
    }

    /**
     * @brief Update legacy Entity position from ECS Entity
     * @param ecs_entity Source ECS entity
     * @param legacy_entity Target legacy entity to update
     */
    static void updatePosition(const ecs::Entity& ecs_entity, ::Entity& legacy_entity) {
        if (auto* pos = ecs_entity.getComponent<PositionComponent>()) {
            legacy_entity.x = pos->position.x;
            legacy_entity.y = pos->position.y;
            legacy_entity.prev_x = pos->previous_position.x;
            legacy_entity.prev_y = pos->previous_position.y;
        }
    }

    /**
     * @brief Update legacy Entity health from ECS Entity
     * @param ecs_entity Source ECS entity
     * @param legacy_entity Target legacy entity to update
     */
    static void updateHealth(const ecs::Entity& ecs_entity, ::Entity& legacy_entity) {
        if (auto* health = ecs_entity.getComponent<HealthComponent>()) {
            legacy_entity.hp = health->getHealth();
            legacy_entity.max_hp = health->getMaxHealth();
        }
    }

    /**
     * @brief Update legacy Entity rendering from ECS Entity
     * @param ecs_entity Source ECS entity
     * @param legacy_entity Target legacy entity to update
     */
    static void updateRendering(const ecs::Entity& ecs_entity, ::Entity& legacy_entity) {
        if (auto* render = ecs_entity.getComponent<RenderableComponent>()) {
            legacy_entity.glyph = render->glyph;
            legacy_entity.color = render->color;
            legacy_entity.setVisible(render->isVisible());
        }
    }

    /**
     * @brief Sync all changes from ECS to legacy entity
     * @param ecs_entity Source ECS entity
     * @param legacy_entity Target legacy entity to update
     */
    static void syncToLegacy(const ecs::Entity& ecs_entity, ::Entity& legacy_entity) {
        updatePosition(ecs_entity, legacy_entity);
        updateHealth(ecs_entity, legacy_entity);
        updateRendering(ecs_entity, legacy_entity);
    }

    /**
     * @brief Check if ECS entity represents a player
     * @param entity ECS entity to check
     * @return true if entity has player-like components
     */
    static bool isPlayer(const ecs::Entity& entity) {
        // Players have all components and specific combat name
        if (auto* combat = entity.getComponent<CombatComponent>()) {
            return combat->combat_name == "Player" &&
                   entity.hasComponent<HealthComponent>() &&
                   entity.hasComponent<PositionComponent>();
        }
        return false;
    }

    /**
     * @brief Check if ECS entity represents a monster
     * @param entity ECS entity to check
     * @return true if entity has monster-like components
     */
    static bool isMonster(const ecs::Entity& entity) {
        // Monsters have combat and health but aren't players
        return entity.hasComponent<CombatComponent>() &&
               entity.hasComponent<HealthComponent>() &&
               !isPlayer(entity);
    }

    /**
     * @brief Check if ECS entity represents an item
     * @param entity ECS entity to check
     * @return true if entity has item-like components
     */
    static bool isItem(const ecs::Entity& entity) {
        // Items have position and rendering but no combat/health
        return entity.hasComponent<PositionComponent>() &&
               entity.hasComponent<RenderableComponent>() &&
               !entity.hasComponent<CombatComponent>() &&
               !entity.hasComponent<HealthComponent>();
    }
};

/**
 * @class EntityMigrationHelper
 * @brief Utilities for migrating collections of entities
 */
class EntityMigrationHelper {
public:
    /**
     * @brief Convert all entities in a container to ECS
     * @tparam Container Container type (vector, list, etc.)
     * @param legacy_entities Container of legacy entity pointers
     * @return Vector of ECS entity unique pointers
     */
    template<typename Container>
    static std::vector<std::unique_ptr<ecs::Entity>> migrateAll(
        const Container& legacy_entities) {
        std::vector<std::unique_ptr<ecs::Entity>> ecs_entities;

        for (const auto& legacy : legacy_entities) {
            if (legacy) {
                ecs_entities.push_back(
                    EntityAdapter::fromLegacyEntity(*legacy));
            }
        }

        return ecs_entities;
    }

    /**
     * @brief Create a mapping between legacy and ECS entities
     * @tparam Container Container type
     * @param legacy_entities Container of legacy entity pointers
     * @return Map from legacy entity pointer to ECS entity
     */
    template<typename Container>
    static std::unordered_map<const ::Entity*, std::unique_ptr<ecs::Entity>>
    createMigrationMap(const Container& legacy_entities) {
        std::unordered_map<const ::Entity*, std::unique_ptr<ecs::Entity>> mapping;

        for (const auto& legacy : legacy_entities) {
            if (legacy) {
                mapping[legacy.get()] =
                    EntityAdapter::fromLegacyEntity(*legacy);
            }
        }

        return mapping;
    }
};

} // namespace ecs