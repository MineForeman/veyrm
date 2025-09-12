#include <catch2/catch_test_macros.hpp>
#include "fov.h"
#include "map.h"
#include "map_memory.h"
#include "map_generator.h"
#include <chrono>

TEST_CASE("FOV: Basic visibility calculation", "[fov]") {
    Map map(30, 30);
    
    SECTION("Empty room - all visible within radius") {
        map.fill(TileType::FLOOR);
        
        Point origin(15, 15);
        std::vector<std::vector<bool>> visible;
        FOV::calculate(map, origin, 5, visible);
        
        // Check center is visible
        REQUIRE(visible[15][15] == true);
        
        // Check points within radius
        REQUIRE(visible[15][10] == true);  // 5 tiles north
        REQUIRE(visible[15][20] == true);  // 5 tiles south
        REQUIRE(visible[10][15] == true);  // 5 tiles west
        REQUIRE(visible[20][15] == true);  // 5 tiles east
        
        // Check points outside radius
        REQUIRE(visible[15][9] == false);   // 6 tiles north
        REQUIRE(visible[15][21] == false);  // 6 tiles south
        REQUIRE(visible[9][15] == false);   // 6 tiles west
        REQUIRE(visible[21][15] == false);  // 6 tiles east
    }
    
    SECTION("Wall blocks vision") {
        map.fill(TileType::FLOOR);
        
        // Place wall at (15, 13)
        map.setTile(15, 13, TileType::WALL);
        
        Point origin(15, 15);
        std::vector<std::vector<bool>> visible;
        FOV::calculate(map, origin, 10, visible);
        
        // Wall itself is visible
        REQUIRE(visible[13][15] == true);
        
        // Point behind wall is not visible
        REQUIRE(visible[12][15] == false);
        REQUIRE(visible[11][15] == false);
    }
    
    SECTION("Vision around corners") {
        map.fill(TileType::FLOOR);
        
        // Create L-shaped wall
        map.setTile(14, 15, TileType::WALL);
        map.setTile(14, 14, TileType::WALL);
        map.setTile(14, 13, TileType::WALL);
        map.setTile(15, 13, TileType::WALL);
        map.setTile(16, 13, TileType::WALL);
        
        Point origin(15, 15);
        std::vector<std::vector<bool>> visible;
        FOV::calculate(map, origin, 10, visible);
        
        // Can't see around the corner
        REQUIRE(visible[12][13] == false);
        
        // Can see along unblocked paths
        REQUIRE(visible[17][15] == true);
        REQUIRE(visible[15][17] == true);
    }
}

TEST_CASE("FOV: Circular radius", "[fov]") {
    Map map(50, 50);
    map.fill(TileType::FLOOR);
    
    Point origin(25, 25);
    int radius = 10;
    std::vector<std::vector<bool>> visible;
    FOV::calculate(map, origin, radius, visible);
    
    SECTION("Points at exact radius") {
        // Check cardinal directions at radius
        REQUIRE(visible[15][25] == true);  // North
        REQUIRE(visible[35][25] == true);  // South
        REQUIRE(visible[25][15] == true);  // West
        REQUIRE(visible[25][35] == true);  // East
    }
    
    SECTION("Circular boundary") {
        // Points just outside circle radius
        // Distance > 10 in diagonal
        REQUIRE(visible[17][17] == false);  // NW corner beyond radius
        REQUIRE(visible[33][33] == false);  // SE corner beyond radius
    }
}

TEST_CASE("FOV: Symmetry", "[fov]") {
    Map map(40, 40);
    map.fill(TileType::FLOOR);
    
    SECTION("FOV is symmetric") {
        Point a(10, 10);
        Point b(20, 20);
        
        // Calculate FOV from A
        std::vector<std::vector<bool>> visibleFromA;
        FOV::calculate(map, a, 15, visibleFromA);
        
        // Calculate FOV from B
        std::vector<std::vector<bool>> visibleFromB;
        FOV::calculate(map, b, 15, visibleFromB);
        
        // If A can see B, then B should see A
        if (visibleFromA[b.y][b.x]) {
            REQUIRE(visibleFromB[a.y][a.x] == true);
        }
    }
}

TEST_CASE("FOV: Door visibility", "[fov]") {
    Map map(30, 30);
    map.fill(TileType::FLOOR);
    
    // Create room with door
    for (int x = 10; x <= 20; x++) {
        map.setTile(x, 10, TileType::WALL);
        map.setTile(x, 20, TileType::WALL);
    }
    for (int y = 10; y <= 20; y++) {
        map.setTile(10, y, TileType::WALL);
        map.setTile(20, y, TileType::WALL);
    }
    
    // Place door
    map.setTile(15, 10, TileType::DOOR_CLOSED);
    
    SECTION("Closed door blocks vision") {
        Point origin(15, 5);  // Outside room
        std::vector<std::vector<bool>> visible;
        FOV::calculate(map, origin, 10, visible);
        
        // Can see the door
        REQUIRE(visible[10][15] == true);
        
        // Can't see through closed door
        REQUIRE(visible[11][15] == false);
        REQUIRE(visible[15][15] == false);  // Center of room
    }
    
    SECTION("Open door allows vision") {
        map.setTile(15, 10, TileType::DOOR_OPEN);
        
        Point origin(15, 5);  // Outside room
        std::vector<std::vector<bool>> visible;
        FOV::calculate(map, origin, 15, visible);
        
        // Can see through open door
        REQUIRE(visible[11][15] == true);
        REQUIRE(visible[12][15] == true);
    }
}

