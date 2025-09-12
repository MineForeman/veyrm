#pragma once

enum class GameState {
    MENU,
    PLAYING,
    QUIT
};

class GameManager {
public:
    GameManager();
    
    GameState getState() const { return current_state; }
    void setState(GameState state) { current_state = state; }
    
    // Game data that will be added later
    int player_hp = 10;
    int player_max_hp = 10;
    int turn_count = 0;
    
private:
    GameState current_state = GameState::MENU;
};

inline GameManager::GameManager() : current_state(GameState::MENU) {}