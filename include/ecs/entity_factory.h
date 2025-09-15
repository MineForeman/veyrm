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
#include "inventory_component.h"
#include "player_component.h"
#include "stats_component.h"
#include "ai_system.h"
#include "../color_scheme.h"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace ecs {

/**
 * @class EntityFactory
 * @brief Factory for creating configured entities
 */
class EntityFactory {
public:
    EntityFactory() = default;
    ~EntityFactory() = default;

    /**
     * @brief Create player entity
     * @param x Starting X position
     * @param y Starting Y position
     * @param name Player name
     * @return Player entity
     */
    static std::unique_ptr<Entity> createPlayer(int x, int y, const std::string& name = "Player");

    /**
     * @brief Create monster from template ID
     * @param monster_id Monster template ID
     * @param x X position
     * @param y Y position
     * @param level Optional level override
     * @return Monster entity
     */
    static std::unique_ptr<Entity> createMonster(const std::string& monster_id,
                                                 int x, int y, int level = -1);

    /**
     * @brief Create item from template ID
     * @param item_id Item template ID
     * @param x X position
     * @param y Y position
     * @param quantity Stack size
     * @return Item entity
     */
    static std::unique_ptr<Entity> createItem(const std::string& item_id,
                                             int x, int y, int quantity = 1);

    /**
     * @brief Create NPC entity
     * @param npc_id NPC template ID
     * @param x X position
     * @param y Y position
     * @param dialogue_id Dialogue tree ID
     * @return NPC entity
     */
    static std::unique_ptr<Entity> createNPC(const std::string& npc_id,
                                            int x, int y,
                                            const std::string& dialogue_id = "");

    /**
     * @brief Create door entity
     * @param x X position
     * @param y Y position
     * @param locked Whether door is locked
     * @param key_id Required key ID if locked
     * @return Door entity
     */
    static std::unique_ptr<Entity> createDoor(int x, int y, bool locked = false,
                                             const std::string& key_id = "");

    /**
     * @brief Create container entity
     * @param x X position
     * @param y Y position
     * @param container_type Type (chest, barrel, etc)
     * @param locked Whether container is locked
     * @return Container entity
     */
    static std::unique_ptr<Entity> createContainer(int x, int y,
                                                  const std::string& container_type = "chest",
                                                  bool locked = false);

    /**
     * @brief Create trap entity
     * @param x X position
     * @param y Y position
     * @param trap_type Type of trap
     * @param damage Trap damage
     * @return Trap entity
     */
    static std::unique_ptr<Entity> createTrap(int x, int y,
                                             const std::string& trap_type = "spike",
                                             int damage = 10);

    /**
     * @brief Create stairs entity
     * @param x X position
     * @param y Y position
     * @param going_down Direction (true = down, false = up)
     * @param destination_level Target level
     * @return Stairs entity
     */
    static std::unique_ptr<Entity> createStairs(int x, int y,
                                               bool going_down = true,
                                               int destination_level = -1);

    /**
     * @brief Create light source entity
     * @param x X position
     * @param y Y position
     * @param radius Light radius
     * @param color Light color
     * @return Light entity
     */
    static std::unique_ptr<Entity> createLight(int x, int y, int radius = 5,
                                              const Color& color = {255, 255, 200});

    /**
     * @brief Create projectile entity
     * @param x Starting X
     * @param y Starting Y
     * @param target_x Target X
     * @param target_y Target Y
     * @param damage Projectile damage
     * @param speed Movement speed
     * @return Projectile entity
     */
    static std::unique_ptr<Entity> createProjectile(int x, int y,
                                                   int target_x, int target_y,
                                                   int damage = 5,
                                                   float speed = 10.0f);

    /**
     * @brief Load entity from JSON data
     * @param json Entity data
     * @return Entity or nullptr on error
     */
    static std::unique_ptr<Entity> createFromJSON(const nlohmann::json& json);

