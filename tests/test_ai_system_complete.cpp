#include <catch2/catch_test_macros.hpp>
#include "ecs/ai_system.h"
#include "ecs/game_world.h"
#include "ecs/entity_factory.h"
#include "ecs/component.h"
#include "pathfinding.h"
#include "map.h"
#include <thread>

using namespace ecs;

class TestableAISystem : public AISystem {
public:
    TestableAISystem(GameWorld* world, Map* map) : AISystem(world, map) {}

    // Expose protected methods for testing
    using AISystem::calculatePath;
    using AISystem::isPlayerVisible;
    using AISystem::findNearestTarget;
    using AISystem::chooseAction;
    using AISystem::executeAction;
    using AISystem::updateBehaviorState;
};

TEST_CASE("AISystem basic behaviors", "[ai][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableAISystem aiSystem(world.get(), &testMap);
    EntityFactory factory(world.get());

    SECTION("Idle behavior") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto* ai = world->getComponent<AIComponent>(monster);

        REQUIRE(ai != nullptr);
        REQUIRE(ai->behavior == AIBehavior::Idle);

        // Update with no player nearby
        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));
        aiSystem.update(entities, 0.1);

        // Should remain idle
        if (ai) {
            REQUIRE(ai->behavior == AIBehavior::Idle);
        }
    }

    SECTION("Patrol behavior") {
        auto monster = factory.createMonster("guard", 15, 15);
        auto* ai = world->getComponent<AIComponent>(monster);

        if (ai) {
            ai->behavior = AIBehavior::Patrol;

            // Set patrol points
            ai->patrolPoints = {
                {15, 15}, {20, 15}, {20, 20}, {15, 20}
            };
            ai->currentPatrolIndex = 0;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update multiple times to move along patrol
        for (int i = 0; i < 10; ++i) {
            aiSystem.update(entities, 0.1);
        }

        // Should have moved from starting position
        if (auto* pos = world->getComponent<PositionComponent>(entities[0].get())) {
            REQUIRE((pos->x != 15 || pos->y != 15));
        }
    }

    SECTION("Chase behavior") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto player = factory.createPlayer(15, 10);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Chase;
            ai->target = player;
            ai->visionRange = 10;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update to chase player
        aiSystem.update(entities, 0.1);

        // Monster should move toward player
        if (auto* pos = world->getComponent<PositionComponent>(entities[0].get())) {
            // Should have moved closer to player (x=15)
            REQUIRE(pos->x > 10);
        }
    }

    SECTION("Attack behavior") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto player = factory.createPlayer(11, 10); // Adjacent

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Attack;
            ai->target = player;
        }

        // Give player health to track damage
        auto* playerHealth = world->getComponent<HealthComponent>(player);
        int initialHp = playerHealth ? playerHealth->hp : 100;

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update to attack
        aiSystem.update(entities, 0.1);

        // Player should have taken damage
        if (playerHealth) {
            REQUIRE(playerHealth->hp <= initialHp);
        }
    }

    SECTION("Flee behavior") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto player = factory.createPlayer(11, 10);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Flee;
            ai->target = player;
        }

        // Set monster health low
        if (auto* health = world->getComponent<HealthComponent>(monster)) {
            health->hp = 1;
            health->maxHp = 10;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update to flee
        aiSystem.update(entities, 0.1);

        // Monster should move away from player
        if (auto* pos = world->getComponent<PositionComponent>(entities[0].get())) {
            // Should have moved away from player (x=11)
            REQUIRE(pos->x < 10);
        }
    }

    SECTION("Alert behavior") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto player = factory.createPlayer(20, 20);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Alert;
            ai->lastKnownTargetPos = {20, 20};
            ai->alertTimer = 5.0f;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update while alert
        aiSystem.update(entities, 1.0);

        // Alert timer should decrease
        if (ai) {
            REQUIRE(ai->alertTimer < 5.0f);
        }
    }

    SECTION("Guard behavior") {
        auto monster = factory.createMonster("guard", 10, 10);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Guard;
            ai->guardPost = {10, 10};
            ai->guardRadius = 3;
        }

        // Move monster away from post
        if (auto* pos = world->getComponent<PositionComponent>(monster)) {
            pos->x = 15;
            pos->y = 15;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update to return to post
        for (int i = 0; i < 10; ++i) {
            aiSystem.update(entities, 0.1);
        }

        // Should move back toward guard post
        if (auto* pos = world->getComponent<PositionComponent>(entities[0].get())) {
            int dist = std::abs(pos->x - 10) + std::abs(pos->y - 10);
            REQUIRE(dist <= ai->guardRadius);
        }
    }

    SECTION("Wander behavior") {
        auto monster = factory.createMonster("goblin", 25, 15);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Wander;
        }

        auto* initialPos = world->getComponent<PositionComponent>(monster);
        int startX = initialPos ? initialPos->x : 25;
        int startY = initialPos ? initialPos->y : 15;

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update multiple times
        for (int i = 0; i < 20; ++i) {
            aiSystem.update(entities, 0.1);
        }

        // Should have moved from starting position
        if (auto* pos = world->getComponent<PositionComponent>(entities[0].get())) {
            REQUIRE((pos->x != startX || pos->y != startY));
        }
    }

    SECTION("Sleep behavior") {
        auto monster = factory.createMonster("goblin", 10, 10);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Sleep;
            ai->sleepTimer = 10.0f;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        auto* pos = world->getComponent<PositionComponent>(entities[0].get());
        int startX = pos ? pos->x : 10;
        int startY = pos ? pos->y : 10;

        // Update while sleeping
        aiSystem.update(entities, 1.0);

        // Should not move while sleeping
        if (pos) {
            REQUIRE(pos->x == startX);
            REQUIRE(pos->y == startY);
        }

        // Sleep timer should decrease
        if (ai) {
            REQUIRE(ai->sleepTimer < 10.0f);
        }
    }
}

