#include <catch2/catch_test_macros.hpp>
#include "room.h"
#include "map_generator.h"
#include "map.h"
#include "config.h"
#include <random>

TEST_CASE("Room: Basic properties", "[room]") {
    Room room(10, 15, 8, 6);
    
    SECTION("Dimensions") {
        REQUIRE(room.x == 10);
        REQUIRE(room.y == 15);
        REQUIRE(room.width == 8);
        REQUIRE(room.height == 6);
    }
    
    SECTION("Boundaries") {
        REQUIRE(room.left() == 10);
        REQUIRE(room.right() == 17);  // 10 + 8 - 1
        REQUIRE(room.top() == 15);
        REQUIRE(room.bottom() == 20); // 15 + 6 - 1
    }
    
    SECTION("Center calculation") {
        Point center = room.center();
        REQUIRE(center.x == 14);  // 10 + 8/2
        REQUIRE(center.y == 18);  // 15 + 6/2
    }
    
    SECTION("Area calculation") {
        REQUIRE(room.area() == 48);  // 8 * 6
    }
    
    SECTION("Room type") {
        REQUIRE(room.type == Room::RoomType::NORMAL);
        
        Room special(5, 5, 10, 10, Room::RoomType::TREASURE);
        REQUIRE(special.type == Room::RoomType::TREASURE);
    }
}

TEST_CASE("Room: Overlap detection", "[room]") {
    Room room1(10, 10, 5, 5);
    
    SECTION("Overlapping rooms") {
        Room room2(12, 12, 5, 5);  // Overlaps
        REQUIRE(room1.overlaps(room2) == true);
        REQUIRE(room1.overlaps(room2, 0) == true);
    }
    
    SECTION("Adjacent rooms (touching)") {
        Room room2(15, 10, 5, 5);  // Touching but not overlapping
        REQUIRE(room1.overlaps(room2) == false);
        REQUIRE(room1.overlaps(room2, 0) == false);
    }
    
    SECTION("Adjacent rooms with padding") {
        Room room2(15, 10, 5, 5);  // Touching
        REQUIRE(room1.overlaps(room2, 1) == true);  // With padding, they overlap
        REQUIRE(room1.overlaps(room2, 2) == true);
    }
    
    SECTION("Distant rooms") {
        Room room2(20, 20, 5, 5);  // Far apart
        REQUIRE(room1.overlaps(room2) == false);
        REQUIRE(room1.overlaps(room2, 3) == true);  // With padding 3, they touch at corner
    }
    
    SECTION("Contained room") {
        Room room2(11, 11, 3, 3);  // Inside room1
        REQUIRE(room1.overlaps(room2) == true);
    }
}

TEST_CASE("Room: Point containment", "[room]") {
    Room room(10, 10, 5, 5);
    
    SECTION("Interior points") {
        REQUIRE(room.contains(12, 12) == true);  // Center
        REQUIRE(room.contains(10, 10) == true);  // Top-left corner
        REQUIRE(room.contains(14, 14) == true);  // Bottom-right corner
    }
    
    SECTION("Exterior points") {
        REQUIRE(room.contains(9, 10) == false);   // Just outside left
        REQUIRE(room.contains(15, 10) == false);  // Just outside right
        REQUIRE(room.contains(10, 9) == false);   // Just outside top
        REQUIRE(room.contains(10, 15) == false);  // Just outside bottom
    }
    
    SECTION("Point object containment") {
        Point inside(12, 12);
        Point outside(20, 20);
        REQUIRE(room.contains(inside) == true);
        REQUIRE(room.contains(outside) == false);
    }
}

TEST_CASE("Room: Perimeter and floor tiles", "[room]") {
    Room room(10, 10, 4, 4);
    
    SECTION("Perimeter points") {
        auto perimeter = room.getPerimeter();
        // 4x4 room has 12 perimeter points (4*4 - 2*2 interior)
        REQUIRE(perimeter.size() == 12);
        
        // Check corners are included
        bool has_top_left = false;
        bool has_bottom_right = false;
        for (const auto& p : perimeter) {
            if (p.x == 10 && p.y == 10) has_top_left = true;
            if (p.x == 13 && p.y == 13) has_bottom_right = true;
        }
        REQUIRE(has_top_left == true);
        REQUIRE(has_bottom_right == true);
    }
    
    SECTION("Floor tiles") {
        auto floor = room.getFloorTiles();
        // 4x4 room has 2x2 = 4 floor tiles (interior only)
        REQUIRE(floor.size() == 4);
        
        // Check all floor tiles are interior
        for (const auto& p : floor) {
            REQUIRE(p.x > room.x);
            REQUIRE(p.x < room.x + room.width - 1);
            REQUIRE(p.y > room.y);
            REQUIRE(p.y < room.y + room.height - 1);
        }
    }
}

TEST_CASE("Room: Validation", "[room]") {
    SECTION("Valid rooms") {
        Room room1(0, 0, 3, 3);  // Minimum valid size
        REQUIRE(room1.isValid() == true);
        
        Room room2(0, 0, 10, 10);  // Normal size
        REQUIRE(room2.isValid() == true);
    }
    
    SECTION("Invalid rooms") {
        Room room1(0, 0, 2, 3);  // Too narrow
        REQUIRE(room1.isValid() == false);
        
        Room room2(0, 0, 3, 2);  // Too short
        REQUIRE(room2.isValid() == false);
        
        Room room3(0, 0, 1, 1);  // Way too small
        REQUIRE(room3.isValid() == false);
    }
}

