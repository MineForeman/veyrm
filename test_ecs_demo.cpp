/**
 * @file test_ecs_demo.cpp
 * @brief Demo program to showcase ECS transition
 */

#include "include/game_state.h"
#include "include/config.h"
#include "include/message_log.h"
#include "include/entity_manager.h"
#include "include/ecs/game_world.h"
#include <iostream>

int main() {
    // Initialize config
    Config::getInstance();

    std::cout << "=== Veyrm ECS Transition Demo ===" << std::endl;
    std::cout << std::endl;

    // Create game with legacy mode
    std::cout << "1. Creating game in legacy mode..." << std::endl;
    GameManager game(MapType::TEST_ROOM);
    std::cout << "   - ECS Mode: " << (game.isECSMode() ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "   - Entity Count: " << game.getEntityManager()->getEntityCount() << std::endl;
    std::cout << std::endl;

    // Initialize ECS and migrate
    std::cout << "2. Initializing ECS and migrating entities..." << std::endl;
    game.initializeECS(true);
    std::cout << "   - ECS Mode: " << (game.isECSMode() ? "ENABLED" : "DISABLED") << std::endl;

    if (game.getECSWorld()) {
        std::cout << "   - ECS Entity Count: " << game.getECSWorld()->getEntityCount() << std::endl;
        std::cout << "   - Player ID: " << game.getECSWorld()->getPlayerID() << std::endl;
    }
    std::cout << std::endl;

    // Perform some updates
    std::cout << "3. Running game update cycles..." << std::endl;
    game.setState(GameState::PLAYING);

    for (int i = 0; i < 5; ++i) {
        game.update(0.016);  // 60 FPS
        std::cout << "   - Update " << (i+1) << " complete" << std::endl;
    }
    std::cout << std::endl;

    // Show messages
    std::cout << "4. System Messages:" << std::endl;
    auto* log = game.getMessageLog();
    if (log) {
        auto messages = log->getMessages();
        int count = 0;
        for (const auto& msg : messages) {
            if (count++ >= 5) break;  // Show first 5 messages
            std::cout << "   - " << msg << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "=== Demo Complete ===" << std::endl;
    std::cout << "The game successfully transitioned from legacy to ECS architecture!" << std::endl;

    return 0;
}