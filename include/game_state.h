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
    SAVE_LOAD,  // Save/Load menu
    DEATH,
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
class CombatSystem;
class ItemManager;
class GameSerializer;

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
    int getCurrentDepth() const { return current_depth; }
    void setCurrentDepth(int depth) { current_depth = depth; }
    
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

    // Combat system
    CombatSystem* getCombatSystem() { return combat_system.get(); }

    // Item system
    ItemManager* getItemManager() { return item_manager.get(); }
    const ItemManager* getItemManager() const { return item_manager.get(); }

    // Save/Load system
    bool saveGame(int slot);
    bool loadGame(int slot);
    GameSerializer* getSerializer() { return serializer.get(); }

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
    std::unique_ptr<CombatSystem> combat_system;
    std::unique_ptr<ItemManager> item_manager;
    std::unique_ptr<GameSerializer> serializer;
    std::vector<std::vector<bool>> current_fov;
    Room* current_room = nullptr;  // Track which room the player is currently in
    bool debug_mode = false;
    int current_depth = 1;  // Track dungeon depth for spawning

    // Map generation tracking
    MapType current_map_type = MapType::TEST_DUNGEON;
    unsigned int current_map_seed = 0;  // 0 means random

    // Save/Load state
    bool save_menu_mode = true;  // true = save, false = load

public:
    // Map generation
    MapType getCurrentMapType() const { return current_map_type; }
    void setCurrentMapType(MapType type) { current_map_type = type; }
    unsigned int getCurrentMapSeed() const { return current_map_seed; }
    void setCurrentMapSeed(unsigned int seed) { current_map_seed = seed; }

    // Save/Load UI
    bool getSaveMenuMode() const { return save_menu_mode; }
    void setSaveMenuMode(bool save_mode) { save_menu_mode = save_mode; }
};