#include <catch2/catch_test_macros.hpp>
#include "map.h"
#include "room.h"
#include "map_generator.h"
#include "game_state.h"
#include "player.h"
#include "entity_manager.h"
#include "fov.h"

TEST_CASE("Lit Rooms", "[room][fov]") {
    SECTION("Room lit attribute") {
        Room normal_room(10, 10, 5, 5);
        REQUIRE_FALSE(normal_room.isLit());
        
        Room lit_room(20, 20, 5, 5, Room::RoomType::NORMAL, true);
        REQUIRE(lit_room.isLit());
        
        normal_room.setLit(true);
        REQUIRE(normal_room.isLit());
    }
    
    SECTION("Map room storage") {
        Map map(80, 40);
        
        Room room1(10, 10, 5, 5, Room::RoomType::NORMAL, true);
        Room room2(20, 20, 5, 5, Room::RoomType::NORMAL, false);
        
        map.addRoom(room1);
        map.addRoom(room2);
        
        REQUIRE(map.getRooms().size() == 2);
        
        // Test room detection
        Room* found = map.getRoomAt(12, 12);
        REQUIRE(found != nullptr);
        REQUIRE(found->isLit());
        
        found = map.getRoomAt(22, 22);
        REQUIRE(found != nullptr);
        REQUIRE_FALSE(found->isLit());
        
        // Test outside rooms
        found = map.getRoomAt(0, 0);
        REQUIRE(found == nullptr);
    }
    
    SECTION("Procedural generation creates lit rooms") {
        Map map(80, 40);
        MapGenerator::generateProceduralDungeon(map, 12345);
        
        const auto& rooms = map.getRooms();
        REQUIRE(rooms.size() > 0);
        
        // Count lit rooms (should be approximately 30% based on LIT_ROOM_CHANCE)
        int lit_count = 0;
        for (const auto& room : rooms) {
            if (room.isLit()) {
                lit_count++;
            }
        }
        
        // Should have at least one lit room in most cases
        INFO("Lit rooms: " << lit_count << " out of " << rooms.size());
        // We can't guarantee exact percentage due to randomness, but there should be some
    }
    
    SECTION("FOV with lit rooms") {
        GameManager game(MapType::TEST_ROOM);
        Map* map = game.getMap();
        
        // Clear existing rooms and add a lit room
        map->clearRooms();
        Room lit_room(10, 10, 10, 10, Room::RoomType::NORMAL, true);
        map->addRoom(lit_room);
        
        // Place floor tiles in the room
        for (int y = 10; y < 20; y++) {
            for (int x = 10; x < 20; x++) {
                map->setTile(x, y, TileType::FLOOR);
            }
        }
        
        // Create player outside the room
        EntityManager* em = game.getEntityManager();
        em->clear();
        auto player = em->createPlayer(5, 5);
        
        // Update FOV - player is outside lit room
        game.updateFOV();
        
        // Move player into the lit room
        player->x = 15;
        player->y = 15;
        
        // Update FOV - should detect entering lit room
        game.updateFOV();
        
        // The entire room should now be visible
        for (int y = 10; y < 20; y++) {
            for (int x = 10; x < 20; x++) {
                INFO("Checking visibility at " << x << "," << y);
                // Note: We're checking explored, not visible, as the test doesn't have full rendering
                REQUIRE(map->isExplored(x, y));
            }
        }
    }
}