#include "turn_manager.h"
#include "game_state.h"
#include <iostream>

TurnManager::TurnManager(GameManager* gm) 
    : game_manager(gm),
      current_turn(0),
      world_time(0),
      player_next_action_time(0),
      current_phase(TurnPhase::WAITING_FOR_INPUT) {
}

void TurnManager::startPlayerTurn() {
    current_phase = TurnPhase::WAITING_FOR_INPUT;
    // Player can act when world_time >= player_next_action_time
}

void TurnManager::executePlayerAction(ActionSpeed speed) {
    if (current_phase != TurnPhase::WAITING_FOR_INPUT) {
        return;
    }
    
    current_phase = TurnPhase::PLAYER_ACTION;
    
    // Calculate time cost of action
    int cost = getActionCost(speed);
    player_next_action_time = world_time + cost;
    
    // Process the action (handled by game logic)
    current_turn++;
    
    // Move to world update phase
    processWorldTurn();
}

void TurnManager::processWorldTurn() {
    current_phase = TurnPhase::WORLD_UPDATE;
    
    // Advance time to next player action
    int time_to_advance = player_next_action_time - world_time;
    if (time_to_advance > 0) {
        advanceTime(time_to_advance);
    }
    
    // Process any scheduled actions that are due
    processScheduledActions();

    // Update monsters
    if (game_manager) {
        game_manager->updateMonsters();
    }

    endTurn();
}

void TurnManager::endTurn() {
    current_phase = TurnPhase::TURN_COMPLETE;
    
    // Check if player can act again
    if (world_time >= player_next_action_time) {
        startPlayerTurn();
    }
}

void TurnManager::scheduleAction(int delay, std::function<void()> action) {
    action_queue.push({world_time + delay, action});
}

void TurnManager::processScheduledActions() {
    while (!action_queue.empty() && 
           action_queue.top().executionTime <= world_time) {
        auto scheduled = action_queue.top();
        action_queue.pop();
        scheduled.action();
    }
}

int TurnManager::getActionCost(ActionSpeed speed) const {
    return static_cast<int>(speed);
}

void TurnManager::advanceTime(int amount) {
    world_time += amount;
}