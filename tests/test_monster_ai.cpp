#include <catch2/catch_test_macros.hpp>
#include "monster_ai.h"
#include "pathfinding.h"
#include "monster.h"
#include "player.h"
#include "map.h"
#include "room.h"

class TestMonster : public Monster {
public:
    TestMonster(int x, int y) : Monster(x, y, "test") {
        setStats(10, 10, 5, 2, 100, 5);
        setMetadata("Test Monster", "A test monster", "T", ftxui::Color::Red, 'a');
        setFlags(true, false, false);
    }
};

class TestPlayer : public Player {
public:
    TestPlayer(int x, int y) : Player(x, y) {
        hp = 20;
        max_hp = 20;
    }
};

// Helper to create a simple test map
std::unique_ptr<Map> createTestMap() {
    auto map = std::make_unique<Map>(20, 20);

    // Fill with walls
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 20; ++x) {
            map->setTile(x, y, TileType::WALL);
        }
    }

    // Create a simple room
    for (int y = 5; y < 10; ++y) {
        for (int x = 5; x < 10; ++x) {
            map->setTile(x, y, TileType::FLOOR);
        }
    }

    // Create a corridor
    for (int x = 10; x < 15; ++x) {
        map->setTile(x, 7, TileType::FLOOR);
    }

    return map;
}

TEST_CASE("MonsterAI Basic Functionality", "[monster_ai]") {
    MonsterAI ai;
    auto map = createTestMap();
    TestMonster monster(7, 7);
    TestPlayer player(12, 7);

    SECTION("AI data initialization") {
        ai.updateMonsterAI(monster, player, *map);
        REQUIRE(monster.hasAIData() == true);
    }

    SECTION("Can see player in line of sight") {
        bool can_see = ai.canSeePlayer(monster, player, *map);
        REQUIRE(can_see == true);
    }

    SECTION("Cannot see player through walls") {
        TestPlayer hidden_player(17, 7);
        bool can_see = ai.canSeePlayer(monster, hidden_player, *map);
        REQUIRE(can_see == false);
    }
}

TEST_CASE("Pathfinding System", "[pathfinding]") {
    auto map = createTestMap();

    SECTION("Find path in open area") {
        Point start(7, 7);
        Point goal(12, 7);

        auto path = Pathfinding::findPath(start, goal, *map, true);
        REQUIRE(!path.empty());
        REQUIRE(path.back() == goal);
    }

    SECTION("Line of sight detection") {
        Point start(7, 7);
        Point visible_goal(12, 7);
        Point hidden_goal(17, 7);

        REQUIRE(Pathfinding::hasLineOfSight(start, visible_goal, *map) == true);
        REQUIRE(Pathfinding::hasLineOfSight(start, hidden_goal, *map) == false);
    }

    SECTION("8-direction movement") {
        Point start(7, 7);

        // Test all 8 directions
        for (int i = 0; i < 8; ++i) {
            Point direction = Pathfinding::DIRECTIONS_8[i];
            Point goal = start + direction;

            auto path = Pathfinding::findPath(start, goal, *map, true);
            REQUIRE(!path.empty());
        }
    }
}

TEST_CASE("AI State Transitions", "[monster_ai]") {
    MonsterAI ai;
    auto map = createTestMap();
    TestMonster monster(7, 7);

    SECTION("IDLE to HOSTILE when player is close") {
        TestPlayer close_player(8, 8);
        ai.updateMonsterAI(monster, close_player, *map);

        Point next_move = ai.getNextMove(monster, close_player, *map);
        REQUIRE(next_move != monster.getPosition());
    }

    SECTION("FLEEING when monster is low health") {
        TestPlayer player(8, 8);
        monster.hp = 2; // Low health

        ai.updateMonsterAI(monster, player, *map);
        Point next_move = ai.getNextMove(monster, player, *map);

        // Should move away from player
        float initial_dist = Pathfinding::getDistance(monster.getPosition(), player.getPosition());
        float new_dist = Pathfinding::getDistance(next_move, player.getPosition());
        REQUIRE(new_dist >= initial_dist);
    }
}

TEST_CASE("Room Assignment", "[monster_ai]") {
    MonsterAI ai;
    auto map = createTestMap();
    TestMonster monster(7, 7);

    Room test_room(5, 5, 5, 5);

    SECTION("Monster can be assigned to room") {
        ai.assignRoomToMonster(monster, &test_room);
        REQUIRE(monster.hasAIData() == true);
    }

    SECTION("Monster wanders within room when idle") {
        TestPlayer distant_player(17, 17);
        ai.assignRoomToMonster(monster, &test_room);

        for (int i = 0; i < 10; ++i) {
            ai.updateMonsterAI(monster, distant_player, *map);
            Point next_move = ai.getNextMove(monster, distant_player, *map);

            // Should stay within room bounds
            REQUIRE(next_move.x >= test_room.x);
            REQUIRE(next_move.x < test_room.x + test_room.width);
            REQUIRE(next_move.y >= test_room.y);
            REQUIRE(next_move.y < test_room.y + test_room.height);
        }
    }
}

TEST_CASE("Combat Behavior", "[monster_ai]") {
    MonsterAI ai;
    auto map = createTestMap();
    TestMonster monster(7, 7);

    SECTION("Monster moves toward player when hostile") {
        TestPlayer player(12, 7);
        ai.updateMonsterAI(monster, player, *map);

        float initial_dist = Pathfinding::getDistance(monster.getPosition(), player.getPosition());

        Point next_move = ai.getNextMove(monster, player, *map);
        float new_dist = Pathfinding::getDistance(next_move, player.getPosition());

        REQUIRE(new_dist < initial_dist);
    }

    SECTION("Monster chases player out of room") {
        Room test_room(5, 5, 5, 5);
        TestPlayer player(12, 7); // Outside room

        ai.assignRoomToMonster(monster, &test_room);
        ai.updateMonsterAI(monster, player, *map);

        Point next_move = ai.getNextMove(monster, player, *map);

        // Should move toward player even outside room
        float initial_dist = Pathfinding::getDistance(monster.getPosition(), player.getPosition());
        float new_dist = Pathfinding::getDistance(next_move, player.getPosition());

        REQUIRE(new_dist < initial_dist);
    }
}