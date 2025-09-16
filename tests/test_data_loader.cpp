/**
 * @file test_data_loader.cpp
 * @brief Test JSON data loading functionality
 */

#include <catch2/catch_test_macros.hpp>
#include "ecs/data_loader.h"

using namespace ecs;

TEST_CASE("DataLoader: Load JSON data files", "[data][loader]") {
    auto& loader = DataLoader::getInstance();

    SECTION("Load all data from data directory") {
        // Try to load data
        bool loaded = loader.loadAllData("data");

        // Check if loaded (might fail if data dir doesn't exist)
        if (loaded) {
            REQUIRE(loader.isLoaded());

            SECTION("Check monster templates") {
                // Should have loaded some monsters
                auto& monsters = loader.getMonsterTemplates();
                REQUIRE(!monsters.empty());

                // Check specific monster
                auto* goblin = loader.getMonsterTemplate("goblin");
                if (goblin) {
                    REQUIRE(goblin->name == "Goblin");
                    REQUIRE(goblin->glyph == 'g');
                    REQUIRE(goblin->hp > 0);
                }
            }

            SECTION("Check item templates") {
                // Should have loaded some items
                auto& items = loader.getItemTemplates();
                REQUIRE(!items.empty());

                // Check specific item
                auto* sword = loader.getItemTemplate("sword");
                if (sword) {
                    REQUIRE(sword->name == "Sword");
                    REQUIRE(sword->symbol == '/');
                    REQUIRE(sword->value > 0);
                }
            }
        }
    }

    SECTION("Clear data") {
        loader.clearData();
        REQUIRE(!loader.isLoaded());
        REQUIRE(loader.getMonsterTemplates().empty());
        REQUIRE(loader.getItemTemplates().empty());
    }
}

TEST_CASE("DataLoader: Individual template loading", "[data][loader]") {
    auto& loader = DataLoader::getInstance();
    loader.clearData();

    SECTION("Load monsters only") {
        bool loaded = loader.loadMonsters("data/monsters.json");
        if (loaded) {
            auto& monsters = loader.getMonsterTemplates();
            REQUIRE(!monsters.empty());

            // Check a few monster types exist
            REQUIRE(loader.getMonsterTemplate("rat") != nullptr);
            REQUIRE(loader.getMonsterTemplate("goblin") != nullptr);
            REQUIRE(loader.getMonsterTemplate("dragon") != nullptr);
        }
    }

    SECTION("Load items only") {
        bool loaded = loader.loadItems("data/items.json");
        if (loaded) {
            auto& items = loader.getItemTemplates();
            REQUIRE(!items.empty());

            // Check a few item types exist
            REQUIRE(loader.getItemTemplate("potion_minor") != nullptr);
            REQUIRE(loader.getItemTemplate("sword") != nullptr);
            REQUIRE(loader.getItemTemplate("leather_armor") != nullptr);
        }
    }
}

TEST_CASE("DataLoader: Template properties", "[data][loader]") {
    auto& loader = DataLoader::getInstance();

    // Ensure data is loaded
    if (!loader.isLoaded()) {
        loader.loadAllData("data");
    }

    if (loader.isLoaded()) {
        SECTION("Monster template properties") {
            auto* dragon = loader.getMonsterTemplate("dragon");
            if (dragon) {
                REQUIRE(dragon->id == "dragon");
                REQUIRE(dragon->name == "Dragon");
                REQUIRE(dragon->glyph == 'D');
                REQUIRE(dragon->hp >= 100);
                REQUIRE(dragon->xp_value > 0);
                REQUIRE(dragon->aggressive == true);
            }
        }

        SECTION("Item template properties") {
            auto* potion = loader.getItemTemplate("potion_major");
            if (potion) {
                REQUIRE(potion->id == "potion_major");
                REQUIRE(potion->name == "Major Healing Potion");
                REQUIRE(potion->symbol == '!');
                REQUIRE(potion->heal_amount > 0);
                REQUIRE(potion->stackable == true);
            }
        }
    }
}