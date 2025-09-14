#include <catch2/catch_test_macros.hpp>
#include "spawn_manager.h"
#include "map.h"
#include "entity_manager.h"
#include "player.h"
#include "monster.h"
#include "monster_factory.h"
#include "config.h"

TEST_CASE("SpawnManager: Basic functionality", "[spawn]") {
    // Ensure monsters are loaded
    MonsterFactory::getInstance().loadFromFile("data/monsters.json");
    
    Map map(80, 40);
    EntityManager entity_manager;
    
    // Create a simple room for testing
    map.fill(TileType::VOID);
    for (int y = 10; y < 20; y++) {
        for (int x = 10; x < 30; x++) {
            map.setTile(x, y, TileType::FLOOR);
        }
    }
    
    // Place player
    auto player = entity_manager.createPlayer(15, 15);
    
    SECTION("Valid spawn point detection") {
        SpawnManager spawn_manager;
        auto spawn_points = spawn_manager.getValidSpawnPoints(map, player.get());
        
        // Should have spawn points away from player
        REQUIRE(spawn_points.size() > 0);
        
        // All spawn points should be walkable
        for (const auto& point : spawn_points) {
            REQUIRE(map.isWalkable(point.x, point.y));
        }
        
        // All spawn points should be at least min_distance from player
        Config& config = Config::getInstance();
        int min_dist = config.getMinSpawnDistance();
        for (const auto& point : spawn_points) {
            int dx = point.x - player->x;
            int dy = point.y - player->y;
            float dist = static_cast<float>(std::sqrt(dx*dx + dy*dy));
            REQUIRE(dist >= min_dist);
        }
    }
    
    SECTION("Monster species selection by depth") {
        SpawnManager spawn_manager;
        std::mt19937 rng(12345);
        
        // Depth 1 should only have early monsters
        std::string species = spawn_manager.selectMonsterSpecies(1, rng);
        REQUIRE((species == "gutter_rat" || species == "cave_spider" || species == "kobold"));
        
        // Depth 10 should have more variety
        bool found_different = false;
        for (int i = 0; i < 10; i++) {
            std::string s = spawn_manager.selectMonsterSpecies(10, rng);
            if (s == "zombie") {
                found_different = true;
                break;
            }
        }
        REQUIRE(found_different == true);
    }
    
    SECTION("Initial monster spawning") {
        SpawnManager spawn_manager;
        
        // Clear any existing monsters
        auto existing = entity_manager.getMonsters();
        REQUIRE(existing.size() == 0);
        
        // Spawn initial monsters
        spawn_manager.spawnInitialMonsters(map, entity_manager, player.get(), 1);
        
        // Should have spawned some monsters
        auto monsters = entity_manager.getMonsters();
        REQUIRE(monsters.size() > 0);
        REQUIRE(monsters.size() <= static_cast<size_t>(Config::getInstance().getInitialMonsterCount()));
        
        // All monsters should be at valid positions
        for (const auto& monster : monsters) {
            REQUIRE(map.isWalkable(monster->x, monster->y));
            
            // Check distance from player
            int dx = monster->x - player->x;
            int dy = monster->y - player->y;
            float dist = static_cast<float>(std::sqrt(dx*dx + dy*dy));
            REQUIRE(dist >= Config::getInstance().getMinSpawnDistance());
        }
    }
}

TEST_CASE("SpawnManager: Dynamic spawning", "[spawn]") {
    Map map(80, 40);
    EntityManager entity_manager;
    
    // Create room
    map.fill(TileType::VOID);
    for (int y = 5; y < 35; y++) {
        for (int x = 5; x < 75; x++) {
            map.setTile(x, y, TileType::FLOOR);
        }
    }
    
    auto player = entity_manager.createPlayer(40, 20);
    SpawnManager spawn_manager;
    spawn_manager.setSpawnRate(10);  // Fast spawn rate for testing
    
    SECTION("Spawn timer") {
        int initial_count = static_cast<int>(entity_manager.getMonsters().size());
        
        // Update multiple times
        for (int i = 0; i < 15; i++) {
            spawn_manager.update(map, entity_manager, player.get(), 1);
        }
        
        // Should have spawned at least one monster
        int new_count = static_cast<int>(entity_manager.getMonsters().size());
        REQUIRE(new_count > initial_count);
    }
    
    SECTION("Respects max monster limit") {
        spawn_manager.setMaxMonsters(5);
        spawn_manager.setSpawnRate(1);  // Spawn every turn
        
        // Spawn many times
        for (int i = 0; i < 20; i++) {
            spawn_manager.update(map, entity_manager, player.get(), 1);
        }
        
        // Should not exceed max
        REQUIRE(entity_manager.getMonsters().size() <= 5);
    }
}

