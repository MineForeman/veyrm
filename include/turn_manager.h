/**
 * @file turn_manager.h
 * @brief Turn-based game flow and action scheduling
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <vector>
#include <functional>
#include <queue>
#include <memory>

/**
 * @enum TurnPhase
 * @brief Current phase of turn processing
 */
enum class TurnPhase {
    WAITING_FOR_INPUT, ///< Waiting for player input
    PLAYER_ACTION,     ///< Processing player action
    WORLD_UPDATE,      ///< Updating world state
    TURN_COMPLETE      ///< Turn finished
};

/**
 * @enum ActionSpeed
 * @brief Time cost of actions in action points
 */
enum class ActionSpeed {
    INSTANT = 0,      ///< No time cost
    FAST = 50,        ///< Half a turn
    NORMAL = 100,     ///< Standard turn
    SLOW = 150,       ///< 1.5 turns
    VERY_SLOW = 200   ///< 2 turns
};

// Forward declarations
class GameManager;

/**
 * @struct ScheduledAction
 * @brief Action scheduled for future execution
 */
struct ScheduledAction {
    int executionTime;              ///< World time when action executes
    std::function<void()> action;   ///< Function to execute

    /**
     * @brief Comparison for priority queue (min-heap)
     * @param other Action to compare with
     * @return true if this executes later than other
     */
    bool operator>(const ScheduledAction& other) const {
        return executionTime > other.executionTime;
    }
};

/**
 * @class TurnManager
 * @brief Manages turn-based gameplay and action scheduling
 *
 * The TurnManager coordinates the turn-based flow of the game, handling
 * player actions, monster actions, and world updates. It uses an action
 * point system where different actions have different time costs, allowing
 * for varied action speeds.
 *
 * @see GameManager
 * @see ActionSpeed
 */
class TurnManager {
public:
    /**
     * @brief Construct TurnManager
     * @param game_manager Pointer to game manager
     */
    TurnManager(GameManager* game_manager);

    // Turn flow control

    /**
     * @brief Begin a new player turn
     */
    void startPlayerTurn();

    /**
     * @brief Execute player action with time cost
     * @param speed Time cost of the action
     */
    void executePlayerAction(ActionSpeed speed);

    /**
     * @brief Process world updates (monsters, effects)
     */
    void processWorldTurn();

    /**
     * @brief Complete the current turn
     */
    void endTurn();
    
    // Action scheduling

    /**
     * @brief Schedule action for future execution
     * @param delay Delay in action points
     * @param action Function to execute
     */
    void scheduleAction(int delay, std::function<void()> action);

    /**
     * @brief Process all scheduled actions due
     */
    void processScheduledActions();

    // Turn information

    /**
     * @brief Get current turn number
     * @return Turn count since game start
     */
    int getCurrentTurn() const { return current_turn; }

    /**
     * @brief Get world time in action points
     * @return Total action points elapsed
     */
    int getWorldTime() const { return world_time; }

    /**
     * @brief Get current turn phase
     * @return Current TurnPhase
     */
    TurnPhase getCurrentPhase() const { return current_phase; }

    /**
     * @brief Check if waiting for player input
     * @return true if player's turn
     */
    bool isPlayerTurn() const { return current_phase == TurnPhase::WAITING_FOR_INPUT; }
    
    // Speed/time management

    /**
     * @brief Get time cost of action
     * @param speed Action speed enum
     * @return Cost in action points
     */
    int getActionCost(ActionSpeed speed) const;

    /**
     * @brief Advance world time
     * @param amount Action points to advance
     */
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