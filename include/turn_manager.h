#pragma once

#include <vector>
#include <functional>
#include <queue>
#include <memory>

enum class TurnPhase {
    WAITING_FOR_INPUT,
    PLAYER_ACTION,
    WORLD_UPDATE,
    TURN_COMPLETE
};

enum class ActionSpeed {
    INSTANT = 0,      // No time cost
    FAST = 50,        // Half a turn
    NORMAL = 100,     // Standard turn
    SLOW = 150,       // 1.5 turns
    VERY_SLOW = 200   // 2 turns
};

// Forward declarations
class GameManager;

// Represents a scheduled action
struct ScheduledAction {
    int executionTime;
    std::function<void()> action;
    
    bool operator>(const ScheduledAction& other) const {
        return executionTime > other.executionTime;
    }
};

class TurnManager {
public:
    TurnManager(GameManager* game_manager);
    
    // Turn flow control
    void startPlayerTurn();
    void executePlayerAction(ActionSpeed speed);
    void processWorldTurn();
    void endTurn();
    
    // Action scheduling
    void scheduleAction(int delay, std::function<void()> action);
    void processScheduledActions();
    
    // Turn information
    int getCurrentTurn() const { return current_turn; }
    int getWorldTime() const { return world_time; }
    TurnPhase getCurrentPhase() const { return current_phase; }
    bool isPlayerTurn() const { return current_phase == TurnPhase::WAITING_FOR_INPUT; }
    
    // Speed/time management
    int getActionCost(ActionSpeed speed) const;
    void advanceTime(int amount);
    
private:
    [[maybe_unused]] GameManager* game_manager;  // Will be used for entity updates
    int current_turn;
    int world_time;  // In action points (100 = 1 standard turn)
    int player_next_action_time;
    TurnPhase current_phase;
    
    // Priority queue for scheduled actions
    std::priority_queue<ScheduledAction, 
                       std::vector<ScheduledAction>, 
                       std::greater<ScheduledAction>> action_queue;
};