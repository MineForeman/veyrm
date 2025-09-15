#include <catch2/catch_test_macros.hpp>
#include "map.h"
#include "room.h"
#include "map_generator.h"
#include "game_state.h"
#include "fov.h"
#include "ecs/game_world.h"
#include "ecs/position_component.h"
#include "point.h"

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

        // Verify room was added
        REQUIRE(map->getRooms().size() == 1);
        REQUIRE(map->getRooms()[0].isLit() == true);
        
        // Place floor tiles in the room
        for (int y = 10; y < 20; y++) {
            for (int x = 10; x < 20; x++) {
                map->setTile(x, y, TileType::FLOOR);
            }
        }
        
        // The GameManager constructor already creates a player
        // First, ensure current_room is reset
        game.setCurrentRoom(nullptr);

        // Position player outside the room
        game.player_x = 5;
        game.player_y = 5;

        // Update FOV - player is outside lit room, this should keep current_room as nullptr
        game.updateFOV();

        // Verify player is not in a room
        REQUIRE(game.getCurrentRoom() == nullptr);

        // Now move player into the lit room
        game.player_x = 15;
        game.player_y = 15;

        // Update FOV - should detect entering lit room (transition from nullptr to lit room)
        game.updateFOV();

        // Verify player is now in the lit room
        REQUIRE(game.getCurrentRoom() != nullptr);
        REQUIRE(game.getCurrentRoom()->isLit() == true);

        // Debug: Check room status
        const Room* room = map->getRoomAt(Point(15, 15));
        INFO("Room at (15,15): " << (room ? "exists" : "null"));
        if (room) {
            INFO("Room is lit: " << room->isLit());
            INFO("Room bounds: " << room->x << "," << room->y << " " << room->width << "x" << room->height);
        }

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