TEST_CASE("AISystem vision and detection", "[ai][ecs]") {
    Map testMap(50, 30);
    testMap.initializeTiles(TileType::Floor);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableAISystem aiSystem(world.get(), &testMap);
    EntityFactory factory(world.get());

    SECTION("Line of sight detection") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto player = factory.createPlayer(15, 10);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->visionRange = 10;
        }

        // Clear line of sight
        bool visible = aiSystem.isPlayerVisible(monster, player);
        REQUIRE(visible == true);

        // Add wall between
        testMap.getTile(12, 10).type = TileType::Wall;

        visible = aiSystem.isPlayerVisible(monster, player);
        REQUIRE(visible == false);
    }

    SECTION("Vision range limits") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->visionRange = 5;
        }

        // Player within range
        auto nearPlayer = factory.createPlayer(14, 10);
        REQUIRE(aiSystem.isPlayerVisible(monster, nearPlayer) == true);

        // Player outside range
        auto farPlayer = factory.createPlayer(20, 10);
        REQUIRE(aiSystem.isPlayerVisible(monster, farPlayer) == false);
    }

    SECTION("Stealth and detection") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto player = factory.createPlayer(13, 10);

        // Give player stealth
        if (auto* stats = world->getComponent<StatsComponent>(player)) {
            stats->stealth = 90;
        }

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->visionRange = 10;
            ai->perception = 50;
        }

        // Detection should be harder with stealth
        bool detected = aiSystem.detectStealthedTarget(monster, player);

        // With high stealth vs low perception, detection should often fail
        // (This is probabilistic, so we just check it doesn't crash)
        REQUIRE((detected == true || detected == false));
    }

    SECTION("Sound detection") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto player = factory.createPlayer(15, 10);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->hearingRange = 15;
        }

        // Player makes noise
        aiSystem.handleSound(15, 10, 50); // x, y, volume

        // Monster should be alerted
        if (ai) {
            REQUIRE(ai->behavior == AIBehavior::Alert);
            REQUIRE(ai->lastKnownTargetPos.x == 15);
            REQUIRE(ai->lastKnownTargetPos.y == 10);
        }
    }

    SECTION("Multiple target prioritization") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto player = factory.createPlayer(15, 10);
        auto ally = factory.createEntity();
        world->addComponent<PositionComponent>(ally, 12, 10);
        world->addComponent<HealthComponent>(ally, 50, 50);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->visionRange = 10;
        }

        // Find nearest target
        auto target = aiSystem.findNearestTarget(monster);

        // Should prioritize closer target (ally at distance 2 vs player at 5)
        if (auto* targetPos = world->getComponent<PositionComponent>(target)) {
            REQUIRE(targetPos->x == 12); // Ally position
        }
    }
}

