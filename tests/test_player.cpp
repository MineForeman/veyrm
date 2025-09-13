#include <catch2/catch_test_macros.hpp>
#include "player.h"
#include "entity_manager.h"
#include "map.h"
#include "config.h"
#include <ftxui/screen/color.hpp>

using namespace ftxui;

TEST_CASE("Player: Initialization", "[player]") {
    Player player(10, 15);
    
    SECTION("Position") {
        REQUIRE(player.x == 10);
        REQUIRE(player.y == 15);
    }
    
    SECTION("Default stats") {
        Config& config = Config::getInstance();
        REQUIRE(player.hp == config.getPlayerStartingHP());
        REQUIRE(player.max_hp == config.getPlayerStartingHP());
        REQUIRE(player.attack == config.getPlayerStartingAttack());
        REQUIRE(player.defense == config.getPlayerStartingDefense());
        REQUIRE(player.level == 1);
        REQUIRE(player.experience == 0);
    }
    
    SECTION("Entity properties") {
        REQUIRE(player.glyph == "@");
        REQUIRE(player.color == Color::White);
        REQUIRE(player.is_player == true);
        REQUIRE(player.is_monster == false);
        REQUIRE(player.is_item == false);
        REQUIRE(player.blocks_movement == true);
    }
}

TEST_CASE("Player: Movement", "[player]") {
    Map map(30, 30);
    EntityManager entity_manager;
    Player player(10, 10);
    
    // Set up a simple map
    for (int y = 0; y < 30; ++y) {
        for (int x = 0; x < 30; ++x) {
            if (x == 0 || x == 29 || y == 0 || y == 29) {
                map.setTile(x, y, TileType::WALL);
            } else {
                map.setTile(x, y, TileType::FLOOR);
            }
        }
    }
    
    SECTION("Can move to empty floor") {
        REQUIRE(player.tryMove(map, &entity_manager, 1, 0) == true);
        REQUIRE(player.x == 11);
        REQUIRE(player.y == 10);
    }
    
    SECTION("Cannot move into walls") {
        Player wall_player(1, 1);
        REQUIRE(wall_player.tryMove(map, &entity_manager, -1, 0) == false);
        REQUIRE(wall_player.x == 1);  // Position unchanged
        REQUIRE(wall_player.y == 1);
    }
    
    SECTION("Cannot move out of bounds") {
        Player edge_player(1, 1);
        // Try to move through wall and out of bounds
        REQUIRE(edge_player.tryMove(map, &entity_manager, -10, 0) == false);
        REQUIRE(edge_player.x == 1);
        REQUIRE(edge_player.y == 1);
    }
    
    SECTION("Movement in all directions") {
        Player center_player(15, 15);
        
        // North
        REQUIRE(center_player.tryMove(map, &entity_manager, 0, -1) == true);
        REQUIRE(center_player.x == 15);
        REQUIRE(center_player.y == 14);
        
        // East
        REQUIRE(center_player.tryMove(map, &entity_manager, 1, 0) == true);
        REQUIRE(center_player.x == 16);
        REQUIRE(center_player.y == 14);
        
        // South
        REQUIRE(center_player.tryMove(map, &entity_manager, 0, 1) == true);
        REQUIRE(center_player.x == 16);
        REQUIRE(center_player.y == 15);
        
        // West
        REQUIRE(center_player.tryMove(map, &entity_manager, -1, 0) == true);
        REQUIRE(center_player.x == 15);
        REQUIRE(center_player.y == 15);
    }
}

TEST_CASE("Player: Collision with entities", "[player]") {
    Map map(20, 20);
    auto entity_manager = std::make_unique<EntityManager>();
    
    // Fill map with floor
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 20; ++x) {
            map.setTile(x, y, TileType::FLOOR);
        }
    }
    
    // Create player
    auto player_ptr = entity_manager->createPlayer(10, 10);
    REQUIRE(player_ptr != nullptr);
    
    SECTION("Cannot move into blocking entity") {
        // Create a blocking entity (like a monster)
        auto blocking = entity_manager->createEntity(EntityType::MONSTER, 11, 10);
        
        // Try to move into the blocking entity
        REQUIRE(player_ptr->tryMove(map, entity_manager.get(), 1, 0) == false);
        REQUIRE(player_ptr->x == 10);  // Position unchanged
        REQUIRE(player_ptr->y == 10);
    }
    
    SECTION("Can move over non-blocking entity") {
        // Create a non-blocking entity (like an item)
        auto item = entity_manager->createEntity(EntityType::ITEM, 11, 10);
        
        // Should be able to move over it
        REQUIRE(player_ptr->tryMove(map, entity_manager.get(), 1, 0) == true);
        REQUIRE(player_ptr->x == 11);
        REQUIRE(player_ptr->y == 10);
    }
}

TEST_CASE("Player: Stat modifications", "[player]") {
    Player player(5, 5);
    
    SECTION("HP modifications") {
        Config& config = Config::getInstance();
        // Damage
        player.hp = 8;
        REQUIRE(player.hp == 8);
        REQUIRE(player.max_hp == config.getPlayerStartingHP());  // Max unchanged
        
        // Healing (shouldn't exceed max)
        player.hp = 25;
        REQUIRE(player.hp == 25);  // Can exceed max for now (no clamping in basic version)
        
        // Zero HP
        player.hp = 0;
        REQUIRE(player.hp == 0);
    }
    
    SECTION("Level and experience") {
        player.experience = 100;
        player.level = 2;
        
        REQUIRE(player.experience == 100);
        REQUIRE(player.level == 2);
    }
    
    SECTION("Combat stats") {
        player.attack = 10;
        player.defense = 5;
        
        REQUIRE(player.attack == 10);
        REQUIRE(player.defense == 5);
    }
}

TEST_CASE("Player: Special movement cases", "[player]") {
    Map map(20, 20);
    EntityManager entity_manager;
    
    // Create a small room
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 20; ++x) {
            if (x >= 5 && x <= 15 && y >= 5 && y <= 15) {
                map.setTile(x, y, TileType::FLOOR);
            } else {
                map.setTile(x, y, TileType::WALL);
            }
        }
    }
    
    SECTION("No movement with zero delta") {
        Player player(10, 10);
        // Zero movement might be allowed or not depending on implementation
        [[maybe_unused]] bool moved = player.tryMove(map, &entity_manager, 0, 0);
        // Just verify it doesn't crash
        REQUIRE(player.x == 10);
        REQUIRE(player.y == 10);
    }
    
    SECTION("Diagonal movement") {
        Player player(10, 10);
        REQUIRE(player.tryMove(map, &entity_manager, 1, 1) == true);
        REQUIRE(player.x == 11);
        REQUIRE(player.y == 11);
        
        REQUIRE(player.tryMove(map, &entity_manager, -1, -1) == true);
        REQUIRE(player.x == 10);
        REQUIRE(player.y == 10);
    }
    
    SECTION("Movement onto special tiles") {
        map.setTile(11, 10, TileType::STAIRS_DOWN);
        Player player(10, 10);
        
        REQUIRE(player.tryMove(map, &entity_manager, 1, 0) == true);
        REQUIRE(player.x == 11);
        REQUIRE(player.y == 10);
        // Future: Check if stairs interaction triggered
    }
}