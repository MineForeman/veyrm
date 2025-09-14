/**
 * @file entity_factory.h
 * @brief Factory for creating entities with predefined component configurations
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "entity.h"
#include "position_component.h"
#include "renderable_component.h"
#include "health_component.h"
#include "combat_component.h"
#include "ai_system.h"
#include "../color_scheme.h"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace ecs {

/**
 * @class EntityFactory
 * @brief Abstract factory for creating entities with components
 *
 * Provides a flexible way to create entities with predefined
 * component configurations. Supports both preset templates
 * and custom builders.
 */
class EntityFactory {
public:
    virtual ~EntityFactory() = default;

    /**
     * @brief Create entity at position
     * @param x X coordinate
     * @param y Y coordinate
     * @return Unique pointer to configured entity
     */
    virtual std::unique_ptr<Entity> create(int x, int y) = 0;

    /**
     * @brief Create entity with name at position
     * @param name Entity name/type
     * @param x X coordinate
     * @param y Y coordinate
     * @return Unique pointer to configured entity
     */
    virtual std::unique_ptr<Entity> create(const std::string& name, int x, int y) = 0;
};

/**
 * @class EntityBuilder
 * @brief Fluent builder for creating entities with components
 *
 * Provides a fluent interface for building entities with
 * various component configurations.
 */
class EntityBuilder {
public:
    EntityBuilder() : entity(std::make_unique<Entity>()) {}

    /**
     * @brief Add position component
     * @param x X coordinate
     * @param y Y coordinate
     * @return Builder reference for chaining
     */
    EntityBuilder& withPosition(int x, int y) {
        entity->addComponent<PositionComponent>(x, y);
        return *this;
    }

    /**
     * @brief Add renderable component
     * @param glyph Display character
     * @param color Display color
     * @return Builder reference for chaining
     */
    EntityBuilder& withRenderable(const std::string& glyph,
                                  ftxui::Color color = ftxui::Color::White) {
        entity->addComponent<RenderableComponent>(glyph, color);
        return *this;
    }

    /**
     * @brief Add health component
     * @param max_hp Maximum health
     * @param current_hp Current health (defaults to max)
     * @return Builder reference for chaining
     */
    EntityBuilder& withHealth(int max_hp, int current_hp = -1) {
        entity->addComponent<HealthComponent>(max_hp, current_hp);
        return *this;
    }

    /**
     * @brief Add combat component
     * @param damage Base damage
     * @param attack Attack bonus
     * @param defense Defense bonus
     * @return Builder reference for chaining
     */
    EntityBuilder& withCombat(int damage, int attack = 0, int defense = 0) {
        entity->addComponent<CombatComponent>(damage, attack, defense);
        return *this;
    }

    /**
     * @brief Add combat with damage range
     * @param min_damage Minimum damage
     * @param max_damage Maximum damage
     * @param attack Attack bonus
     * @param defense Defense bonus
     * @return Builder reference for chaining
     */
    EntityBuilder& withCombatRange(int min_damage, int max_damage,
                                   int attack = 0, int defense = 0) {
        auto& combat = entity->addComponent<CombatComponent>(
            (min_damage + max_damage) / 2, attack, defense);
        combat.setDamageRange(min_damage, max_damage);
        return *this;
    }

    /**
     * @brief Set combat name for messages
     * @param name Name shown in combat
     * @return Builder reference for chaining
     */
    EntityBuilder& withCombatName(const std::string& name) {
        if (auto* combat = entity->getComponent<CombatComponent>()) {
            combat->combat_name = name;
        }
        return *this;
    }

    /**
     * @brief Set entity as blocking movement
     * @return Builder reference for chaining
     */
    EntityBuilder& asBlocking() {
        // This will be used when we add physics component
        // For now, we can set a flag in renderable
        if (auto* render = entity->getComponent<RenderableComponent>()) {
            render->blocks_sight = true;
        }
        return *this;
    }

    /**
     * @brief Add AI component with behavior
     * @param behavior AI behavior type
     * @param vision_range How far the AI can see
     * @param aggro_range Range at which AI becomes hostile
     * @return Builder reference for chaining
     */
    EntityBuilder& withAI(AIBehavior behavior = AIBehavior::WANDERING,
                         int vision_range = 6,
                         int aggro_range = 4) {
        auto& ai = entity->addComponent<AIComponent>();
        ai.behavior = behavior;
        ai.vision_range = vision_range;
        ai.aggro_range = aggro_range;
        return *this;
    }

    /**
     * @brief Build the entity
     * @return Unique pointer to built entity
     */
    std::unique_ptr<Entity> build() {
        return std::move(entity);
    }

    /**
     * @brief Reset builder for new entity
     */
    void reset() {
        entity = std::make_unique<Entity>();
    }

private:
    std::unique_ptr<Entity> entity;
};

/**
 * @class PlayerFactory
 * @brief Factory for creating player entities
 */
class PlayerFactory : public EntityFactory {
public:
    std::unique_ptr<Entity> create(int x, int y) override {
        return EntityBuilder()
            .withPosition(x, y)
            .withRenderable("@", ftxui::Color::Yellow)
            .withHealth(100)
            .withCombat(6, 3, 2)
            .withCombatName("Player")
            .build();
    }

    std::unique_ptr<Entity> create(const std::string& name, int x, int y) override {
        auto entity = create(x, y);
        if (auto* combat = entity->getComponent<CombatComponent>()) {
            combat->combat_name = name;
        }
        return entity;
    }
};

