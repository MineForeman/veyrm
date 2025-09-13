#pragma once

#include <memory>
#include <vector>
#include "map_generator.h"

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
class EntityManager;
class Player;
class SpawnManager;
class MapMemory;
class MonsterAI;

class GameManager {
public:
    GameManager(MapType initial_map = MapType::TEST_DUNGEON);
    ~GameManager();
    
    // State management
    GameState getState() const { return current_state; }
    void setState(GameState state);
    GameState getPreviousState() const { return previous_state; }
    void returnToPreviousState();
    
    // Component access
    InputHandler* getInputHandler() { return input_handler.get(); }
    TurnManager* getTurnManager() { return turn_manager.get(); }
    TurnManager* getTurnManager() const { return turn_manager.get(); }
    MessageLog* getMessageLog() { return message_log.get(); }
    FrameStats* getFrameStats() { return frame_stats.get(); }
    FrameStats* getFrameStats() const { return frame_stats.get(); }
    Map* getMap() { return map.get(); }
    const Map* getMap() const { return map.get(); }
    EntityManager* getEntityManager() { return entity_manager.get(); }
    const EntityManager* getEntityManager() const { return entity_manager.get(); }
    Player* getPlayer();
    
    // Game loop integration
    void update(double deltaTime);
    void processInput();
    
    // Debug mode
    bool isDebugMode() const { return debug_mode; }
    void setDebugMode(bool enabled) { debug_mode = enabled; }
    void enableFrameStats() { /* Frame stats are always enabled if available */ }
    
    // Spawn management
    SpawnManager* getSpawnManager() { return spawn_manager.get(); }
    
    // Game data (deprecated - use entity_manager->getPlayer() instead)
    int player_hp = 10;      // DEPRECATED
    int player_max_hp = 10;  // DEPRECATED
    int player_x = 30;       // DEPRECATED - Player position
    int player_y = 10;       // DEPRECATED
    
    // Game flow
    void processPlayerAction(ActionSpeed speed);
    bool isGameRunning() const { return current_state != GameState::QUIT; }
    
    // Map initialization
    void initializeMap(MapType type = MapType::TEST_DUNGEON);
    
    // FOV and visibility
    void updateFOV();
    MapMemory* getMapMemory() { return map_memory.get(); }
    const std::vector<std::vector<bool>>& getCurrentFOV() const { return current_fov; }

    // Monster AI
    void updateMonsters();
    MonsterAI* getMonsterAI() { return monster_ai.get(); }
    
private:
    GameState current_state = GameState::MENU;
    GameState previous_state = GameState::MENU;
    std::unique_ptr<InputHandler> input_handler;
    std::unique_ptr<TurnManager> turn_manager;
    std::unique_ptr<MessageLog> message_log;
    std::unique_ptr<FrameStats> frame_stats;
    std::unique_ptr<Map> map;
    std::unique_ptr<EntityManager> entity_manager;
    std::unique_ptr<SpawnManager> spawn_manager;
    std::unique_ptr<MapMemory> map_memory;
    std::unique_ptr<MonsterAI> monster_ai;
    std::vector<std::vector<bool>> current_fov;
    Room* current_room = nullptr;  // Track which room the player is currently in
    bool debug_mode = false;
    int current_depth = 1;  // Track dungeon depth for spawning
};