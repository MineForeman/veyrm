#include <catch2/catch_test_macros.hpp>
#include "entity.h"
#include "map.h"
#include <ftxui/screen/color.hpp>

using namespace ftxui;

TEST_CASE("Entity: Basic properties", "[entity]") {
    Entity entity(10, 15, "@", Color::White, "test_entity");
    
    SECTION("Initial position") {
        REQUIRE(entity.x == 10);
        REQUIRE(entity.y == 15);
    }
    
    SECTION("Rendering properties") {
        REQUIRE(entity.glyph == "@");
        REQUIRE(entity.color == Color::White);
    }
    
    SECTION("Default properties") {
        REQUIRE(entity.blocks_movement == false);  // Default is false
        REQUIRE(entity.is_player == false);
        REQUIRE(entity.is_monster == false);
        REQUIRE(entity.is_item == false);
    }
    
    SECTION("Position modification") {
        entity.x = 20;
        entity.y = 25;
        REQUIRE(entity.x == 20);
        REQUIRE(entity.y == 25);
    }
}

TEST_CASE("Entity: Movement validation", "[entity]") {
    Map map(50, 50);
    Entity entity(10, 10, "@", Color::White, "test_entity");
    
    SECTION("Can move to floor tiles") {
        map.setTile(15, 15, TileType::FLOOR);
        REQUIRE(entity.canMoveTo(map, 15, 15) == true);
    }
    
    SECTION("Cannot move to wall tiles") {
        map.setTile(15, 15, TileType::WALL);
        REQUIRE(entity.canMoveTo(map, 15, 15) == false);
    }
    
    SECTION("Cannot move out of bounds") {
        REQUIRE(entity.canMoveTo(map, -1, 10) == false);
        REQUIRE(entity.canMoveTo(map, 10, -1) == false);
        REQUIRE(entity.canMoveTo(map, 50, 10) == false);
        REQUIRE(entity.canMoveTo(map, 10, 50) == false);
    }
    
    SECTION("Can move to stairs") {
        map.setTile(15, 15, TileType::STAIRS_DOWN);
        REQUIRE(entity.canMoveTo(map, 15, 15) == true);
        
        map.setTile(16, 16, TileType::STAIRS_UP);
        REQUIRE(entity.canMoveTo(map, 16, 16) == true);
    }
}

TEST_CASE("Entity: Component flags", "[entity]") {
    SECTION("Default entity has no components") {
        Entity entity(0, 0, "?", Color::White, "unknown");
        REQUIRE(entity.is_player == false);
        REQUIRE(entity.is_monster == false);
        REQUIRE(entity.is_item == false);
    }
    
    SECTION("Entity doesn't block movement by default") {
        Entity entity(0, 0, "?", Color::White, "unknown");
        REQUIRE(entity.blocks_movement == false);  // Default is false
    }
    
    SECTION("Non-blocking entity") {
        Entity entity(0, 0, "!", Color::Red, "item");
        entity.blocks_movement = false;
        REQUIRE(entity.blocks_movement == false);
    }
}

TEST_CASE("Entity: Virtual functions", "[entity]") {
    Entity entity(5, 5, "E", Color::Green, "entity");
    Map map(20, 20);
    
    SECTION("Update does nothing by default") {
        // Should not crash or throw
        REQUIRE_NOTHROW(entity.update(1.0));
    }
    
    SECTION("onDeath does nothing by default") {
        // Should not crash or throw
        REQUIRE_NOTHROW(entity.onDeath());
    }
    
    SECTION("onInteract does nothing by default") {
        Entity other(6, 6, "O", Color::Blue, "other");
        // Should not crash or throw
        REQUIRE_NOTHROW(entity.onInteract(other));
    }
}

TEST_CASE("Entity: Different entity types", "[entity]") {
    SECTION("Monster-like entity") {
        Entity monster(10, 10, "g", Color::Red, "goblin");
        monster.is_monster = true;
        monster.blocks_movement = true;
        
        REQUIRE(monster.is_monster == true);
        REQUIRE(monster.is_player == false);
        REQUIRE(monster.is_item == false);
        REQUIRE(monster.blocks_movement == true);
    }
    
    SECTION("Item-like entity") {
        Entity item(15, 15, "!", Color::Magenta, "potion");
        item.is_item = true;
        item.blocks_movement = false;
        
        REQUIRE(item.is_item == true);
        REQUIRE(item.is_player == false);
        REQUIRE(item.is_monster == false);
        REQUIRE(item.blocks_movement == false);
    }
    
    SECTION("Player-like entity") {
        Entity player(20, 20, "@", Color::White, "player");
        player.is_player = true;
        player.blocks_movement = true;
        
        REQUIRE(player.is_player == true);
        REQUIRE(player.is_monster == false);
        REQUIRE(player.is_item == false);
        REQUIRE(player.blocks_movement == true);
    }
}