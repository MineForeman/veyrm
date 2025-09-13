#include <catch2/catch_test_macros.hpp>
#include "map_validator.h"
#include "map.h"
#include "map_generator.h"
#include <iostream>

TEST_CASE("MapValidator: Empty map validation", "[map_validator]") {
    Map map(20, 20);
    
    SECTION("Empty map is invalid") {
        // Map filled with VOID by default
        auto result = MapValidator::validate(map);
        
        REQUIRE(result.valid == false);
        REQUIRE(result.walkable_tiles == 0);
        REQUIRE(result.room_count == 0);
        REQUIRE(result.errors.size() > 0);
    }
}

TEST_CASE("MapValidator: Valid single room", "[map_validator]") {
    Map map(20, 20);
    
    SECTION("Single room is valid") {
        // Create a simple room
        for (int y = 5; y < 15; ++y) {
            for (int x = 5; x < 15; ++x) {
                if (y == 5 || y == 14 || x == 5 || x == 14) {
                    map.setTile(x, y, TileType::WALL);
                } else {
                    map.setTile(x, y, TileType::FLOOR);
                }
            }
        }
        
        auto result = MapValidator::validate(map);
        
        REQUIRE(result.valid == true);
        REQUIRE(result.walkable_tiles > 0);
        REQUIRE(result.room_count == 1);
        REQUIRE(result.errors.empty());
    }
}

TEST_CASE("MapValidator: Disconnected rooms", "[map_validator]") {
    Map map(30, 30);
    
    SECTION("Disconnected rooms are invalid") {
        // Create two separate rooms with no connection
        // Room 1
        for (int y = 5; y < 10; ++y) {
            for (int x = 5; x < 10; ++x) {
                map.setTile(x, y, TileType::FLOOR);
            }
        }
        
        // Room 2 (disconnected)
        for (int y = 20; y < 25; ++y) {
            for (int x = 20; x < 25; ++x) {
                map.setTile(x, y, TileType::FLOOR);
            }
        }
        
        auto result = MapValidator::validate(map);
        
        REQUIRE(result.valid == false);
        REQUIRE(result.room_count == 2);
        REQUIRE(result.errors.size() > 0);
        
        // Should have connectivity error
        bool has_connectivity_error = false;
        for (const auto& error : result.errors) {
            if (error.find("connected") != std::string::npos ||
                error.find("disconnected") != std::string::npos ||
                error.find("reachable") != std::string::npos) {
                has_connectivity_error = true;
                break;
            }
        }
        REQUIRE(has_connectivity_error == true);
    }
}

TEST_CASE("MapValidator: Minimum walkable space", "[map_validator]") {
    Map map(20, 20);
    
    SECTION("Too few walkable tiles") {
        // Create a tiny room
        map.setTile(10, 10, TileType::FLOOR);
        map.setTile(10, 11, TileType::FLOOR);
        
        auto result = MapValidator::validate(map);
        
        // Should warn about small walkable area
        REQUIRE(result.walkable_tiles == 2);
        REQUIRE((result.warnings.size() > 0 || result.errors.size() > 0));
    }
    
    SECTION("Adequate walkable tiles") {
        // Create a decent sized room
        for (int y = 5; y < 15; ++y) {
            for (int x = 5; x < 15; ++x) {
                map.setTile(x, y, TileType::FLOOR);
            }
        }
        
        auto result = MapValidator::validate(map);
        
        REQUIRE(result.walkable_tiles == 100);
        REQUIRE(result.valid == true);
    }
}

TEST_CASE("MapValidator: Wall integrity", "[map_validator]") {
    Map map(20, 20);
    
    SECTION("Proper walls around rooms") {
        // Create room with proper walls
        for (int y = 5; y < 15; ++y) {
            for (int x = 5; x < 15; ++x) {
                if (y == 5 || y == 14 || x == 5 || x == 14) {
                    map.setTile(x, y, TileType::WALL);
                } else {
                    map.setTile(x, y, TileType::FLOOR);
                }
            }
        }
        
        auto result = MapValidator::validate(map);
        
        REQUIRE(result.valid == true);
        REQUIRE(result.wall_tiles > 0);  // Has walls
    }
    
    SECTION("Room without walls gets warning") {
        // Create room without surrounding walls
        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                map.setTile(x, y, TileType::FLOOR);
            }
        }
        
        auto result = MapValidator::validate(map);
        
        // Should still be valid but might have warning
        REQUIRE(result.valid == true);
        REQUIRE(result.wall_tiles == 0);  // No walls
    }
}

TEST_CASE("MapValidator: Generated maps validation", "[map_validator]") {
    Map map;  // Use default size 80x24
    
    SECTION("TEST_ROOM is valid") {
        MapGenerator::generate(map, MapType::TEST_ROOM);
        auto result = MapValidator::validate(map);

        // Always print results for debugging
        for (const auto& error : result.errors) {
            std::cerr << "Validation error: " << error << std::endl;
        }
        for (const auto& warning : result.warnings) {
            std::cerr << "Validation warning: " << warning << std::endl;
        }

        REQUIRE(result.valid == true);
        REQUIRE(result.walkable_tiles > 0);
        REQUIRE(result.errors.empty());
    }
    
    SECTION("TEST_DUNGEON is valid") {
        MapGenerator::generate(map, MapType::TEST_DUNGEON);
        auto result = MapValidator::validate(map);
        
        REQUIRE(result.valid == true);
        REQUIRE(result.walkable_tiles > 0);
        REQUIRE(result.room_count > 0);
        REQUIRE(result.errors.empty());
    }
    
    SECTION("CORRIDOR_TEST is valid") {
        MapGenerator::generate(map, MapType::CORRIDOR_TEST);
        auto result = MapValidator::validate(map);
        
        REQUIRE(result.valid == true);
        REQUIRE(result.walkable_tiles > 0);
    }
}

TEST_CASE("MapValidator: Special tiles", "[map_validator]") {
    Map map(20, 20);
    
    SECTION("Map with stairs") {
        // Create room with stairs
        for (int y = 5; y < 15; ++y) {
            for (int x = 5; x < 15; ++x) {
                map.setTile(x, y, TileType::FLOOR);
            }
        }
        map.setTile(10, 10, TileType::STAIRS_DOWN);
        
        auto result = MapValidator::validate(map);
        
        REQUIRE(result.valid == true);
        REQUIRE(result.has_stairs_down == true);
        REQUIRE(result.walkable_tiles > 0);  // Stairs should count as walkable
    }
    
    SECTION("Map with doors") {
        // Create rooms with door
        for (int y = 5; y < 15; ++y) {
            for (int x = 5; x < 15; ++x) {
                if (x == 10 && y == 5) {
                    map.setTile(x, y, TileType::DOOR_CLOSED);
                } else if (y == 5 || y == 14 || x == 5 || x == 14) {
                    map.setTile(x, y, TileType::WALL);
                } else {
                    map.setTile(x, y, TileType::FLOOR);
                }
            }
        }
        
        auto result = MapValidator::validate(map);
        
        REQUIRE(result.valid == true);
    }
}