TEST_CASE("FOV: Edge cases", "[fov]") {
    SECTION("FOV at map edge") {
        Map map(20, 20);
        map.fill(TileType::FLOOR);
        
        Point origin(0, 0);  // Corner
        std::vector<std::vector<bool>> visible;
        FOV::calculate(map, origin, 5, visible);
        
        // Should not crash
        REQUIRE(visible[0][0] == true);
        REQUIRE(visible[0][5] == true);
        REQUIRE(visible[5][0] == true);
    }
    
    SECTION("Zero radius") {
        Map map(10, 10);
        map.fill(TileType::FLOOR);
        
        Point origin(5, 5);
        std::vector<std::vector<bool>> visible;
        FOV::calculate(map, origin, 0, visible);
        
        // Only origin visible
        REQUIRE(visible[5][5] == true);
        REQUIRE(visible[4][5] == false);
        REQUIRE(visible[5][4] == false);
    }
    
    SECTION("Large radius") {
        Map map(100, 100);
        map.fill(TileType::FLOOR);
        
        Point origin(50, 50);
        std::vector<std::vector<bool>> visible;
        FOV::calculate(map, origin, 100, visible);
        
        // Should handle large radius without issues
        REQUIRE(visible[50][50] == true);
        REQUIRE(visible[0][50] == true);
        REQUIRE(visible[99][50] == true);
    }
}

TEST_CASE("FOV: Helper functions", "[fov]") {
    Map map(30, 30);
    map.fill(TileType::FLOOR);
    map.setTile(15, 13, TileType::WALL);
    
    SECTION("isVisible point check") {
        Point origin(15, 15);
        Point target1(15, 14);  // Visible
        Point target2(15, 12);  // Behind wall
        
        REQUIRE(FOV::isVisible(map, origin, target1, 10) == true);
        REQUIRE(FOV::isVisible(map, origin, target2, 10) == false);
    }
    
    SECTION("getVisibleTiles") {
        Point origin(15, 15);
        auto visibleTiles = FOV::getVisibleTiles(map, origin, 3);
        
        // Should contain origin
        REQUIRE(visibleTiles.count(origin) == 1);
        
        // Should have reasonable number of visible tiles
        REQUIRE(visibleTiles.size() > 10);
        REQUIRE(visibleTiles.size() < 50);
    }
}

TEST_CASE("MapMemory: Memory tracking", "[fov]") {
    Map map(20, 20);
    map.fill(TileType::FLOOR);
    map.setTile(10, 10, TileType::WALL);
    
    MapMemory memory(20, 20);
    
    SECTION("Initial state") {
        REQUIRE(memory.isExplored(10, 10) == false);
        REQUIRE(memory.isVisible(10, 10) == false);
        REQUIRE(memory.getVisibility(10, 10) == MapMemory::VisibilityState::UNKNOWN);
    }
    
    SECTION("After seeing tile") {
        std::vector<std::vector<bool>> fov(20, std::vector<bool>(20, false));
        fov[10][10] = true;
        
        memory.updateVisibility(map, fov);
        
        REQUIRE(memory.isExplored(10, 10) == true);
        REQUIRE(memory.isVisible(10, 10) == true);
        REQUIRE(memory.getRemembered(10, 10) == TileType::WALL);
        REQUIRE(memory.getVisibility(10, 10) == MapMemory::VisibilityState::VISIBLE);
    }
    
    SECTION("Memory persists when out of sight") {
        // First, see the tile
        std::vector<std::vector<bool>> fov1(20, std::vector<bool>(20, false));
        fov1[10][10] = true;
        memory.updateVisibility(map, fov1);
        
        // Then lose sight of it
        std::vector<std::vector<bool>> fov2(20, std::vector<bool>(20, false));
        memory.updateVisibility(map, fov2);
        
        REQUIRE(memory.isExplored(10, 10) == true);
        REQUIRE(memory.isVisible(10, 10) == false);
        REQUIRE(memory.getRemembered(10, 10) == TileType::WALL);
        REQUIRE(memory.getVisibility(10, 10) == MapMemory::VisibilityState::REMEMBERED);
    }
    
    SECTION("Forget all") {
        std::vector<std::vector<bool>> fov(20, std::vector<bool>(20, true));
        memory.updateVisibility(map, fov);
        
        REQUIRE(memory.isExplored(10, 10) == true);
        
        memory.forgetAll();
        
        REQUIRE(memory.isExplored(10, 10) == false);
        REQUIRE(memory.isVisible(10, 10) == false);
    }
}

TEST_CASE("FOV: Performance", "[fov][!benchmark]") {
    Map map(198, 66);  // Full Angband size
    MapGenerator::generateProceduralDungeon(map, 12345);
    
    SECTION("FOV calculation speed") {
        Point origin(99, 33);
        std::vector<std::vector<bool>> visible;
        
        // Should complete quickly even on large map
        auto start = std::chrono::high_resolution_clock::now();
        FOV::calculate(map, origin, 10, visible);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        REQUIRE(duration.count() < 10);  // Should take less than 10ms
    }
}