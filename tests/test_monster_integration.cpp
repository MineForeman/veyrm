#include <catch2/catch_test_macros.hpp>
#include "ecs/data_loader.h"
#include "ecs/entity_factory.h"
#include "entity_manager.h"

TEST_CASE("Monster Data Loading via ECS", "[monster][integration][ecs]") {
    ecs::DataLoader& loader = ecs::DataLoader::getInstance();

    SECTION("Load actual monsters.json file via ECS DataLoader") {
        // Load the actual data file (from project root)
        bool loaded = loader.loadMonsters("data/monsters.json");
        REQUIRE(loaded == true);

        // Check all expected monsters are loaded
        auto gutter_rat = loader.getMonsterTemplate("gutter_rat");
        REQUIRE(gutter_rat != nullptr);
        REQUIRE(gutter_rat->name == "Gutter Rat");

        auto orc_rookling = loader.getMonsterTemplate("orc_rookling");
        REQUIRE(orc_rookling != nullptr);
        REQUIRE(orc_rookling->name == "Orc Rookling");

        auto cave_spider = loader.getMonsterTemplate("cave_spider");
        REQUIRE(cave_spider != nullptr);

        auto goblin = loader.getMonsterTemplate("goblin");
        REQUIRE(goblin != nullptr);

        auto zombie = loader.getMonsterTemplate("zombie");
        REQUIRE(zombie != nullptr);

        // Verify we have the expected number of species
        auto& templates = loader.getMonsterTemplates();
        REQUIRE(templates.size() == 13);
    }

    SECTION("Create monsters via ECS EntityFactory") {
        loader.loadMonsters("data/monsters.json");
        ecs::EntityFactory factory;

        // Test creating a gutter rat
        auto rat_template = loader.getMonsterTemplate("gutter_rat");
        REQUIRE(rat_template != nullptr);

        auto rat = factory.createMonster("gutter_rat", 10, 10);
        REQUIRE(rat != nullptr);

        // Verify position
        auto* pos = rat->getComponent<ecs::PositionComponent>();
        REQUIRE(pos != nullptr);
        REQUIRE(pos->position.x == 10);
        REQUIRE(pos->position.y == 10);

        // Verify health
        auto* health = rat->getComponent<ecs::HealthComponent>();
        REQUIRE(health != nullptr);
        REQUIRE(health->max_hp == rat_template->hp);

        // Test creating multiple types
        auto goblin = factory.createMonster("goblin", 5, 5);
        REQUIRE(goblin != nullptr);

        auto zombie = factory.createMonster("zombie", 15, 15);
        REQUIRE(zombie != nullptr);
    }

    SECTION("Verify monster properties from templates") {
        loader.loadMonsters("data/monsters.json");

        // Check gutter rat properties
        auto rat = loader.getMonsterTemplate("gutter_rat");
        REQUIRE(rat != nullptr);
        REQUIRE(rat->hp == 3);
        // Attack and defense are now in the combat component, not template
        // XP value is in experience.amount

        // Check orc rookling exists and is stronger
        auto orc = loader.getMonsterTemplate("orc_rookling");
        REQUIRE(orc != nullptr);
        REQUIRE(orc->hp > rat->hp);
    }
}

TEST_CASE("Monster Spawning via ECS", "[monster][spawn][ecs]") {
    ecs::DataLoader& loader = ecs::DataLoader::getInstance();
    loader.loadMonsters("data/monsters.json");

    SECTION("Spawn monsters at different depths") {
        ecs::EntityFactory factory;

        // Should be able to create any monster regardless of depth
        // (depth filtering would be done at a higher level)
        auto rat = factory.createMonster("gutter_rat", 0, 0);
        REQUIRE(rat != nullptr);

        auto orc = factory.createMonster("orc_rookling", 0, 0);
        REQUIRE(orc != nullptr);

        auto goblin = factory.createMonster("goblin", 0, 0);
        REQUIRE(goblin != nullptr);
    }

    SECTION("Monster pack spawning") {
        auto goblin_template = loader.getMonsterTemplate("goblin");
        REQUIRE(goblin_template != nullptr);

        // Goblins should have pack size info
        REQUIRE(goblin_template->min_pack_size >= 1);
        REQUIRE(goblin_template->max_pack_size >= goblin_template->min_pack_size);
    }
}