#include <catch2/catch_test_macros.hpp>
#include "monster.h"
#include "monster_factory.h"
#include "entity_manager.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST_CASE("Monster Class", "[monster]") {
    SECTION("Basic monster creation") {
        Monster monster(10, 10, "test_monster");
        
        REQUIRE(monster.x == 10);
        REQUIRE(monster.y == 10);
        REQUIRE(monster.species == "test_monster");
        REQUIRE(monster.getType() == EntityType::MONSTER);
        REQUIRE(monster.isBlocking() == true);
    }
    
    SECTION("Monster stats") {
        Monster monster(0, 0, "test");
        monster.setStats(10, 10, 5, 2, 100, 50);
        
        REQUIRE(monster.hp == 10);
        REQUIRE(monster.max_hp == 10);
        REQUIRE(monster.attack == 5);
        REQUIRE(monster.defense == 2);
        REQUIRE(monster.speed == 100);
        REQUIRE(monster.xp_value == 50);
    }
    
    SECTION("Monster damage") {
        Monster monster(0, 0, "test");
        monster.setStats(20, 20, 5, 2, 100, 50);
        
        // Take damage with defense
        monster.takeDamage(5);
        REQUIRE(monster.hp == 17); // 5 - 2 defense = 3 damage
        
        // Minimum damage is 1
        monster.takeDamage(1);
        REQUIRE(monster.hp == 16); // 1 damage even with 2 defense
        
        // Death check
        monster.takeDamage(100);
        REQUIRE(monster.hp == 0);
        REQUIRE(monster.isDead() == true);
    }
    
    SECTION("Monster metadata") {
        Monster monster(0, 0, "test");
        monster.setMetadata("Test Monster", "A test creature", "T", 
                          ftxui::Color::Red, 'c');
        
        REQUIRE(monster.name == "Test Monster");
        REQUIRE(monster.description == "A test creature");
        REQUIRE(monster.glyph == "T");
        REQUIRE(monster.threat_level == 'c');
    }
    
    SECTION("Monster flags") {
        Monster monster(0, 0, "test");
        
        // Default flags
        REQUIRE(monster.aggressive == true);
        REQUIRE(monster.can_open_doors == false);
        REQUIRE(monster.can_see_invisible == false);
        
        // Set flags
        monster.setFlags(false, true, true);
        REQUIRE(monster.aggressive == false);
        REQUIRE(monster.can_open_doors == true);
        REQUIRE(monster.can_see_invisible == true);
    }
}

TEST_CASE("MonsterFactory", "[monster][factory]") {
    MonsterFactory& factory = MonsterFactory::getInstance();
    
    SECTION("Load from JSON") {
        json data = {
            {"monsters", {
                {
                    {"id", "test_goblin"},
                    {"name", "Test Goblin"},
                    {"description", "A test goblin"},
                    {"glyph", "g"},
                    {"color", "green"},
                    {"hp", 5},
                    {"attack", 3},
                    {"defense", 1},
                    {"speed", 110},
                    {"xp_value", 10},
                    {"threat_level", "b"},
                    {"flags", {
                        {"aggressive", true},
                        {"can_open_doors", false}
                    }}
                }
            }}
        };
        
        factory.clearTemplates();
        REQUIRE(factory.loadFromJson(data) == true);
        REQUIRE(factory.hasSpecies("test_goblin") == true);
        
        auto available = factory.getAvailableSpecies();
        REQUIRE(available.size() == 1);
        REQUIRE(available[0] == "test_goblin");
    }
    
    SECTION("Create monster from template") {
        json data = {
            {"monsters", {
                {
                    {"id", "test_orc"},
                    {"name", "Test Orc"},
                    {"glyph", "o"},
                    {"color", "green"},
                    {"hp", 8},
                    {"attack", 4},
                    {"defense", 1},
                    {"speed", 100},
                    {"xp_value", 15},
                    {"threat_level", "c"}
                }
            }}
        };
        
        factory.clearTemplates();
        factory.loadFromJson(data);
        
        auto monster = factory.createMonster("test_orc", 5, 5);
        REQUIRE(monster != nullptr);
        REQUIRE(monster->x == 5);
        REQUIRE(monster->y == 5);
        REQUIRE(monster->species == "test_orc");
        REQUIRE(monster->name == "Test Orc");
        REQUIRE(monster->hp == 8);
        REQUIRE(monster->attack == 4);
        REQUIRE(monster->defense == 1);
        REQUIRE(monster->glyph == "o");
        REQUIRE(monster->threat_level == 'c');
    }
    
    SECTION("Invalid species") {
        factory.clearTemplates();
        
        auto monster = factory.createMonster("nonexistent", 0, 0);
        REQUIRE(monster == nullptr);
        
        REQUIRE(factory.hasSpecies("nonexistent") == false);
    }
}

TEST_CASE("Monster EntityManager Integration", "[monster][entity]") {
    EntityManager manager;
    MonsterFactory& factory = MonsterFactory::getInstance();
    
    // Load test monster
    json data = {
        {"monsters", {
            {
                {"id", "test_rat"},
                {"name", "Test Rat"},
                {"glyph", "r"},
                {"color", "brown"},
                {"hp", 3},
                {"attack", 2},
                {"defense", 0},
                {"speed", 120},
                {"xp_value", 2},
                {"threat_level", "a"}
            }
        }}
    };
    
    factory.clearTemplates();
    factory.loadFromJson(data);
    
    SECTION("Create monster through EntityManager") {
        auto monster = manager.createMonster("test_rat", 10, 10);
        
        REQUIRE(monster != nullptr);
        REQUIRE(monster->species == "test_rat");
        REQUIRE(monster->x == 10);
        REQUIRE(monster->y == 10);
        
        // Check it's in the entity list
        auto entities = manager.getEntitiesAt(10, 10);
        REQUIRE(entities.size() == 1);
        
        auto monsters = manager.getMonsters();
        REQUIRE(monsters.size() == 1);
    }
    
    SECTION("Get monster at position") {
        manager.createMonster("test_rat", 5, 5);
        
        auto monster = manager.getMonsterAt(5, 5);
        REQUIRE(monster != nullptr);
        REQUIRE(monster->species == "test_rat");
        
        // No monster at empty position
        auto empty = manager.getMonsterAt(0, 0);
        REQUIRE(empty == nullptr);
    }
    
    SECTION("Monster blocking") {
        manager.createMonster("test_rat", 3, 3);
        
        auto blocking = manager.getBlockingEntityAt(3, 3);
        REQUIRE(blocking != nullptr);
        
        bool blocked = manager.isPositionBlocked(3, 3);
        REQUIRE(blocked == true);
    }
}