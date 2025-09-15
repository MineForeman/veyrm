#include <catch2/catch_test_macros.hpp>
#include "../include/game_state.h"
#include "../include/ecs/game_world.h"
#include "../include/ecs/movement_system.h"
#include "../include/ecs/render_system.h"
#include "../include/ecs/position_component.h"
#include "../include/ecs/health_component.h"
#include "../include/ecs/combat_component.h"
#include "../include/entity_manager.h"
// legacy combat_system.h removed - using ECS CombatSystem
#include "../include/ecs/combat_system.h"
#include "../include/message_log.h"
#include "../include/map.h"
#include "../include/config.h"

TEST_CASE("ECS Integration with GameManager", "[ecs][integration]") {
    // Initialize config
    Config::getInstance();

    SECTION("GameManager can initialize ECS") {
        GameManager game(MapType::TEST_ROOM);

        // ECS is now enabled by default
        REQUIRE(game.isECSMode() == true);
        REQUIRE(game.getECSWorld() != nullptr);

        // We can disable and re-enable ECS
        game.setECSMode(false);
        REQUIRE(game.isECSMode() == false);

        game.setECSMode(true);
        REQUIRE(game.isECSMode() == true);
    }

    SECTION("ECS world creates entities") {
        // Create standalone components
        EntityManager entity_manager;
        MessageLog message_log;
        Map map(20, 20);

        // Create ECS world
        ecs::GameWorld world(&entity_manager, &message_log, &map);
        world.initialize(false);

        // Create player
        auto player_id = world.createPlayer(10, 10);
        REQUIRE(player_id > 0);

        auto* player = world.getEntity(player_id);
        REQUIRE(player != nullptr);
        REQUIRE(player->hasComponent<ecs::PositionComponent>());
        REQUIRE(player->hasComponent<ecs::RenderableComponent>());
        REQUIRE(player->hasComponent<ecs::HealthComponent>());
        REQUIRE(player->hasComponent<ecs::CombatComponent>());

        // Verify position
        auto* pos = player->getComponent<ecs::PositionComponent>();
        REQUIRE(pos->position.x == 10);
        REQUIRE(pos->position.y == 10);
    }

    SECTION("ECS world processes movement") {
        EntityManager entity_manager;
        MessageLog message_log;
        Map map(20, 20);

        // Make map walkable
        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                map.setTile(x, y, TileType::FLOOR);
            }
        }

        ecs::GameWorld world(&entity_manager, &message_log, &map);
        world.initialize(false);

        // Create player
        auto player_id = world.createPlayer(10, 10);
        auto* player = world.getEntity(player_id);

        // Get movement system
        auto* movement_system = world.getMovementSystem();
        REQUIRE(movement_system != nullptr);

        // Move player
        bool moved = movement_system->moveEntity(*player, 1, 0);
        REQUIRE(moved == true);

        // Check new position
        auto* pos = player->getComponent<ecs::PositionComponent>();
        REQUIRE(pos->position.x == 11);
        REQUIRE(pos->position.y == 10);
    }

    SECTION("ECS world handles combat") {
        EntityManager entity_manager;
        MessageLog message_log;
        Map map(20, 20);

        ecs::GameWorld world(&entity_manager, &message_log, &map);
        world.initialize(false);

        // Create player and monster
        auto player_id = world.createPlayer(10, 10);
        auto monster_id = world.createMonster("goblin", 11, 10);

        REQUIRE(player_id > 0);
        REQUIRE(monster_id > 0);

        auto* player = world.getEntity(player_id);
        auto* monster = world.getEntity(monster_id);

        REQUIRE(player != nullptr);
        REQUIRE(monster != nullptr);

        // Process attack using native combat system
        auto* ecs_combat = world.getCombatSystem();
        REQUIRE(ecs_combat != nullptr);

        // Queue attack
        ecs_combat->queueAttack(player->getID(), monster->getID());

        // Check initial state
        INFO("Initial monster HP: " << monster->getComponent<ecs::HealthComponent>()->hp);
        INFO("Monster position: " << monster->getComponent<ecs::PositionComponent>()->position.x
             << "," << monster->getComponent<ecs::PositionComponent>()->position.y);
        INFO("Player position: " << player->getComponent<ecs::PositionComponent>()->position.x
             << "," << player->getComponent<ecs::PositionComponent>()->position.y);

        // Process the attack
        world.update(0.016); // One frame update

        // Check that monster took damage
        auto* monster_health = monster->getComponent<ecs::HealthComponent>();
        REQUIRE(monster_health != nullptr);
        INFO("After update monster HP: " << monster_health->hp);

        // If this fails, print diagnostic info
        if (monster_health->hp >= monster_health->max_hp) {
            INFO("Monster HP: " << monster_health->hp << "/" << monster_health->max_hp);
            INFO("Player has CombatComponent: " << player->hasComponent<ecs::CombatComponent>());
            INFO("Monster has CombatComponent: " << monster->hasComponent<ecs::CombatComponent>());

            // Let's try updating again in case it needs more time
            world.update(0.016);

            // Check again
            REQUIRE(monster_health->hp < monster_health->max_hp);
        } else {
            REQUIRE(monster_health->hp < monster_health->max_hp);
        }
    }

    // Legacy sync test removed - using full ECS mode only

    SECTION("GameManager with ECS mode processes updates") {
        GameManager game(MapType::TEST_ROOM);
        game.initializeECS(true);  // Migrate existing

        // Update should work with ECS
        game.update(0.016);  // 60 FPS

        // Should have ECS world
        auto* ecs_world = game.getECSWorld();
        REQUIRE(ecs_world != nullptr);

        // Should have entities
        REQUIRE(ecs_world->getEntityCount() > 0);
    }
}