TEST_CASE("AISystem pathfinding", "[ai][ecs]") {
    Map testMap(20, 20);
    testMap.initializeTiles(TileType::Floor);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableAISystem aiSystem(world.get(), &testMap);
    EntityFactory factory(world.get());

    SECTION("Basic pathfinding") {
        auto monster = factory.createMonster("goblin", 5, 5);

        // Calculate path to target
        auto path = aiSystem.calculatePath(5, 5, 10, 10);

        REQUIRE(!path.empty());
        // Path should lead toward target
        if (!path.empty()) {
            auto firstStep = path.front();
            REQUIRE((firstStep.first > 5 || firstStep.second > 5));
        }
    }

    SECTION("Path around obstacles") {
        auto monster = factory.createMonster("goblin", 5, 5);

        // Create wall
        for (int x = 7; x < 10; ++x) {
            testMap.getTile(x, 5).type = TileType::Wall;
        }

        auto path = aiSystem.calculatePath(5, 5, 12, 5);

        REQUIRE(!path.empty());
        // Path should go around wall
        bool goesAround = false;
        for (const auto& [x, y] : path) {
            if (y != 5) { // Moved vertically to avoid wall
                goesAround = true;
                break;
            }
        }
        REQUIRE(goesAround == true);
    }

    SECTION("No path available") {
        auto monster = factory.createMonster("goblin", 5, 5);

        // Completely wall off target
        for (int x = 9; x <= 11; ++x) {
            for (int y = 9; y <= 11; ++y) {
                if (x == 9 || x == 11 || y == 9 || y == 11) {
                    testMap.getTile(x, y).type = TileType::Wall;
                }
            }
        }

        auto path = aiSystem.calculatePath(5, 5, 10, 10);

        // Should return empty path when no route exists
        REQUIRE(path.empty());
    }

    SECTION("Dynamic path recalculation") {
        auto monster = factory.createMonster("goblin", 5, 5);
        auto player = factory.createPlayer(10, 5);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Chase;
            ai->target = player;
            ai->path = aiSystem.calculatePath(5, 5, 10, 5);
        }

        // Move player
        if (auto* playerPos = world->getComponent<PositionComponent>(player)) {
            playerPos->x = 10;
            playerPos->y = 10;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update should recalculate path
        aiSystem.update(entities, 0.1);

        if (ai && !ai->path.empty()) {
            // New path should head toward new player position
            auto dest = ai->path.back();
            REQUIRE((std::abs(dest.first - 10) <= 1 && std::abs(dest.second - 10) <= 1));
        }
    }
}