TEST_CASE("MapGenerator: Random room generation", "[map_generator]") {
    Map map(198, 66);  // Use full Angband-sized map for room generation tests

    SECTION("Generate with random seed") {
        auto rooms = MapGenerator::generateRandomRooms(map);

        // Should generate between config min and max rooms (not the old static constants)
        Config& config = Config::getInstance();
        REQUIRE(rooms.size() >= static_cast<size_t>(config.getMinRooms()));
        REQUIRE(rooms.size() <= static_cast<size_t>(config.getMaxRooms()));
        
        // All rooms should be valid
        for (const auto& room : rooms) {
            REQUIRE(room.isValid() == true);
        }
    }
    
    SECTION("Generate with fixed seed for reproducibility") {
        unsigned int seed = 12345;
        auto rooms1 = MapGenerator::generateRandomRooms(map, seed);
        
        Map map2(198, 66);  // Same size for reproducibility
        auto rooms2 = MapGenerator::generateRandomRooms(map2, seed);
        
        // Same seed should produce same rooms
        REQUIRE(rooms1.size() == rooms2.size());
        for (size_t i = 0; i < rooms1.size(); i++) {
            REQUIRE(rooms1[i].x == rooms2[i].x);
            REQUIRE(rooms1[i].y == rooms2[i].y);
            REQUIRE(rooms1[i].width == rooms2[i].width);
            REQUIRE(rooms1[i].height == rooms2[i].height);
        }
    }
    
    SECTION("No overlapping rooms") {
        auto rooms = MapGenerator::generateRandomRooms(map, 54321);
        
        // Check all pairs of rooms for overlaps
        for (size_t i = 0; i < rooms.size(); i++) {
            for (size_t j = i + 1; j < rooms.size(); j++) {
                // Rooms should not overlap even with 1 tile padding
                REQUIRE(rooms[i].overlaps(rooms[j], 1) == false);
            }
        }
    }
    
    SECTION("Room size constraints") {
        auto rooms = MapGenerator::generateRandomRooms(map, 99999);
        
        Config& config = Config::getInstance();
        for (const auto& room : rooms) {
            REQUIRE(room.width >= config.getMinRoomSize());
            REQUIRE(room.width <= config.getMaxRoomSize());
            REQUIRE(room.height >= config.getMinRoomSize());
            REQUIRE(room.height <= config.getMaxRoomSize());
        }
    }
    
    SECTION("Rooms carved into map") {
        auto rooms = MapGenerator::generateRandomRooms(map, 11111);
        
        // Check that each room has floor tiles
        for (const auto& room : rooms) {
            Point center = room.center();
            REQUIRE(map.getTile(center.x, center.y) == TileType::FLOOR);
            
            // Check room has walls
            REQUIRE(map.getTile(room.x, room.y) == TileType::WALL);
            REQUIRE(map.getTile(room.right(), room.bottom()) == TileType::WALL);
        }
    }
}

TEST_CASE("MapGenerator: Procedural dungeon generation", "[map_generator]") {
    Map map(198, 66);  // Use full Angband-sized map
    
    SECTION("Generate complete dungeon") {
        MapGenerator::generateProceduralDungeon(map, 42);

        // Should have floor tiles
        int floor_count = 0;
        int wall_count = 0;
        bool has_stairs = false;
        int stairs_up_count = 0;
        int door_count = 0;
        
        for (int y = 0; y < map.getHeight(); y++) {
            for (int x = 0; x < map.getWidth(); x++) {
                TileType tile = map.getTile(x, y);
                if (tile == TileType::FLOOR) floor_count++;
                if (tile == TileType::WALL) wall_count++;
                if (tile == TileType::STAIRS_DOWN) {
                    has_stairs = true;
                    floor_count++; // Stairs are walkable
                }
                if (tile == TileType::STAIRS_UP) {
                    stairs_up_count++;
                }
                if (tile == TileType::DOOR_CLOSED || tile == TileType::DOOR_OPEN) {
                    door_count++;
                }
            }
        }

        REQUIRE(floor_count > 50);  // Should have substantial floor space
        REQUIRE(wall_count > 50);   // Should have walls

        INFO("Floor count: " << floor_count);
        INFO("Wall count: " << wall_count);
        INFO("Has stairs down: " << has_stairs);
        INFO("Stairs up count: " << stairs_up_count);
        INFO("Door count: " << door_count);

        REQUIRE(has_stairs == true); // Should have stairs
    }
    
    SECTION("Small map handling") {
        Map small_map(30, 20);
        MapGenerator::generateProceduralDungeon(small_map, 123);
        
        // Should still generate at least one room
        int floor_count = 0;
        for (int y = 0; y < small_map.getHeight(); y++) {
            for (int x = 0; x < small_map.getWidth(); x++) {
                if (small_map.getTile(x, y) == TileType::FLOOR ||
                    small_map.getTile(x, y) == TileType::STAIRS_DOWN) {
                    floor_count++;
                }
            }
        }
        
        REQUIRE(floor_count > 0);  // At least one room was created
    }
}

TEST_CASE("MapGenerator: Edge cases", "[map_generator]") {
    SECTION("Very small map") {
        Map tiny_map(20, 10);
        auto rooms = MapGenerator::generateRandomRooms(tiny_map, 777);
        
        // Should create at least one room even in tiny map
        REQUIRE(rooms.size() > 0);
        
        // Room should fit within bounds
        for (const auto& room : rooms) {
            REQUIRE(room.x >= 0);
            REQUIRE(room.y >= 0);
            REQUIRE(room.right() < tiny_map.getWidth());
            REQUIRE(room.bottom() < tiny_map.getHeight());
        }
    }
    
    SECTION("Minimum room always generated") {
        // Even with impossible constraints, should generate at least one room
        Map map(80, 24);
        std::mt19937 rng(888);
        
        // This should trigger the emergency room creation
        auto rooms = MapGenerator::generateRandomRooms(map, rng);
        REQUIRE(rooms.size() >= 1);
    }
}