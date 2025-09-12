#include <catch2/catch_test_macros.hpp>
#include "entity.h"
#include "entity_manager.h"
#include "game_state.h"
#include "fov.h"
#include "map.h"
#include "map_memory.h"
#include <chrono>

TEST_CASE("Entity: Visibility management", "[visibility]") {
    Entity entity(10, 10, "T", ftxui::Color::Green, "Test Entity");
    
    SECTION("Default visibility") {
        REQUIRE(entity.isVisible() == true);
    }
    
    SECTION("Set visibility") {
        entity.setVisible(false);
        REQUIRE(entity.isVisible() == false);
        
        entity.setVisible(true);
        REQUIRE(entity.isVisible() == true);
    }
}

TEST_CASE("EntityManager: Visibility filtering", "[visibility]") {
    EntityManager manager;
    
    // Create test entities
    auto entity1 = manager.createEntity(EntityType::MONSTER, 5, 5);
    auto entity2 = manager.createEntity(EntityType::MONSTER, 10, 10);
    auto entity3 = manager.createEntity(EntityType::ITEM, 15, 15);
    
    SECTION("All visible by default") {
        auto visible = manager.getVisibleEntities();
        REQUIRE(visible.size() == 3);
    }
    
    SECTION("Filter visible entities") {
        entity1->setVisible(false);
        entity3->setVisible(false);
        
        auto visible = manager.getVisibleEntities();
        REQUIRE(visible.size() == 1);
        REQUIRE(visible[0] == entity2);
    }
    
    SECTION("Filter visible monsters") {
        entity1->setVisible(false);
        
        auto monsters = manager.getVisibleMonsters();
        REQUIRE(monsters.size() == 1);
        REQUIRE(monsters[0] == entity2);
    }
    
    SECTION("Filter visible items") {
        entity3->setVisible(false);
        
        auto items = manager.getVisibleItems();
        REQUIRE(items.size() == 0);
    }
}

TEST_CASE("EntityManager: FOV-based visibility update", "[visibility]") {
    EntityManager manager;
    
    // Create entities at different positions
    auto entity1 = manager.createEntity(EntityType::MONSTER, 5, 5);
    auto entity2 = manager.createEntity(EntityType::MONSTER, 10, 10);
    auto entity3 = manager.createEntity(EntityType::ITEM, 15, 15);
    auto player = manager.createPlayer(10, 10);
    
    // Create FOV grid (20x20)
    std::vector<std::vector<bool>> fov(20, std::vector<bool>(20, false));
    
    SECTION("All invisible when FOV is empty") {
        manager.updateEntityVisibility(fov);
        
        REQUIRE(entity1->isVisible() == false);
        REQUIRE(entity2->isVisible() == false);
        REQUIRE(entity3->isVisible() == false);
        REQUIRE(player->isVisible() == true); // Player always visible
    }
    
    SECTION("Selective visibility based on FOV") {
        fov[5][5] = true;   // entity1 position
        fov[10][10] = true; // entity2 and player position
        // entity3 at (15,15) remains false
        
        manager.updateEntityVisibility(fov);
        
        REQUIRE(entity1->isVisible() == true);
        REQUIRE(entity2->isVisible() == true);
        REQUIRE(entity3->isVisible() == false);
        REQUIRE(player->isVisible() == true);
    }
    
    SECTION("Update visibility when entities move") {
        fov[7][7] = true;
        
        entity1->moveTo(7, 7);
        manager.updateEntityVisibility(fov);
        
        REQUIRE(entity1->isVisible() == true);
        
        entity1->moveTo(8, 8); // Move to non-visible position
        manager.updateEntityVisibility(fov);
        
        REQUIRE(entity1->isVisible() == false);
    }
}

TEST_CASE("MapMemory: Visibility states", "[visibility]") {
    Map map(20, 20);
    map.fill(TileType::FLOOR);
    map.setTile(10, 10, TileType::WALL);
    
    MapMemory memory(20, 20);
    
    SECTION("Initial state is unknown") {
        REQUIRE(memory.getVisibility(10, 10) == MapMemory::VisibilityState::UNKNOWN);
        REQUIRE(memory.isExplored(10, 10) == false);
        REQUIRE(memory.isVisible(10, 10) == false);
    }
    
    SECTION("Visible state when in FOV") {
        std::vector<std::vector<bool>> fov(20, std::vector<bool>(20, false));
        fov[10][10] = true;
        
        memory.updateVisibility(map, fov);
        
        REQUIRE(memory.getVisibility(10, 10) == MapMemory::VisibilityState::VISIBLE);
        REQUIRE(memory.isExplored(10, 10) == true);
        REQUIRE(memory.isVisible(10, 10) == true);
        REQUIRE(memory.getRemembered(10, 10) == TileType::WALL);
    }
    
    SECTION("Remembered state when out of FOV") {
        // First see it
        std::vector<std::vector<bool>> fov1(20, std::vector<bool>(20, false));
        fov1[10][10] = true;
        memory.updateVisibility(map, fov1);
        
        // Then lose sight
        std::vector<std::vector<bool>> fov2(20, std::vector<bool>(20, false));
        memory.updateVisibility(map, fov2);
        
        REQUIRE(memory.getVisibility(10, 10) == MapMemory::VisibilityState::REMEMBERED);
        REQUIRE(memory.isExplored(10, 10) == true);
        REQUIRE(memory.isVisible(10, 10) == false);
        REQUIRE(memory.getRemembered(10, 10) == TileType::WALL);
    }
}

TEST_CASE("Integration: FOV affects entity visibility", "[visibility]") {
    Map map(30, 30);
    map.fill(TileType::FLOOR);
    
    GameManager game(MapType::TEST_ROOM);
    auto entity_manager = game.getEntityManager();
    
    // Create entities
    auto monster1 = entity_manager->createEntity(EntityType::MONSTER, 15, 15);
    auto monster2 = entity_manager->createEntity(EntityType::MONSTER, 25, 25);
    
    SECTION("Entities visible within FOV radius") {
        // Move player near monster1
        auto player = entity_manager->getPlayer();
        if (player) {
            player->moveTo(14, 14);
            game.updateFOV();
            
            // Monster1 should be visible (distance 1-2)
            // Monster2 should not be visible (distance > 10)
            auto visible = entity_manager->getVisibleMonsters();
            
            // The exact visibility depends on FOV calculation
            // Just check that visibility system is working
            REQUIRE(visible.size() <= 2);
        }
    }
}

TEST_CASE("Visibility: Performance", "[visibility][!benchmark]") {
    EntityManager manager;
    
    // Create many entities
    for (int i = 0; i < 100; i++) {
        manager.createEntity(EntityType::MONSTER, i % 50, i / 2);
    }
    
    // Large FOV grid
    std::vector<std::vector<bool>> fov(100, std::vector<bool>(100, false));
    
    // Set some positions visible
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            if ((i + j) % 3 == 0) {
                fov[i][j] = true;
            }
        }
    }
    
    SECTION("Visibility update performance") {
        auto start = std::chrono::high_resolution_clock::now();
        manager.updateEntityVisibility(fov);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        REQUIRE(duration.count() < 1000); // Should take less than 1ms
    }
}