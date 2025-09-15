/**
 * @file game_state.h
 * @brief Core game state management and coordination
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <memory>
#include <vector>
#include "map_generator.h"

/**
 * @enum GameState
 * @brief Possible game states for the main loop
 */
enum class GameState {
    MENU,       ///< Main menu screen
    PLAYING,    ///< Active gameplay
    PAUSED,     ///< Game paused
    INVENTORY,  ///< Inventory management screen
    HELP,       ///< Help/controls screen
    SAVE_LOAD,  ///< Save/Load menu
    DEATH,      ///< Player death screen
    QUIT        ///< Exit game
};

enum class ActionSpeed;

// Forward declarations
class InputHandler;
class TurnManager;
class MessageLog;
class FrameStats;
class Map;
// class Player;  // Legacy - removed
// SpawnManager removed - using ECS spawning
class MapMemory;
// class ItemManager;  // Legacy - removed
class GameSerializer;

// Forward declare ECS namespace
namespace ecs {
    class GameWorld;
}

/**
 * @class GameManager
 * @brief Central game state and system coordinator
 *
 * The GameManager is the central hub that coordinates all game systems
 * including entities, map, combat, AI, input, and rendering. It manages
 * the overall game state, handles save/load functionality, and provides
 * access to all major subsystems.
 *
 * @see GameLoop
 * @see ecs::GameWorld
 * @see Map
 * @see TurnManager
 */
class GameManager {
public:
    /**
     * @brief Construct GameManager with initial map type
     * @param initial_map Type of map to generate on start
     */
    GameManager(MapType initial_map = MapType::TEST_DUNGEON);

    /// Destructor
    ~GameManager();
    
    // State management

    /**
     * @brief Get current game state
     * @return Current GameState enum value
     */
    GameState getState() const { return current_state; }

    /**
     * @brief Change game state
     * @param state New game state
     * @note Stores previous state for return
     */
    void setState(GameState state);

    /**
     * @brief Get previous game state
     * @return Previous GameState value
     */
    GameState getPreviousState() const { return previous_state; }

    /**
     * @brief Return to previous game state
     * @note Used for backing out of menus
     */
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
    // EntityManager removed - using ECS only
    Player* getPlayer();
    int getCurrentDepth() const { return current_depth; }
    void setCurrentDepth(int depth) { current_depth = depth; }
    
    // Game loop integration

    /**
     * @brief Update game state
     * @param deltaTime Time since last update (seconds)
     */
    void update(double deltaTime);

    /**
     * @brief Process player input
     */
    void processInput();

    // Debug mode

    /**
     * @brief Check if debug mode is enabled
     * @return true if in debug mode
     */
    bool isDebugMode() const { return debug_mode; }

    /**
     * @brief Enable/disable debug mode
     * @param enabled Debug mode state
     */
    void setDebugMode(bool enabled) { debug_mode = enabled; }

    /**
     * @brief Enable frame statistics
     * @deprecated Frame stats are always enabled
     */
    void enableFrameStats() { /* Frame stats are always enabled if available */ }
    
    // Spawn management
    // getSpawnManager removed - using ECS spawning
    
    // Game data (deprecated - use entity_manager->getPlayer() instead)
    int player_hp = 10;      // DEPRECATED
    int player_max_hp = 10;  // DEPRECATED
    int player_x = 30;       // DEPRECATED - Player position
    int player_y = 10;       // DEPRECATED
    
    // Game flow

    /**
     * @brief Process player action with speed cost
     * @param speed Speed cost of the action
     */
    void processPlayerAction(ActionSpeed speed);

    /**
     * @brief Check if game is still running
     * @return true if not in QUIT state
     */
    bool isGameRunning() const { return current_state != GameState::QUIT; }

    // Map initialization

    /**
     * @brief Initialize/regenerate the map
     * @param type Type of map to generate
     */
    void initializeMap(MapType type = MapType::TEST_DUNGEON);

    // FOV and visibility

    /**
     * @brief Update field of view from player position
     */
    void updateFOV();

    /**
     * @brief Get map memory system
     * @return Pointer to MapMemory
     */
    MapMemory* getMapMemory() { return map_memory.get(); }

    /**
     * @brief Get current FOV grid
     * @return 2D visibility grid
     */
    const std::vector<std::vector<bool>>& getCurrentFOV() const { return current_fov; }

    // Monster AI
    void updateMonsters();


    // Item system - Legacy (using ECS item system)
    ItemManager* getItemManager() { return nullptr; /*item_manager.get();*/ }
    const ItemManager* getItemManager() const { return nullptr; /*item_manager.get();*/ }

    // Save/Load system

    /**
     * @brief Save game to slot
     * @param slot Save slot number (0-9)
     * @return true if save succeeded
     */
    bool saveGame(int slot);

    /**
     * @brief Load game from slot
     * @param slot Save slot number (0-9)
     * @return true if load succeeded
     */
    bool loadGame(int slot);

    /**
     * @brief Get game serializer
     * @return Pointer to GameSerializer
     */
    GameSerializer* getSerializer() { return serializer.get(); }

    // ECS Integration

    /**
     * @brief Get ECS game world
     * @return Pointer to ECS GameWorld
     */
    ecs::GameWorld* getECSWorld() { return ecs_world.get(); }
    const ecs::GameWorld* getECSWorld() const { return ecs_world.get(); }

    /**
     * @brief Enable ECS mode
     * @param enable If true, use ECS for entity management
     */
    void setECSMode(bool enable) { use_ecs = enable; }

    /**
     * @brief Check if ECS mode is enabled
     * @return true if using ECS
     */
    bool isECSMode() const { return use_ecs; }

    /**
     * @brief Initialize ECS world
     * @param migrate_existing If true, migrate existing entities to ECS
     */
    void initializeECS(bool migrate_existing = true);

private:
    GameState current_state = GameState::MENU;
    GameState previous_state = GameState::MENU;
    std::unique_ptr<InputHandler> input_handler;
    std::unique_ptr<TurnManager> turn_manager;
    std::unique_ptr<MessageLog> message_log;
    std::unique_ptr<FrameStats> frame_stats;
    std::unique_ptr<Map> map;
    // spawn_manager removed - using ECS spawning
    std::unique_ptr<MapMemory> map_memory;
    // std::unique_ptr<ItemManager> item_manager;  // Legacy - using ECS item system
    std::unique_ptr<GameSerializer> serializer;
    std::unique_ptr<ecs::GameWorld> ecs_world;  ///< ECS world manager
    std::vector<std::vector<bool>> current_fov;
    bool use_ecs = false;  ///< Flag to enable ECS mode

    // Room tracking - using observer pointer since Map owns the rooms
    // This is safe because rooms lifetime is tied to Map lifetime
    // and Map is owned by this GameManager
    const Room* current_room = nullptr;  // Observer pointer to current room

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

    // Room tracking
    const Room* getCurrentRoom() const { return current_room; }
    void setCurrentRoom(const Room* room) { current_room = room; }

    // Save/Load UI
    bool getSaveMenuMode() const { return save_menu_mode; }
    void setSaveMenuMode(bool save_mode) { save_menu_mode = save_mode; }
};