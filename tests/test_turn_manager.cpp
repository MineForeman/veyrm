#include <catch2/catch_test_macros.hpp>
#include "turn_manager.h"
#include "game_state.h"

TEST_CASE("TurnManager: Basic turn tracking", "[turn_manager]") {
    GameManager game_manager;
    TurnManager turn_manager(&game_manager);
    
    SECTION("Initial state") {
        REQUIRE(turn_manager.getCurrentTurn() == 0);
        REQUIRE(turn_manager.getWorldTime() == 0);
        REQUIRE(turn_manager.isPlayerTurn() == true);
    }
    
    SECTION("Turn incrementing") {
        int initial_turn = turn_manager.getCurrentTurn();
        
        turn_manager.executePlayerAction(ActionSpeed::NORMAL);
        
        REQUIRE(turn_manager.getCurrentTurn() > initial_turn);
    }
}

TEST_CASE("TurnManager: Action speed effects", "[turn_manager]") {
    GameManager game_manager;
    TurnManager turn_manager(&game_manager);
    
    SECTION("Fast action") {
        int initial_time = turn_manager.getWorldTime();
        
        turn_manager.executePlayerAction(ActionSpeed::FAST);
        
        int time_passed = turn_manager.getWorldTime() - initial_time;
        REQUIRE(time_passed == 50);  // FAST = 50 time units
    }
    
    SECTION("Normal action") {
        int initial_time = turn_manager.getWorldTime();
        
        turn_manager.executePlayerAction(ActionSpeed::NORMAL);
        
        int time_passed = turn_manager.getWorldTime() - initial_time;
        REQUIRE(time_passed == 100);  // NORMAL = 100 time units
    }
    
    SECTION("Slow action") {
        int initial_time = turn_manager.getWorldTime();
        
        turn_manager.executePlayerAction(ActionSpeed::SLOW);
        
        int time_passed = turn_manager.getWorldTime() - initial_time;
        REQUIRE(time_passed == 150);  // SLOW = 150 time units
    }
}

TEST_CASE("TurnManager: World time tracking", "[turn_manager]") {
    GameManager game_manager;
    TurnManager turn_manager(&game_manager);
    
    SECTION("Time accumulates") {
        int initial_time = turn_manager.getWorldTime();
        
        // Execute several actions
        turn_manager.executePlayerAction(ActionSpeed::FAST);    // +50
        turn_manager.executePlayerAction(ActionSpeed::NORMAL);  // +100
        turn_manager.executePlayerAction(ActionSpeed::SLOW);    // +150
        
        int total_time = turn_manager.getWorldTime() - initial_time;
        REQUIRE(total_time == 300);
    }
    
    SECTION("Turn count increases") {
        int initial_turn = turn_manager.getCurrentTurn();
        
        // Execute several actions
        for (int i = 0; i < 5; ++i) {
            turn_manager.executePlayerAction(ActionSpeed::NORMAL);
        }
        
        REQUIRE(turn_manager.getCurrentTurn() == initial_turn + 5);
    }
}

TEST_CASE("TurnManager: Player turn state", "[turn_manager]") {
    GameManager game_manager;
    TurnManager turn_manager(&game_manager);
    
    SECTION("Starts as player turn") {
        REQUIRE(turn_manager.isPlayerTurn() == true);
    }
    
    SECTION("Remains player turn after action") {
        turn_manager.executePlayerAction(ActionSpeed::NORMAL);
        
        // In current implementation, it's always player's turn
        // (no monster turns implemented yet)
        REQUIRE(turn_manager.isPlayerTurn() == true);
    }
}

TEST_CASE("TurnManager: Turn persistence", "[turn_manager]") {
    GameManager game_manager;
    TurnManager turn_manager(&game_manager);
    
    SECTION("Turns persist throughout game") {
        // Advance some turns
        for (int i = 0; i < 10; ++i) {
            turn_manager.executePlayerAction(ActionSpeed::NORMAL);
        }
        
        int turns = turn_manager.getCurrentTurn();
        int time = turn_manager.getWorldTime();
        
        REQUIRE(turns > 0);
        REQUIRE(time > 0);
        
        // Turns continue to increment
        turn_manager.executePlayerAction(ActionSpeed::NORMAL);
        
        REQUIRE(turn_manager.getCurrentTurn() == turns + 1);
        REQUIRE(turn_manager.getWorldTime() > time);
    }
}

TEST_CASE("TurnManager: Action queue", "[turn_manager]") {
    GameManager game_manager;
    TurnManager turn_manager(&game_manager);
    
    SECTION("Multiple actions in sequence") {
        std::vector<ActionSpeed> actions = {
            ActionSpeed::FAST,
            ActionSpeed::FAST,
            ActionSpeed::NORMAL,
            ActionSpeed::SLOW,
            ActionSpeed::NORMAL
        };
        
        int expected_time = 0;
        for (auto speed : actions) {
            switch(speed) {
                case ActionSpeed::FAST: expected_time += 50; break;
                case ActionSpeed::NORMAL: expected_time += 100; break;
                case ActionSpeed::SLOW: expected_time += 150; break;
                default: break;
            }
        }
        
        int initial_time = turn_manager.getWorldTime();
        
        for (auto speed : actions) {
            turn_manager.executePlayerAction(speed);
        }
        
        int actual_time = turn_manager.getWorldTime() - initial_time;
        REQUIRE(actual_time == expected_time);
        REQUIRE(turn_manager.getCurrentTurn() == static_cast<int>(actions.size()));
    }
}