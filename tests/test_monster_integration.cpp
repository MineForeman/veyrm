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
        REQUIRE(factory.hasSpecies("kobold") == true);
        REQUIRE(factory.hasSpecies("zombie") == true);
        
        // Verify we have exactly 5 species
        auto species = factory.getAvailableSpecies();
        REQUIRE(species.size() == 5);
    }
    
    SECTION("Create each monster type from file") {
        factory.loadFromFile("data/monsters.json");
        EntityManager manager;
        
        // Gutter Rat
        auto rat = manager.createMonster("gutter_rat", 0, 0);
        REQUIRE(rat != nullptr);
        REQUIRE(rat->name == "Gutter Rat");
        REQUIRE(rat->hp == 3);
        REQUIRE(rat->attack == 2);
        REQUIRE(rat->defense == 0);
        REQUIRE(rat->glyph == "r");
        REQUIRE(rat->threat_level == 'a');
        
        // Orc Rookling
        auto orc = manager.createMonster("orc_rookling", 1, 1);
        REQUIRE(orc != nullptr);
        REQUIRE(orc->name == "Orc Rookling");
        REQUIRE(orc->hp == 8);
        REQUIRE(orc->attack == 4);
        REQUIRE(orc->defense == 1);
        REQUIRE(orc->can_open_doors == true);
        
        // Cave Spider
        auto spider = manager.createMonster("cave_spider", 2, 2);
        REQUIRE(spider != nullptr);
        REQUIRE(spider->can_see_invisible == true);
        
        // Kobold
        auto kobold = manager.createMonster("kobold", 3, 3);
        REQUIRE(kobold != nullptr);
        REQUIRE(kobold->aggressive == false);  // Kobolds are cowardly
        
        // Zombie
        auto zombie = manager.createMonster("zombie", 4, 4);
        REQUIRE(zombie != nullptr);
        REQUIRE(zombie->hp == 12);
        REQUIRE(zombie->speed == 80);  // Slow
    }
    
    SECTION("Verify threat levels") {
        factory.loadFromFile("data/monsters.json");
        
        REQUIRE(factory.getThreatLevel("gutter_rat") == 'a');
        REQUIRE(factory.getThreatLevel("cave_spider") == 'b');
        REQUIRE(factory.getThreatLevel("kobold") == 'b');
        REQUIRE(factory.getThreatLevel("orc_rookling") == 'c');
        REQUIRE(factory.getThreatLevel("zombie") == 'd');
    }
}