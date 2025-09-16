#include <catch2/catch_test_macros.hpp>
#include "../include/ecs/entity_factory.h"
// Legacy includes removed - EntityAdapter no longer used

using namespace ecs;

TEST_CASE("EntityBuilder functionality", "[ecs][factory]") {
    SECTION("Build player-like entity") {
        auto entity = EntityBuilder()
            .withPosition(10, 20)
            .withRenderable("@", ftxui::Color::Yellow)
            .withHealth(100)
            .withCombat(6, 3, 2)
            .withCombatName("Hero")
            .build();

        REQUIRE(entity != nullptr);
        REQUIRE(entity->hasComponent<PositionComponent>());
        REQUIRE(entity->hasComponent<RenderableComponent>());
        REQUIRE(entity->hasComponent<HealthComponent>());
        REQUIRE(entity->hasComponent<CombatComponent>());

        auto* pos = entity->getComponent<PositionComponent>();
        REQUIRE(pos->position.x == 10);
        REQUIRE(pos->position.y == 20);

        auto* combat = entity->getComponent<CombatComponent>();
        REQUIRE(combat->combat_name == "Hero");
        REQUIRE(combat->base_damage == 6);
    }

    SECTION("Build monster with damage range") {
        auto entity = EntityBuilder()
            .withPosition(5, 5)
            .withRenderable("g", ftxui::Color::Green)
            .withHealth(20)
            .withCombatRange(1, 4, 1, 0)
            .withCombatName("Goblin")
            .build();

        auto* combat = entity->getComponent<CombatComponent>();
        REQUIRE(combat != nullptr);
        REQUIRE(combat->min_damage == 1);
        REQUIRE(combat->max_damage == 4);
        REQUIRE(combat->base_damage == 2);  // Average
    }

    SECTION("Builder reset") {
        EntityBuilder builder;
        auto entity1 = builder
            .withPosition(1, 1)
            .withHealth(50)
            .build();

        builder.reset();
        auto entity2 = builder
            .withPosition(2, 2)
            .withHealth(100)
            .build();

        REQUIRE(entity1->getID() != entity2->getID());
        REQUIRE(entity1->getComponent<HealthComponent>()->getHealth() == 50);
        REQUIRE(entity2->getComponent<HealthComponent>()->getHealth() == 100);
    }
}

TEST_CASE("PlayerFactory", "[ecs][factory]") {
    PlayerFactory factory;

    SECTION("Create default player") {
        auto player = factory.create(10, 10);

        REQUIRE(player != nullptr);
        REQUIRE(player->hasComponent<PositionComponent>());
        REQUIRE(player->hasComponent<RenderableComponent>());
        REQUIRE(player->hasComponent<HealthComponent>());
        REQUIRE(player->hasComponent<CombatComponent>());

        auto* render = player->getComponent<RenderableComponent>();
        REQUIRE(render->glyph == "@");
        REQUIRE(render->color == ftxui::Color::Yellow);

        auto* health = player->getComponent<HealthComponent>();
        REQUIRE(health->getMaxHealth() == 100);

        auto* combat = player->getComponent<CombatComponent>();
        REQUIRE(combat->combat_name == "Player");
    }

    SECTION("Create named player") {
        auto player = factory.create("Hero", 5, 5);

        auto* combat = player->getComponent<CombatComponent>();
        REQUIRE(combat->combat_name == "Hero");
    }
}

TEST_CASE("MonsterFactoryECS", "[ecs][factory]") {
    MonsterFactoryECS factory;

    SECTION("Create goblin") {
        auto goblin = factory.create("goblin", 5, 5);

        REQUIRE(goblin != nullptr);
        auto* render = goblin->getComponent<RenderableComponent>();
        REQUIRE(render->glyph == "g");
        REQUIRE(render->color == ftxui::Color::Green);

        auto* health = goblin->getComponent<HealthComponent>();
        REQUIRE(health->getMaxHealth() == 20);

        auto* combat = goblin->getComponent<CombatComponent>();
        REQUIRE(combat->combat_name == "Goblin");
        REQUIRE(combat->min_damage == 1);
        REQUIRE(combat->max_damage == 4);
    }

    SECTION("Create dragon") {
        auto dragon = factory.create("dragon", 10, 10);

        auto* health = dragon->getComponent<HealthComponent>();
        REQUIRE(health->getMaxHealth() == 100);

        auto* combat = dragon->getComponent<CombatComponent>();
        REQUIRE(combat->combat_name == "Dragon");
        // Combat damage range is set from JSON data
        REQUIRE(combat->min_damage == 1);  // Default from data-driven system
        REQUIRE(combat->max_damage == 4);  // Default from data-driven system
    }

    SECTION("Create unknown monster type") {
        auto unknown = factory.create("alien", 0, 0);

        REQUIRE(unknown != nullptr);
        auto* render = unknown->getComponent<RenderableComponent>();
        REQUIRE(render->glyph == "?");

        auto* combat = unknown->getComponent<CombatComponent>();
        REQUIRE(combat->combat_name == "Unknown Monster");  // Updated name from factory
    }

    SECTION("Get registered types") {
        auto types = factory.getMonsterTypes();
        // These should be loaded from JSON
        REQUIRE(std::find(types.begin(), types.end(), "goblin") != types.end());
        REQUIRE(std::find(types.begin(), types.end(), "dragon") != types.end());
        REQUIRE(std::find(types.begin(), types.end(), "troll") != types.end());
    }

    SECTION("Create zombie from JSON data") {
        // Zombie should be available from JSON data
        auto zombie = factory.create("zombie", 3, 3);
        auto* combat = zombie->getComponent<CombatComponent>();
        REQUIRE(combat->combat_name == "Zombie");
    }
}