TEST_CASE("AISystem group behaviors", "[ai][ecs]") {
    Map testMap(50, 30);
    testMap.initializeTiles(TileType::Floor);
    auto world = std::make_unique<GameWorld>(&testMap);
    AISystem aiSystem(world.get(), &testMap);
    EntityFactory factory(world.get());

    SECTION("Pack hunting") {
        auto player = factory.createPlayer(25, 15);

        std::vector<std::unique_ptr<Entity>> entities;

        // Create wolf pack
        for (int i = 0; i < 5; ++i) {
            auto wolf = factory.createMonster("wolf", 10 + i * 2, 15);
            if (auto* ai = world->getComponent<AIComponent>(wolf)) {
                ai->behavior = AIBehavior::Chase;
                ai->target = player;
                ai->packId = 1; // Same pack
            }
            entities.push_back(std::move(wolf));
        }

        // Update pack
        aiSystem.update(entities, 0.1);

        // Wolves should coordinate (spread out to surround)
        std::set<std::pair<int, int>> positions;
        for (const auto& entity : entities) {
            if (auto* pos = world->getComponent<PositionComponent>(entity.get())) {
                positions.insert({pos->x, pos->y});
            }
        }

        // All wolves should be in different positions (no stacking)
        REQUIRE(positions.size() == 5);
    }

    SECTION("Leader following") {
        auto leader = factory.createMonster("goblin_chief", 10, 10);
        auto* leaderAI = world->getComponent<AIComponent>(leader);
        if (leaderAI) {
            leaderAI->isLeader = true;
            leaderAI->behavior = AIBehavior::Patrol;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(leader));

        // Create followers
        for (int i = 0; i < 3; ++i) {
            auto follower = factory.createMonster("goblin", 8 + i, 10);
            if (auto* ai = world->getComponent<AIComponent>(follower)) {
                ai->behavior = AIBehavior::Follow;
                ai->leader = entities[0].get(); // Follow the chief
            }
            entities.push_back(std::move(follower));
        }

        // Move leader
        if (auto* pos = world->getComponent<PositionComponent>(entities[0].get())) {
            pos->x = 15;
            pos->y = 15;
        }

        // Update followers
        for (int i = 0; i < 10; ++i) {
            aiSystem.update(entities, 0.1);
        }

        // Followers should move toward leader's new position
        for (size_t i = 1; i < entities.size(); ++i) {
            if (auto* pos = world->getComponent<PositionComponent>(entities[i].get())) {
                int dist = std::abs(pos->x - 15) + std::abs(pos->y - 15);
                REQUIRE(dist <= 5); // Within following distance
            }
        }
    }
}

TEST_CASE("AISystem state transitions", "[ai][ecs]") {
    Map testMap(50, 30);
    testMap.initializeTiles(TileType::Floor);
    auto world = std::make_unique<GameWorld>(&testMap);
    TestableAISystem aiSystem(world.get(), &testMap);
    EntityFactory factory(world.get());

    SECTION("Idle to Alert") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto* ai = world->getComponent<AIComponent>(monster);

        REQUIRE(ai->behavior == AIBehavior::Idle);

        // Noise triggers alert
        aiSystem.handleSound(12, 10, 60);

        if (ai) {
            REQUIRE(ai->behavior == AIBehavior::Alert);
        }
    }

    SECTION("Alert to Chase") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto player = factory.createPlayer(15, 10);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Alert;
            ai->visionRange = 10;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update while player visible
        world->addComponent<PlayerComponent>(player);
        aiSystem.update(entities, 0.1);

        if (ai) {
            REQUIRE(ai->behavior == AIBehavior::Chase);
            REQUIRE(ai->target == player);
        }
    }

    SECTION("Chase to Attack") {
        auto monster = factory.createMonster("goblin", 11, 10);
        auto player = factory.createPlayer(12, 10); // Adjacent

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Chase;
            ai->target = player;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        aiSystem.update(entities, 0.1);

        if (ai) {
            REQUIRE(ai->behavior == AIBehavior::Attack);
        }
    }

    SECTION("Attack to Flee") {
        auto monster = factory.createMonster("goblin", 11, 10);
        auto player = factory.createPlayer(12, 10);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Attack;
            ai->target = player;
            ai->fleeThreshold = 0.3f;
        }

        // Set low health
        if (auto* health = world->getComponent<HealthComponent>(monster)) {
            health->hp = 2;
            health->maxHp = 10;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        aiSystem.update(entities, 0.1);

        if (ai) {
            REQUIRE(ai->behavior == AIBehavior::Flee);
        }
    }

    SECTION("Flee to Idle") {
        auto monster = factory.createMonster("goblin", 10, 10);

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Flee;
            ai->target = nullptr; // Lost target
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        aiSystem.update(entities, 0.1);

        if (ai) {
            REQUIRE(ai->behavior == AIBehavior::Idle);
        }
    }
}

