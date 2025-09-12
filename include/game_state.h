#pragma once

#include <memory>

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    INVENTORY,
    HELP,
    QUIT
};

// Forward declaration
class InputHandler;

class GameManager {
public:
    GameManager();
    ~GameManager();
    
    // State management
    GameState getState() const { return current_state; }
    void setState(GameState state);
    GameState getPreviousState() const { return previous_state; }
    void returnToPreviousState();
    
    // Input handling
    InputHandler* getInputHandler() { return input_handler.get(); }
    
    // Game data
    int player_hp = 10;
    int player_max_hp = 10;
    int player_x = 30;  // Player position
    int player_y = 10;
    int turn_count = 0;
    
    // Game flow
    void incrementTurn() { turn_count++; }
    bool isGameRunning() const { return current_state != GameState::QUIT; }
    
private:
    GameState current_state = GameState::MENU;
    GameState previous_state = GameState::MENU;
    std::unique_ptr<InputHandler> input_handler;
};