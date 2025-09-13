#include "game_state.h"
#include "input_handler.h"
#include "turn_manager.h"
#include "message_log.h"
#include "frame_stats.h"
#include "map.h"
#include "color_scheme.h"
#include "map_generator.h"
#include "map_validator.h"
#include "entity_manager.h"
#include "player.h"
#include "fov.h"
#include "map_memory.h"

GameManager::GameManager(MapType initial_map) 
    : current_state(GameState::MENU),
      previous_state(GameState::MENU),
      input_handler(std::make_unique<InputHandler>()),
      turn_manager(std::make_unique<TurnManager>(this)),
      message_log(std::make_unique<MessageLog>()),
      frame_stats(std::make_unique<FrameStats>()),
      map(std::make_unique<Map>()),
      entity_manager(std::make_unique<EntityManager>()),
      debug_mode(false) {
    
    // Initialize color scheme with auto-detection
    ColorScheme::setCurrentTheme(TerminalTheme::AUTO_DETECT);
    
    // Initialize map with MapGenerator
    initializeMap(initial_map);
    
    // Don't set anything visible initially - FOV will handle visibility
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
    
    // Clear existing entities
    entity_manager->clear();
    
    // Set player spawn point
    Point spawn = MapGenerator::getDefaultSpawnPoint(type);
    
    // Verify spawn point is walkable
    if (!Map::getTileProperties(map->getTile(spawn.x, spawn.y)).walkable) {
        // Fallback to finding any safe spawn point
        spawn = MapGenerator::findSafeSpawnPoint(*map);
        message_log->addSystemMessage("Using fallback spawn point");
    }
    
    // Create player entity at spawn point
    auto player = entity_manager->createPlayer(spawn.x, spawn.y);
    
    // Update deprecated variables for compatibility
    if (player) {
        player_x = player->x;
        player_y = player->y;
        player_hp = player->hp;
        player_max_hp = player->max_hp;
    }
    
    // Log map statistics
    message_log->addSystemMessage("Map: " + std::to_string(validation.walkable_tiles) + 
                                 " walkable tiles, " + std::to_string(validation.room_count) + 
                                 " rooms");
    
    // Calculate initial FOV from player position
    updateFOV();
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

Player* GameManager::getPlayer() {
    if (entity_manager) {
        auto player_ptr = entity_manager->getPlayer();
        return player_ptr ? player_ptr.get() : nullptr;
    }
    return nullptr;
}

void GameManager::update([[maybe_unused]] double deltaTime) {
    // Skip updates if not playing
    if (current_state != GameState::PLAYING) {
        return;
    }
    
    // Update all entities
    if (entity_manager) {
        entity_manager->updateAll(deltaTime);
    }
    
    // Update deprecated player position variables
    if (auto player = getPlayer()) {
        player_x = player->x;
        player_y = player->y;
        player_hp = player->hp;
        player_max_hp = player->max_hp;
    }
    
    // Update game systems
    // Future: Update animations, particles, etc.
    // For now, the turn system handles its own timing
}

void GameManager::processInput() {
    // Input is handled by FTXUI events for now
    // This is a placeholder for future input processing
}

void GameManager::updateFOV() {
    if (!entity_manager || !map) return;
    
    Player* player = getPlayer();
    if (!player) return;
    
    // Calculate FOV from player position
    Point playerPos(player->x, player->y);
    FOV::calculate(*map, playerPos, FOV::DEFAULT_RADIUS, current_fov);
    
    // Check if player entered a new room
    Room* new_room = map->getRoomAt(playerPos);
    if (new_room != current_room) {
        // Player entered a different room (or left a room)
        Room* old_room = current_room;
        current_room = new_room;
        
        // If entering a lit room, reveal it
        if (current_room && current_room->isLit()) {
            // Make entire lit room visible
            for (const auto& tile : current_room->getFloorTiles()) {
                if (map->inBounds(tile)) {
                    current_fov[tile.y][tile.x] = true;
                    map->setExplored(tile.x, tile.y, true);
                }
            }
            
            // Also reveal the walls around the room
            for (int y = current_room->top() - 1; y <= current_room->bottom() + 1; y++) {
                for (int x = current_room->left() - 1; x <= current_room->right() + 1; x++) {
                    if (map->inBounds(x, y)) {
                        current_fov[y][x] = true;
                        map->setExplored(x, y, true);
                    }
                }
            }
            
            message_log->addSystemMessage("The room is lit!");
        }
        
        // If leaving a lit room, keep it explored but not fully visible
        if (old_room && old_room->isLit() && old_room != current_room) {
            message_log->addSystemMessage("You leave the lit room.");
        }
    } else if (current_room && current_room->isLit()) {
        // Player is still in a lit room, keep it fully visible
        for (const auto& tile : current_room->getFloorTiles()) {
            if (map->inBounds(tile)) {
                current_fov[tile.y][tile.x] = true;
            }
        }
        
        // Keep walls visible too
        for (int y = current_room->top() - 1; y <= current_room->bottom() + 1; y++) {
            for (int x = current_room->left() - 1; x <= current_room->right() + 1; x++) {
                if (map->inBounds(x, y)) {
                    current_fov[y][x] = true;
                }
            }
        }
    }
    
    // Update map memory with new visibility
    if (map_memory) {
        map_memory->updateVisibility(*map, current_fov);
    }
    
    // Update map's visibility flags (for compatibility)
    for (int y = 0; y < map->getHeight(); y++) {
        for (int x = 0; x < map->getWidth(); x++) {
            map->setVisible(x, y, current_fov[y][x]);
            if (current_fov[y][x]) {
                map->setExplored(x, y, true);
            }
        }
    }
    
    // Update entity visibility based on FOV
    if (entity_manager) {
        entity_manager->updateEntityVisibility(current_fov);
    }
}