TEST_CASE("SpawnManager: Room preference spawning", "[spawn]") {
    // Ensure monsters are loaded
    MonsterFactory::getInstance().loadFromFile("data/monsters.json");
    
    Map map(100, 50);
    EntityManager entity_manager;
    
    // Create a map with distinct rooms and corridors
    map.fill(TileType::WALL);
    
    // Create two rooms
    Room room1(10, 10, 10, 10);
    Room room2(30, 10, 10, 10);
    
    // Add floors to rooms
    for (int y = 10; y < 20; y++) {
        for (int x = 10; x < 20; x++) {
            map.setTile(x, y, TileType::FLOOR);
        }
    }
    for (int y = 10; y < 20; y++) {
        for (int x = 30; x < 40; x++) {
            map.setTile(x, y, TileType::FLOOR);
        }
    }
    
    // Create a corridor connecting them
    for (int x = 20; x < 30; x++) {
        map.setTile(x, 15, TileType::FLOOR);
    }
    
    // Add rooms to map
    map.addRoom(room1);
    map.addRoom(room2);
    
    // Place player far away
    auto player = entity_manager.createPlayer(50, 25);
    
    SpawnManager spawn_manager;
    spawn_manager.setInitialMonsterCount(20); // Spawn more monsters for better statistics
    
    // Get spawn points by type
    auto room_points = spawn_manager.getRoomSpawnPoints(map, player.get());
    auto corridor_points = spawn_manager.getCorridorSpawnPoints(map, player.get());
    
    // Should have both room and corridor spawn points
    REQUIRE(room_points.size() > 0);
    REQUIRE(corridor_points.size() > 0);
    
    // Room points should be in rooms
    for (const auto& point : room_points) {
        REQUIRE(map.getRoomAt(point) != nullptr);
    }
    
    // Corridor points should NOT be in rooms
    for (const auto& point : corridor_points) {
        REQUIRE(map.getRoomAt(point) == nullptr);
    }
    
    // Spawn initial monsters
    spawn_manager.spawnInitialMonsters(map, entity_manager, player.get(), 1);
    
    // Count monsters in rooms vs corridors
    auto monsters = entity_manager.getMonsters();
    int monsters_in_rooms = 0;
    
    for (const auto& monster : monsters) {
        if (map.getRoomAt(Point(monster->x, monster->y))) {
            monsters_in_rooms++;
        }
    }
    
    // Most monsters should be in rooms (95% by default)
    if (monsters.size() > 0) {
        float room_percentage = static_cast<float>(monsters_in_rooms) / monsters.size();
        // Allow some variance from the configured 95%
        REQUIRE(room_percentage >= 0.75f);
        
        // Also verify that not ALL monsters are in rooms (some should be in corridors)
        REQUIRE(monsters_in_rooms < static_cast<int>(monsters.size()));
    }
}

TEST_CASE("SpawnManager: Spawn point validation", "[spawn]") {
    Map map(30, 30);
    Player player(15, 15);
    SpawnManager spawn_manager;
    
    SECTION("Rejects non-walkable tiles") {
        map.fill(TileType::WALL);
        REQUIRE(spawn_manager.isValidSpawnPoint(map, &player, 10, 10) == false);
    }
    
    SECTION("Rejects tiles too close to player") {
        map.fill(TileType::FLOOR);
        
        // Right next to player
        REQUIRE(spawn_manager.isValidSpawnPoint(map, &player, 15, 16) == false);
        REQUIRE(spawn_manager.isValidSpawnPoint(map, &player, 16, 15) == false);
        
        // Far from player
        REQUIRE(spawn_manager.isValidSpawnPoint(map, &player, 5, 5) == true);
        REQUIRE(spawn_manager.isValidSpawnPoint(map, &player, 25, 25) == true);
    }
    
    SECTION("Rejects special tiles") {
        map.fill(TileType::FLOOR);
        map.setTile(10, 10, TileType::STAIRS_DOWN);
        map.setTile(20, 20, TileType::STAIRS_UP);
        
        REQUIRE(spawn_manager.isValidSpawnPoint(map, &player, 10, 10) == false);
        REQUIRE(spawn_manager.isValidSpawnPoint(map, &player, 20, 20) == false);
    }
}

TEST_CASE("SpawnManager: Threat level tracking", "[spawn]") {
    EntityManager entity_manager;
    SpawnManager spawn_manager;
    
    SECTION("Calculate total threat") {
        // Spawn specific monsters with known threat values
        entity_manager.createMonster("gutter_rat", 10, 10);    // threat: 1
        entity_manager.createMonster("cave_spider", 20, 20);   // threat: 2
        entity_manager.createMonster("orc_rookling", 30, 30);  // threat: 3
        
        int total_threat = spawn_manager.getCurrentThreatLevel(entity_manager);
        REQUIRE(total_threat == 6);  // 1 + 2 + 3
    }
}