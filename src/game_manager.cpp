#include "game_state.h"
#include "input_handler.h"
#include "turn_manager.h"
#include "message_log.h"
#include "frame_stats.h"
#include "map.h"
#include "color_scheme.h"
#include "map_generator.h"
#include "map_validator.h"

GameManager::GameManager(MapType initial_map) 
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
    
    // Initialize map with MapGenerator
    initializeMap(initial_map);
    
    // Set everything visible for now (no FOV yet)
    for (int y = 0; y < map->getHeight(); y++) {
        for (int x = 0; x < map->getWidth(); x++) {
            map->setVisible(x, y, true);
            map->setExplored(x, y, true);  // Also mark as explored
        }
    }
}

GameManager::~GameManager() = default;

void GameManager::initializeMap(MapType type) {
    // Generate the map
    MapGenerator::generate(*map, type);
    
    // Validate the map
    auto validation = MapValidator::validate(*map);
    if (!validation.valid) {
        // Log errors
        for (const auto& error : validation.errors) {
            message_log->addSystemMessage("Map Error: " + error);
        }
    }
    
    // Log warnings
    for (const auto& warning : validation.warnings) {
        message_log->addSystemMessage("Map Warning: " + warning);
    }
    
    // Set player spawn point
    Point spawn = MapGenerator::getDefaultSpawnPoint(type);
    
    // Verify spawn point is walkable
    if (Map::getTileProperties(map->getTile(spawn.x, spawn.y)).walkable) {
        player_x = spawn.x;
        player_y = spawn.y;
    } else {
        // Fallback to finding any safe spawn point
        spawn = MapGenerator::findSafeSpawnPoint(*map);
        player_x = spawn.x;
        player_y = spawn.y;
        message_log->addSystemMessage("Using fallback spawn point");
    }
    
    // Log map statistics
    message_log->addSystemMessage("Map: " + std::to_string(validation.walkable_tiles) + 
                                 " walkable tiles, " + std::to_string(validation.room_count) + 
                                 " rooms");
}

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