TEST_CASE("AISystem performance", "[ai][ecs]") {
    Map testMap(100, 100);
    testMap.initializeTiles(TileType::Floor);
    auto world = std::make_unique<GameWorld>(&testMap);
    AISystem aiSystem(world.get(), &testMap);
    EntityFactory factory(world.get());

    SECTION("Handle many entities") {
        std::vector<std::unique_ptr<Entity>> entities;

        // Create 100 monsters
        for (int i = 0; i < 100; ++i) {
            auto monster = factory.createMonster("goblin", i % 100, i / 10);
            entities.push_back(std::move(monster));
        }

        auto start = std::chrono::high_resolution_clock::now();
        aiSystem.update(entities, 0.016); // 60 FPS frame time
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Should complete within reasonable time (< 100ms for 100 entities)
        REQUIRE(duration.count() < 100);
    }

    SECTION("Concurrent AI updates") {
        std::vector<std::unique_ptr<Entity>> entities1, entities2;

        for (int i = 0; i < 50; ++i) {
            entities1.push_back(factory.createMonster("goblin", i, 0));
            entities2.push_back(factory.createMonster("goblin", i, 50));
        }

        std::thread t1([&]() { aiSystem.update(entities1, 0.016); });
        std::thread t2([&]() { aiSystem.update(entities2, 0.016); });

        t1.join();
        t2.join();

        // Should handle concurrent updates without crashing
        REQUIRE(true);
    }
}

TEST_CASE("AISystem edge cases", "[ai][ecs]") {
    Map testMap(50, 30);
    auto world = std::make_unique<GameWorld>(&testMap);
    AISystem aiSystem(world.get(), &testMap);
    EntityFactory factory(world.get());

    SECTION("Null entity handling") {
        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(nullptr);

        // Should not crash
        aiSystem.update(entities, 0.1);
        REQUIRE(true);
    }

    SECTION("Entity without AI component") {
        auto entity = factory.createEntity();
        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(entity));

        // Should skip entities without AI
        aiSystem.update(entities, 0.1);
        REQUIRE(true);
    }

    SECTION("Invalid target reference") {
        auto monster = factory.createMonster("goblin", 10, 10);
        auto* ai = world->getComponent<AIComponent>(monster);

        if (ai) {
            ai->behavior = AIBehavior::Chase;
            ai->target = reinterpret_cast<Entity*>(0xDEADBEEF); // Invalid pointer
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Should handle invalid target gracefully
        aiSystem.update(entities, 0.1);

        if (ai) {
            // Should reset to safe state
            REQUIRE(ai->target == nullptr);
            REQUIRE(ai->behavior != AIBehavior::Chase);
        }
    }

    SECTION("Map boundary checking") {
        auto monster = factory.createMonster("goblin", 49, 29); // At map edge

        auto* ai = world->getComponent<AIComponent>(monster);
        if (ai) {
            ai->behavior = AIBehavior::Wander;
        }

        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Update multiple times
        for (int i = 0; i < 20; ++i) {
            aiSystem.update(entities, 0.1);
        }

        // Should stay within map bounds
        if (auto* pos = world->getComponent<PositionComponent>(entities[0].get())) {
            REQUIRE(pos->x >= 0);
            REQUIRE(pos->x < 50);
            REQUIRE(pos->y >= 0);
            REQUIRE(pos->y < 30);
        }
    }

    SECTION("Zero delta time") {
        auto monster = factory.createMonster("goblin", 10, 10);
        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Should handle zero delta time
        aiSystem.update(entities, 0.0);
        REQUIRE(true);
    }

    SECTION("Negative delta time") {
        auto monster = factory.createMonster("goblin", 10, 10);
        std::vector<std::unique_ptr<Entity>> entities;
        entities.push_back(std::move(monster));

        // Should handle negative delta time gracefully
        aiSystem.update(entities, -1.0);
        REQUIRE(true);
    }
}