    /**
     * @brief Create random monster for level
     * @param x X position
     * @param y Y position
     * @param dungeon_level Current dungeon level
     * @return Monster entity
     */
    static std::unique_ptr<Entity> createRandomMonster(int x, int y, int dungeon_level);

    /**
     * @brief Create random item for level
     * @param x X position
     * @param y Y position
     * @param dungeon_level Current dungeon level
     * @param item_category Optional category filter
     * @return Item entity
     */
    static std::unique_ptr<Entity> createRandomItem(int x, int y, int dungeon_level,
                                                   const std::string& item_category = "");

private:
    /**
     * @brief Apply monster template to entity
     * @param entity Entity to configure
     * @param monster_id Template ID
     * @param level Optional level override
     */
    static void applyMonsterTemplate(Entity* entity, const std::string& monster_id, int level);

    /**
     * @brief Apply item template to entity
     * @param entity Entity to configure
     * @param item_id Template ID
     * @param quantity Stack size
     */
    static void applyItemTemplate(Entity* entity, const std::string& item_id, int quantity);

    /**
     * @brief Scale monster stats by level
     * @param entity Monster entity
     * @param level Target level
     */
    static void scaleMonsterToLevel(Entity* entity, int level);

    /**
     * @brief Generate random item properties
     * @param entity Item entity
     * @param quality Item quality level
     */
    static void generateRandomProperties(Entity* entity, int quality);
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
class PlayerFactory {
public:
    std::unique_ptr<Entity> create(int x, int y) {
        auto entity = EntityBuilder()
            .withPosition(x, y)
            .withRenderable("@", ftxui::Color::Yellow)
            .withHealth(100)
            .withCombat(6, 3, 2)
            .withCombatName("Player")
            .build();
        entity->addTag("player");

        // Add player-specific component
        entity->addComponent<PlayerComponent>();

        // Add inventory component for item pickup functionality
        entity->addComponent<InventoryComponent>(26, 100.0f);  // 26 slots, 100 weight limit

        // Add stats component for RPG mechanics
        auto& stats = entity->addComponent<StatsComponent>();
        stats.strength = 12;
        stats.dexterity = 10;
        stats.constitution = 14;
        stats.intelligence = 10;
        stats.wisdom = 10;
        stats.charisma = 10;
        stats.recalculateDerived();

        return entity;
    }

