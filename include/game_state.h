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

enum class ActionSpeed;

// Forward declarations
class InputHandler;
class TurnManager;
class MessageLog;
class FrameStats;
class Map;

class GameManager {
public:
    GameManager();
    ~GameManager();
    
    // State management
    GameState getState() const { return current_state; }
    void setState(GameState state);
    GameState getPreviousState() const { return previous_state; }
    void returnToPreviousState();
    
    // Component access
    InputHandler* getInputHandler() { return input_handler.get(); }
    TurnManager* getTurnManager() { return turn_manager.get(); }
    MessageLog* getMessageLog() { return message_log.get(); }
    FrameStats* getFrameStats() { return frame_stats.get(); }
    Map* getMap() { return map.get(); }
    
    // Game loop integration
    void update(double deltaTime);
    void processInput();
    
    // Debug mode
    bool isDebugMode() const { return debug_mode; }
    void setDebugMode(bool enabled) { debug_mode = enabled; }
    
    // Game data
    int player_hp = 10;
    int player_max_hp = 10;
    int player_x = 30;  // Player position
    int player_y = 10;
    
    // Game flow
    void processPlayerAction(ActionSpeed speed);
    bool isGameRunning() const { return current_state != GameState::QUIT; }
    
private:
    GameState current_state = GameState::MENU;
    GameState previous_state = GameState::MENU;
    std::unique_ptr<InputHandler> input_handler;
    std::unique_ptr<TurnManager> turn_manager;
    std::unique_ptr<MessageLog> message_log;
    std::unique_ptr<FrameStats> frame_stats;
    std::unique_ptr<Map> map;
    bool debug_mode = false;
};