/**
 * @class MonsterFactoryECS
 * @brief Factory for creating monster entities in ECS
 *
 * Note: Named MonsterFactoryECS to avoid conflict with existing MonsterFactory
 */
class MonsterFactoryECS : public EntityFactory {
public:
    using MonsterBuilder = std::function<std::unique_ptr<Entity>(int, int)>;

    MonsterFactoryECS() {
        // Register common monster types
        registerMonster("goblin", [](int x, int y) {
            return EntityBuilder()
                .withPosition(x, y)
                .withRenderable("g", ftxui::Color::Green)
                .withHealth(20)
                .withCombatRange(1, 4, 1, 0)
                .withCombatName("Goblin")
                .withAI(AIBehavior::AGGRESSIVE, 5, 3)
                .build();
        });

        registerMonster("orc", [](int x, int y) {
            return EntityBuilder()
                .withPosition(x, y)
                .withRenderable("o", ftxui::Color::RGB(139, 69, 19))  // Brown
                .withHealth(35)
                .withCombatRange(2, 6, 2, 1)
                .withCombatName("Orc")
                .withAI(AIBehavior::AGGRESSIVE, 6, 4)
                .build();
        });

        registerMonster("troll", [](int x, int y) {
            return EntityBuilder()
                .withPosition(x, y)
                .withRenderable("T", ftxui::Color::RGB(0, 128, 0))  // Dark green
                .withHealth(50)
                .withCombatRange(3, 8, 3, 2)
                .withCombatName("Troll")
                .withAI(AIBehavior::DEFENSIVE, 5, 3)
                .build();
        });

        registerMonster("skeleton", [](int x, int y) {
            return EntityBuilder()
                .withPosition(x, y)
                .withRenderable("s", ftxui::Color::RGB(255, 255, 240))  // Ivory
                .withHealth(15)
                .withCombatRange(1, 3, 2, 0)
                .withCombatName("Skeleton")
                .withAI(AIBehavior::WANDERING, 4, 2)
                .build();
        });

        registerMonster("dragon", [](int x, int y) {
            return EntityBuilder()
                .withPosition(x, y)
                .withRenderable("D", ftxui::Color::Red)
                .withHealth(100)
                .withCombatRange(5, 15, 5, 5)
                .withCombatName("Dragon")
                .withAI(AIBehavior::AGGRESSIVE, 8, 6)
                .asBlocking()
                .build();
        });
    }

    /**
     * @brief Register a new monster type
     * @param type Monster type name
     * @param builder Builder function for the monster
     */
    void registerMonster(const std::string& type, MonsterBuilder builder) {
        monster_builders[type] = builder;
    }

    std::unique_ptr<Entity> create(int x, int y) override {
        // Default to goblin
        return create("goblin", x, y);
    }

    std::unique_ptr<Entity> create(const std::string& type, int x, int y) override {
        auto it = monster_builders.find(type);
        if (it != monster_builders.end()) {
            return it->second(x, y);
        }
        // Unknown type - create generic monster
        return EntityBuilder()
            .withPosition(x, y)
            .withRenderable("?", ftxui::Color::Magenta)
            .withHealth(10)
            .withCombat(1)
            .withCombatName("Unknown")
            .build();
    }

    /**
     * @brief Get list of registered monster types
     * @return Vector of type names
     */
    std::vector<std::string> getRegisteredTypes() const {
        std::vector<std::string> types;
        for (const auto& [type, _] : monster_builders) {
            types.push_back(type);
        }
        return types;
    }

private:
    std::unordered_map<std::string, MonsterBuilder> monster_builders;
};

/**
 * @class ItemFactoryECS
 * @brief Factory for creating item entities in ECS
 */
class ItemFactoryECS : public EntityFactory {
public:
    using ItemBuilder = std::function<std::unique_ptr<Entity>(int, int)>;

    ItemFactoryECS() {
        // Register common item types
        registerItem("potion", [](int x, int y) {
            return EntityBuilder()
                .withPosition(x, y)
                .withRenderable("!", ftxui::Color::Magenta)
                .build();
        });

        registerItem("sword", [](int x, int y) {
            return EntityBuilder()
                .withPosition(x, y)
                .withRenderable("/", ftxui::Color::RGB(192, 192, 192))  // Silver
                .build();
        });

        registerItem("gold", [](int x, int y) {
            return EntityBuilder()
                .withPosition(x, y)
                .withRenderable("$", ftxui::Color::Yellow)
                .build();
        });

        registerItem("scroll", [](int x, int y) {
            return EntityBuilder()
                .withPosition(x, y)
                .withRenderable("?", ftxui::Color::RGB(255, 248, 220))  // Cornsilk
                .build();
        });
    }

    void registerItem(const std::string& type, ItemBuilder builder) {
        item_builders[type] = builder;
    }

    std::unique_ptr<Entity> create(int x, int y) override {
        // Default to potion
        return create("potion", x, y);
    }

    std::unique_ptr<Entity> create(const std::string& type, int x, int y) override {
        auto it = item_builders.find(type);
        if (it != item_builders.end()) {
            return it->second(x, y);
        }
        // Unknown item
        return EntityBuilder()
            .withPosition(x, y)
            .withRenderable("*", ftxui::Color::White)
            .build();
    }

private:
    std::unordered_map<std::string, ItemBuilder> item_builders;
};

} // namespace ecs