    std::unique_ptr<Entity> create(const std::string& name, int x, int y) {
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
class MonsterFactoryECS {
public:
    using MonsterBuilder = std::function<std::unique_ptr<Entity>(int, int)>;

    MonsterFactoryECS() {
        // Register common monster types
        registerMonster("goblin", [](int x, int y) {
            auto entity = EntityBuilder()
                .withPosition(x, y)
                .withRenderable("g", ftxui::Color::Green)
                .withHealth(20)
                .withCombatRange(1, 4, 1, 0)
                .withCombatName("Goblin")
                .withAI(AIBehavior::AGGRESSIVE, 5, 3)
                .build();
            entity->addTag("monster");
            entity->addTag("goblin");
            return entity;
        });

        registerMonster("orc", [](int x, int y) {
            auto entity = EntityBuilder()
                .withPosition(x, y)
                .withRenderable("o", ftxui::Color::RGB(139, 69, 19))  // Brown
                .withHealth(35)
                .withCombatRange(2, 6, 2, 1)
                .withCombatName("Orc")
                .withAI(AIBehavior::AGGRESSIVE, 6, 4)
                .build();
            entity->addTag("monster");
            entity->addTag("orc");
            return entity;
        });

        registerMonster("troll", [](int x, int y) {
            auto entity = EntityBuilder()
                .withPosition(x, y)
                .withRenderable("T", ftxui::Color::RGB(0, 128, 0))  // Dark green
                .withHealth(50)
                .withCombatRange(3, 8, 3, 2)
                .withCombatName("Troll")
                .withAI(AIBehavior::DEFENSIVE, 5, 3)
                .build();
            entity->addTag("monster");
            entity->addTag("troll");
            return entity;
        });

        registerMonster("skeleton", [](int x, int y) {
            auto entity = EntityBuilder()
                .withPosition(x, y)
                .withRenderable("s", ftxui::Color::RGB(255, 255, 240))  // Ivory
                .withHealth(15)
                .withCombatRange(1, 3, 2, 0)
                .withCombatName("Skeleton")
                .withAI(AIBehavior::WANDERING, 4, 2)
                .build();
            entity->addTag("monster");
            entity->addTag("skeleton");
            return entity;
        });

        registerMonster("dragon", [](int x, int y) {
            auto entity = EntityBuilder()
                .withPosition(x, y)
                .withRenderable("D", ftxui::Color::Red)
                .withHealth(100)
                .withCombatRange(5, 15, 5, 5)
                .withCombatName("Dragon")
                .withAI(AIBehavior::AGGRESSIVE, 8, 6)
                .asBlocking()
                .build();
            entity->addTag("monster");
            entity->addTag("dragon");
            entity->addTag("boss");
            return entity;
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

    std::unique_ptr<Entity> create(int x, int y) {
        // Default to goblin
        return create("goblin", x, y);
    }

    std::unique_ptr<Entity> create(const std::string& type, int x, int y) {
        auto it = monster_builders.find(type);
        if (it != monster_builders.end()) {
            return it->second(x, y);
        }
        // Unknown type - create generic monster
        auto entity = EntityBuilder()
            .withPosition(x, y)
            .withRenderable("?", ftxui::Color::Magenta)
            .withHealth(10)
            .withCombat(1)
            .withCombatName("Unknown")
            .build();
        entity->addTag("monster");
        return entity;
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
class ItemFactoryECS {
public:
    using ItemBuilder = std::function<std::unique_ptr<Entity>(int, int)>;

    ItemFactoryECS() {
        // Register common item types
        registerItem("potion", [](int x, int y) {
            auto entity = EntityBuilder()
                .withPosition(x, y)
                .withRenderable("!", ftxui::Color::Magenta)
                .build();
            entity->addTag("item");
            entity->addTag("potion");
            return entity;
        });

        registerItem("sword", [](int x, int y) {
            auto entity = EntityBuilder()
                .withPosition(x, y)
                .withRenderable("/", ftxui::Color::RGB(192, 192, 192))  // Silver
                .build();
            entity->addTag("item");
            entity->addTag("sword");
            entity->addTag("weapon");
            return entity;
        });

        registerItem("gold", [](int x, int y) {
            auto entity = EntityBuilder()
                .withPosition(x, y)
                .withRenderable("$", ftxui::Color::Yellow)
                .build();
            entity->addTag("item");
            entity->addTag("gold");
            entity->addTag("currency");
            return entity;
        });

        registerItem("scroll", [](int x, int y) {
            auto entity = EntityBuilder()
                .withPosition(x, y)
                .withRenderable("?", ftxui::Color::RGB(255, 248, 220))  // Cornsilk
                .build();
            entity->addTag("item");
            entity->addTag("scroll");
            entity->addTag("consumable");
            return entity;
        });
    }

    void registerItem(const std::string& type, ItemBuilder builder) {
        item_builders[type] = builder;
    }

    std::unique_ptr<Entity> create(int x, int y) {
        // Default to potion
        return create("potion", x, y);
    }

    std::unique_ptr<Entity> create(const std::string& type, int x, int y) {
        auto it = item_builders.find(type);
        if (it != item_builders.end()) {
            return it->second(x, y);
        }
        // Unknown item
        auto entity = EntityBuilder()
            .withPosition(x, y)
            .withRenderable("*", ftxui::Color::White)
            .build();
        entity->addTag("item");
        return entity;
    }

private:
    std::unordered_map<std::string, ItemBuilder> item_builders;
};

} // namespace ecs