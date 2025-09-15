#include <catch2/catch_test_macros.hpp>
#include "monster_factory.h"
#include "monster.h"
#include "entity_manager.h"

TEST_CASE("Monster JSON Loading Integration", "[monster][integration]") {
    MonsterFactory& factory = MonsterFactory::getInstance();
    factory.clearTemplates();
    
    SECTION("Load actual monsters.json file") {
        // Load the actual data file (from project root)
        bool loaded = factory.loadFromFile("data/monsters.json");
        REQUIRE(loaded == true);
        
        // Check all expected monsters are loaded
        REQUIRE(factory.hasSpecies("gutter_rat") == true);
        REQUIRE(factory.hasSpecies("orc_rookling") == true);
        REQUIRE(factory.hasSpecies("cave_spider") == true);
        REQUIRE(factory.hasSpecies("goblin") == true);
        REQUIRE(factory.hasSpecies("zombie") == true);
        
        // Verify we have the expected number of species
        auto species = factory.getAvailableSpecies();
        REQUIRE(species.size() == 13);
    }
    
    SECTION("Create each monster type from file") {
        factory.loadFromFile("data/monsters.json");
        EntityManager manager;
        
        // Gutter Rat
        auto rat = manager.createMonster("gutter_rat", 0, 0);
        REQUIRE(rat != nullptr);
        REQUIRE(rat->name == "Gutter Rat");
        REQUIRE(rat->hp == 3);
        REQUIRE(rat->max_hp == 3);
        REQUIRE(rat->glyph == "r");
        REQUIRE(rat->threat_level == 'a');
        
        // Orc Rookling
        auto orc = manager.createMonster("orc_rookling", 1, 1);
        REQUIRE(orc != nullptr);
        REQUIRE(orc->name == "Orc Rookling");
        REQUIRE(orc->hp == 20);
        REQUIRE(orc->max_hp == 20);

        // Cave Spider
        auto spider = manager.createMonster("cave_spider", 2, 2);
        REQUIRE(spider != nullptr);
        REQUIRE(spider->name == "Cave Spider");

        // Goblin (replacing kobold)
        auto goblin = manager.createMonster("goblin", 3, 3);
        REQUIRE(goblin != nullptr);
        REQUIRE(goblin->name == "Goblin");
        REQUIRE(goblin->hp == 20);

        // Zombie
        auto zombie = manager.createMonster("zombie", 4, 4);
        REQUIRE(zombie != nullptr);
        REQUIRE(zombie->name == "Zombie");
        REQUIRE(zombie->hp == 25);
        REQUIRE(zombie->max_hp == 25);
    }
    
    SECTION("Verify threat levels") {
        factory.loadFromFile("data/monsters.json");

        // Since threat_level defaults to 'a' when not specified in JSON
        REQUIRE(factory.getThreatLevel("gutter_rat") == 'a');
        REQUIRE(factory.getThreatLevel("cave_spider") == 'a');
        REQUIRE(factory.getThreatLevel("goblin") == 'a');
        REQUIRE(factory.getThreatLevel("orc_rookling") == 'a');
        REQUIRE(factory.getThreatLevel("zombie") == 'a');
    }
}