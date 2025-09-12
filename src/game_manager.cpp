#include "game_state.h"
#include "input_handler.h"
#include "turn_manager.h"
#include "message_log.h"
#include "frame_stats.h"
#include "map.h"
#include "color_scheme.h"

GameManager::GameManager() 
    : current_state(GameState::MENU),
      previous_state(GameState::MENU),
      input_handler(std::make_unique<InputHandler>()),
      turn_manager(std::make_unique<TurnManager>(this)),
      message_log(std::make_unique<MessageLog>()),
      frame_stats(std::make_unique<FrameStats>()),
      map(std::make_unique<Map>()),
      debug_mode(false) {
    
    // Initialize color scheme with auto-detection
    ColorScheme::setCurrentTheme(TerminalTheme::AUTO_DETECT);
    
    // Create a larger test map to demonstrate viewport
    map->fill(TileType::VOID);
    
    // Create multiple rooms
    map->createRoom(10, 5, 20, 10);   // Top-left room
    map->createRoom(35, 5, 20, 10);   // Top-right room
    map->createRoom(10, 18, 20, 10);  // Bottom-left room
    map->createRoom(35, 18, 25, 10);  // Bottom-right room (larger)
    map->createRoom(22, 10, 16, 12);  // Central room
    
    // Connect rooms with corridors
    map->createCorridor(Point(20, 10), Point(30, 10));  // Top-left to central
    map->createCorridor(Point(37, 10), Point(45, 10));  // Central to top-right
    map->createCorridor(Point(20, 22), Point(30, 22));  // Bottom-left to central
    map->createCorridor(Point(37, 22), Point(45, 22));  // Central to bottom-right
    map->createCorridor(Point(30, 15), Point(30, 18));  // Central vertical
    
    // Place stairs in bottom-right room
    map->setTile(55, 25, TileType::STAIRS_DOWN);
    
    // Place player in central room initially
    player_x = 30;
    player_y = 15;
    
    // Set everything visible for now (no FOV yet)
    for (int y = 0; y < map->getHeight(); y++) {
        for (int x = 0; x < map->getWidth(); x++) {
            map->setVisible(x, y, true);
            map->setExplored(x, y, true);  // Also mark as explored
        }
    }
}

GameManager::~GameManager() = default;

void GameManager::setState(GameState state) {
    // Don't update previous state if we're going to quit
    if (state != GameState::QUIT) {
        previous_state = current_state;
    }
    current_state = state;
}

void GameManager::returnToPreviousState() {
    GameState temp = current_state;
    current_state = previous_state;
    previous_state = temp;
}

void GameManager::processPlayerAction(ActionSpeed speed) {
    turn_manager->executePlayerAction(speed);
}

void GameManager::update([[maybe_unused]] double deltaTime) {
    // Skip updates if not playing
    if (current_state != GameState::PLAYING) {
        return;
    }
    
    // Update game systems
    // Future: Update animations, entities, particles, etc.
    // For now, the turn system handles its own timing
}

void GameManager::processInput() {
    // Input is handled by FTXUI events for now
    // This is a placeholder for future input processing
}