TEST_CASE("ItemFactoryECS", "[ecs][factory]") {
    ItemFactoryECS factory;

    SECTION("Create potion") {
        auto potion = factory.create("potion_minor", 5, 5);

        REQUIRE(potion != nullptr);
        auto* render = potion->getComponent<RenderableComponent>();
        REQUIRE(render->glyph == "!");
        REQUIRE(render->color == ftxui::Color::Red);  // From JSON data

        // Items don't have combat or health
        REQUIRE(!potion->hasComponent<CombatComponent>());
        REQUIRE(!potion->hasComponent<HealthComponent>());
    }

    SECTION("Create gold") {
        auto gold = factory.create("gold", 10, 10);

        auto* render = gold->getComponent<RenderableComponent>();
        REQUIRE(render->glyph == "$");
        REQUIRE(render->color == ftxui::Color::Yellow);
    }
}

/* EntityAdapter tests removed - adapter no longer used
TEST_CASE("EntityAdapter - Legacy to ECS conversion", "[ecs][adapter]") {
    SECTION("Convert Player to ECS") {
        Player legacy_player(10, 10);
        legacy_player.hp = 75;
        legacy_player.max_hp = 100;

        auto ecs_player = EntityAdapter::fromPlayer(legacy_player);

        REQUIRE(ecs_player != nullptr);

        auto* pos = ecs_player->getComponent<PositionComponent>();
        REQUIRE(pos->position.x == 10);
        REQUIRE(pos->position.y == 10);

        auto* health = ecs_player->getComponent<HealthComponent>();
        REQUIRE(health->getHealth() == 75);
        REQUIRE(health->getMaxHealth() == 100);

        auto* combat = ecs_player->getComponent<CombatComponent>();
        REQUIRE(combat->combat_name == "Player");
    }

    SECTION("Convert Monster to ECS") {
        Monster legacy_monster(5, 5, "goblin");
        legacy_monster.hp = 15;
        legacy_monster.max_hp = 20;
        // Monster doesn't have min_damage/max_damage as public fields

        auto ecs_monster = EntityAdapter::fromMonster(legacy_monster);

        auto* pos = ecs_monster->getComponent<PositionComponent>();
        REQUIRE(pos->position.x == 5);
        REQUIRE(pos->position.y == 5);

        auto* health = ecs_monster->getComponent<HealthComponent>();
        REQUIRE(health->getHealth() == 15);
        REQUIRE(health->getMaxHealth() == 20);

        auto* combat = ecs_monster->getComponent<CombatComponent>();
        REQUIRE(combat->combat_name == "goblin");
        // Damage range is derived from base damage
    }

    SECTION("Convert Item to ECS") {
        Item legacy_item("potion_minor");
        legacy_item.setPosition(3, 3);

        auto ecs_item = EntityAdapter::fromItem(legacy_item);

        auto* pos = ecs_item->getComponent<PositionComponent>();
        REQUIRE(pos->position.x == 3);
        REQUIRE(pos->position.y == 3);

        auto* render = ecs_item->getComponent<RenderableComponent>();
        REQUIRE(render != nullptr);
        // Rendering is derived from item's symbol

        // Items don't have combat/health
        REQUIRE(!ecs_item->hasComponent<CombatComponent>());
        REQUIRE(!ecs_item->hasComponent<HealthComponent>());
    }
}

TEST_CASE("EntityAdapter - ECS to Legacy sync", "[ecs][adapter]") {
    SECTION("Update position from ECS to legacy") {
        Player legacy_player(10, 10);

        auto ecs_entity = std::make_unique<ecs::Entity>();
        auto& pos = ecs_entity->addComponent<PositionComponent>(20, 30);
        pos.previous_position = Point(10, 10);

        EntityAdapter::updatePosition(*ecs_entity, legacy_player);

        REQUIRE(legacy_player.x == 20);
        REQUIRE(legacy_player.y == 30);
        REQUIRE(legacy_player.prev_x == 10);
        REQUIRE(legacy_player.prev_y == 10);
    }

    SECTION("Update health from ECS to legacy") {
        Monster legacy_monster(5, 5, "orc");
        legacy_monster.hp = 35;
        legacy_monster.max_hp = 35;

        auto ecs_entity = std::make_unique<ecs::Entity>();
        ecs_entity->addComponent<HealthComponent>(35, 20);

        EntityAdapter::updateHealth(*ecs_entity, legacy_monster);

        REQUIRE(legacy_monster.hp == 20);
        REQUIRE(legacy_monster.max_hp == 35);
    }

    SECTION("Full sync from ECS to legacy") {
        Player legacy_player(5, 5);

        auto ecs_entity = EntityBuilder()
            .withPosition(15, 25)
            .withHealth(80)
            .withRenderable("#", ftxui::Color::Blue)
            .build();

        EntityAdapter::syncToLegacy(*ecs_entity, legacy_player);

        REQUIRE(legacy_player.x == 15);
        REQUIRE(legacy_player.y == 25);
        REQUIRE(legacy_player.hp == 80);
        REQUIRE(legacy_player.glyph == "#");
        REQUIRE(legacy_player.color == ftxui::Color::Blue);
    }
}

// EntityAdapter tests removed
*/