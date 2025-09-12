#include <catch2/catch_test_macros.hpp>
#include "map_generator.h"
#include "map.h"
#include "point.h"
#include <set>

TEST_CASE("MapGenerator: Test room generation", "[map_generator]") {
    Map map;  // Use default size 80x24
    
    SECTION("Generate test room") {
        MapGenerator::generate(map, MapType::TEST_ROOM);
        
        // Should have at least some floor tiles
        int floor_count = 0;
        for (int y = 0; y < map.getHeight(); ++y) {
            for (int x = 0; x < map.getWidth(); ++x) {
                if (map.getTile(x, y) == TileType::FLOOR) {
                    floor_count++;
                }
            }
        }
        
        REQUIRE(floor_count > 0);
        
        // Should have walls around the room
        int wall_count = 0;
        for (int y = 0; y < map.getHeight(); ++y) {
            for (int x = 0; x < map.getWidth(); ++x) {
                if (map.getTile(x, y) == TileType::WALL) {
                    wall_count++;
                }
            }
        }
        
        REQUIRE(wall_count > 0);
    }
    
    SECTION("Spawn point is valid") {
        MapGenerator::generate(map, MapType::TEST_ROOM);
        Point spawn = MapGenerator::getDefaultSpawnPoint(MapType::TEST_ROOM);
        
        // Spawn point should be on a floor tile
        REQUIRE(Map::getTileProperties(map.getTile(spawn.x, spawn.y)).walkable == true);
    }
}

TEST_CASE("MapGenerator: Test dungeon generation", "[map_generator]") {
    Map map;  // Use default size
    
    SECTION("Generate test dungeon") {
        MapGenerator::generate(map, MapType::TEST_DUNGEON);
        
        // Should have multiple rooms (check for disconnected floor regions)
        int floor_count = 0;
        for (int y = 0; y < map.getHeight(); ++y) {
            for (int x = 0; x < map.getWidth(); ++x) {
                if (map.getTile(x, y) == TileType::FLOOR) {
                    floor_count++;
                }
            }
        }
        
        // Dungeon should have substantial floor space
        REQUIRE(floor_count > 100);
    }
    
    SECTION("Has stairs") {
        MapGenerator::generate(map, MapType::TEST_DUNGEON);
        
        // Should have at least one stairs tile
        bool has_stairs = false;
        for (int y = 0; y < map.getHeight(); ++y) {
            for (int x = 0; x < map.getWidth(); ++x) {
                if (map.getTile(x, y) == TileType::STAIRS_DOWN ||
                    map.getTile(x, y) == TileType::STAIRS_UP) {
                    has_stairs = true;
                    break;
                }
            }
            if (has_stairs) break;
        }
        
        REQUIRE(has_stairs == true);
    }
}

TEST_CASE("MapGenerator: Corridor test", "[map_generator]") {
    Map map;  // Use default size
    
    SECTION("Generate corridor test map") {
        MapGenerator::generate(map, MapType::CORRIDOR_TEST);
        
        // Should have corridors (narrow passages)
        int floor_count = 0;
        for (int y = 0; y < map.getHeight(); ++y) {
            for (int x = 0; x < map.getWidth(); ++x) {
                if (map.getTile(x, y) == TileType::FLOOR) {
                    floor_count++;
                }
            }
        }
        
        // Corridors should exist
        REQUIRE(floor_count > 0);
        
        // Check that corridors have walls
        bool has_corridor_walls = false;
        for (int y = 1; y < map.getHeight() - 1; ++y) {
            for (int x = 1; x < map.getWidth() - 1; ++x) {
                if (map.getTile(x, y) == TileType::FLOOR) {
                    // Check if this floor tile has walls nearby
                    if (map.getTile(x-1, y) == TileType::WALL ||
                        map.getTile(x+1, y) == TileType::WALL ||
                        map.getTile(x, y-1) == TileType::WALL ||
                        map.getTile(x, y+1) == TileType::WALL) {
                        has_corridor_walls = true;
                        break;
                    }
                }
            }
            if (has_corridor_walls) break;
        }
        
        REQUIRE(has_corridor_walls == true);
    }
}

TEST_CASE("MapGenerator: Safe spawn point", "[map_generator]") {
    Map map;  // Use default size
    
    SECTION("Find safe spawn in empty map") {
        // Fill map with walls
        for (int y = 0; y < map.getHeight(); ++y) {
            for (int x = 0; x < map.getWidth(); ++x) {
                map.setTile(x, y, TileType::WALL);
            }
        }
        
        // Add a small room
        for (int y = 10; y < 15; ++y) {
            for (int x = 10; x < 15; ++x) {
                map.setTile(x, y, TileType::FLOOR);
            }
        }
        
        Point spawn = MapGenerator::findSafeSpawnPoint(map);
        
        // Spawn should be on a floor tile
        REQUIRE(map.getTile(spawn.x, spawn.y) == TileType::FLOOR);
        REQUIRE(spawn.x >= 10);
        REQUIRE(spawn.x < 15);
        REQUIRE(spawn.y >= 10);
        REQUIRE(spawn.y < 15);
    }
    
    SECTION("Find safe spawn in complex map") {
        MapGenerator::generate(map, MapType::TEST_DUNGEON);
        Point spawn = MapGenerator::findSafeSpawnPoint(map);
        
        // Spawn should be walkable
        REQUIRE(Map::getTileProperties(map.getTile(spawn.x, spawn.y)).walkable == true);
    }
}

TEST_CASE("MapGenerator: All map types generate", "[map_generator]") {
    Map map;  // Use default size
    
    SECTION("TEST_ROOM generates without crash") {
        REQUIRE_NOTHROW(MapGenerator::generate(map, MapType::TEST_ROOM));
    }
    
    SECTION("TEST_DUNGEON generates without crash") {
        REQUIRE_NOTHROW(MapGenerator::generate(map, MapType::TEST_DUNGEON));
    }
    
    SECTION("CORRIDOR_TEST generates without crash") {
        REQUIRE_NOTHROW(MapGenerator::generate(map, MapType::CORRIDOR_TEST));
    }
    
    SECTION("COMBAT_ARENA generates without crash") {
        REQUIRE_NOTHROW(MapGenerator::generate(map, MapType::COMBAT_ARENA));
    }
    
    SECTION("STRESS_TEST generates without crash") {
        REQUIRE_NOTHROW(MapGenerator::generate(map, MapType::STRESS_TEST));
    }
}

TEST_CASE("MapGenerator: Map type spawn points", "[map_generator]") {
    SECTION("Each map type has valid default spawn") {
        Map map;  // Use default size
        
        std::vector<MapType> types = {
            MapType::TEST_ROOM,
            MapType::TEST_DUNGEON,
            MapType::CORRIDOR_TEST,
            MapType::COMBAT_ARENA,
            MapType::STRESS_TEST
        };
        
        for (auto type : types) {
            MapGenerator::generate(map, type);
            Point spawn = MapGenerator::getDefaultSpawnPoint(map, type);
            
            // Spawn should be within bounds
            REQUIRE(spawn.x >= 0);
            REQUIRE(spawn.x < map.getWidth());
            REQUIRE(spawn.y >= 0);
            REQUIRE(spawn.y < map.getHeight());
            
            // Spawn should be walkable
            auto tile_props = Map::getTileProperties(map.getTile(spawn.x, spawn.y));
            REQUIRE(tile_props.walkable == true);
        }
    }
}