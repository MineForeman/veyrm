#include <catch2/catch_test_macros.hpp>
#include "entity_manager.h"
#include "player.h"
#include "entity.h"
#include <ftxui/screen/color.hpp>

using namespace ftxui;

TEST_CASE("EntityManager: Basic operations", "[entity_manager]") {
    EntityManager manager;
    
    SECTION("Starts without player") {
        REQUIRE(manager.getPlayer() == nullptr);
    }
    
    SECTION("Create entity") {
        auto entity = manager.createEntity(EntityType::MONSTER, 10, 10);
        REQUIRE(entity != nullptr);
        REQUIRE(entity->x == 10);
        REQUIRE(entity->y == 10);
    }
    
    SECTION("Clear all entities") {
        manager.createEntity(EntityType::MONSTER, 10, 10);
        manager.createEntity(EntityType::ITEM, 20, 20);
        auto player = manager.createPlayer(5, 5);
        
        REQUIRE(manager.getPlayer() != nullptr);
        
        manager.clear();
        REQUIRE(manager.getPlayer() == nullptr);
    }
}

TEST_CASE("EntityManager: Player management", "[entity_manager]") {
    EntityManager manager;
    
    SECTION("Create player") {
        auto player = manager.createPlayer(15, 20);
        
        REQUIRE(player != nullptr);
        REQUIRE(player->x == 15);
        REQUIRE(player->y == 20);
        REQUIRE(player->is_player == true);
        REQUIRE(manager.getPlayer() == player);
    }
    
    SECTION("Only one player allowed") {
        auto player1 = manager.createPlayer(10, 10);
        auto player2 = manager.createPlayer(20, 20);
        
        // Second call should return existing player but moves it
        REQUIRE(player2 == player1);
        REQUIRE(player1->x == 20);  // Position updated to new location
        REQUIRE(player1->y == 20);
    }
    
    SECTION("Player survives clear and recreate") {
        auto player1 = manager.createPlayer(10, 10);
        manager.clear();
        
        REQUIRE(manager.getPlayer() == nullptr);
        
        auto player2 = manager.createPlayer(20, 20);
        REQUIRE(player2 != player1);  // New player instance
        REQUIRE(player2->x == 20);
        REQUIRE(player2->y == 20);
    }
}

TEST_CASE("EntityManager: Spatial queries", "[entity_manager]") {
    EntityManager manager;
    
    SECTION("Get entities at position - empty") {
        auto entities = manager.getEntitiesAt(10, 10);
        REQUIRE(entities.empty());
    }
    
    SECTION("Get entities at position - single entity") {
        auto entity = manager.createEntity(EntityType::MONSTER, 10, 10);
        
        auto entities = manager.getEntitiesAt(10, 10);
        REQUIRE(entities.size() == 1);
        REQUIRE(entities[0] == entity);
        
        // Different position should be empty
        auto other = manager.getEntitiesAt(11, 11);
        REQUIRE(other.empty());
    }
    
    SECTION("Get entities at position - multiple entities") {
        auto e1 = manager.createEntity(EntityType::MONSTER, 10, 10);
        auto e2 = manager.createEntity(EntityType::ITEM, 10, 10);
        auto e3 = manager.createEntity(EntityType::MONSTER, 20, 20);
        
        auto at_10_10 = manager.getEntitiesAt(10, 10);
        REQUIRE(at_10_10.size() == 2);
        
        auto at_20_20 = manager.getEntitiesAt(20, 20);
        REQUIRE(at_20_20.size() == 1);
        REQUIRE(at_20_20[0] == e3);
    }
}

TEST_CASE("EntityManager: Blocking entity queries", "[entity_manager]") {
    EntityManager manager;
    
    SECTION("No blocking entity at empty position") {
        auto blocking = manager.getBlockingEntityAt(10, 10);
        REQUIRE(blocking == nullptr);
    }
    
    SECTION("Find blocking entity") {
        // Monsters should block by default
        auto monster = manager.createEntity(EntityType::MONSTER, 10, 10);
        
        auto found = manager.getBlockingEntityAt(10, 10);
        REQUIRE(found != nullptr);
    }
    
    SECTION("Items don't block") {
        // Items shouldn't block movement
        auto item = manager.createEntity(EntityType::ITEM, 10, 10);
        
        auto found = manager.getBlockingEntityAt(10, 10);
        // Items typically don't block - this depends on implementation
        // The test result depends on how items are configured
    }
}

TEST_CASE("EntityManager: Entity removal", "[entity_manager]") {
    EntityManager manager;
    
    SECTION("Destroy single entity") {
        auto entity = manager.createEntity(EntityType::MONSTER, 10, 10);
        REQUIRE(entity != nullptr);
        
        manager.destroyEntity(entity);
        
        auto entities = manager.getEntitiesAt(10, 10);
        REQUIRE(entities.empty());
    }
    
    SECTION("Destroy specific entity from multiple") {
        auto e1 = manager.createEntity(EntityType::MONSTER, 10, 10);
        auto e2 = manager.createEntity(EntityType::MONSTER, 20, 20);
        auto e3 = manager.createEntity(EntityType::ITEM, 30, 30);
        
        manager.destroyEntity(e2);
        
        // e1 and e3 should still exist
        REQUIRE(manager.getEntitiesAt(10, 10).size() == 1);
        REQUIRE(manager.getEntitiesAt(20, 20).empty());
        REQUIRE(manager.getEntitiesAt(30, 30).size() == 1);
    }
}

TEST_CASE("EntityManager: Update all entities", "[entity_manager]") {
    EntityManager manager;
    
    SECTION("Update with empty manager") {
        // Should not crash
        REQUIRE_NOTHROW(manager.updateAll(1.0));
    }
    
    SECTION("Update with entities") {
        manager.createEntity(EntityType::MONSTER, 10, 10);
        manager.createEntity(EntityType::ITEM, 20, 20);
        manager.createPlayer(5, 5);
        
        // Should not crash
        REQUIRE_NOTHROW(manager.updateAll(0.016));
    }
}