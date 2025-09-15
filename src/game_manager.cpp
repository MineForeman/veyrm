#include "game_state.h"
#include "input_handler.h"
#include "turn_manager.h"
#include "message_log.h"
#include "frame_stats.h"
#include "map.h"
#include "color_scheme.h"
#include "map_generator.h"
#include "map_validator.h"
#include "fov.h"
#include "map_memory.h"
#include "config.h"
// spawn_manager.h removed - using ECS spawning
// MonsterFactory removed - using ECS DataLoader
// monster_ai.h removed - using ECS AISystem
// #include "monster.h"  // Legacy - removed
// combat_system.h removed - using ECS CombatSystem
#include "log.h"
#include "ecs/position_component.h"
#include "ecs/health_component.h"
// #include "item_manager.h"  // Legacy - using ECS item system
// #include "item_factory.h"  // Legacy - using ECS DataLoader
#include "game_serializer.h"
#include "ecs/game_world.h"
#include "ecs/data_loader.h"
#include <random>

GameManager::GameManager(MapType initial_map) 
    : current_state(GameState::MENU),
      previous_state(GameState::MENU),
      input_handler(std::make_unique<InputHandler>()),
      turn_manager(std::make_unique<TurnManager>(this)),
      message_log(std::make_unique<MessageLog>()),
      frame_stats(std::make_unique<FrameStats>()),
      map(std::make_unique<Map>(Config::getInstance().getMapWidth(), Config::getInstance().getMapHeight())),
      // spawn_manager removed - using ECS spawning
      // monster_ai removed - using ECS AISystem
      // combat_system removed - using ECS CombatSystem
      // item_manager(std::make_unique<ItemManager>(map.get())),  // Legacy - using ECS
      serializer(std::make_unique<GameSerializer>(this)),
      debug_mode(false) {
    
    // Initialize color scheme with auto-detection
    ColorScheme::setCurrentTheme(TerminalTheme::AUTO_DETECT);
    
    // Load ECS data (includes monsters and items)
    ecs::DataLoader::getInstance().loadAllData(
        Config::getInstance().getDataDir()
    );

    // Item data is now loaded by ECS DataLoader in loadAllData()

    // Initialize ECS system BEFORE map (so it exists when we create the player)
    initializeECS(false);  // false = don't migrate existing entities yet

    // Initialize map with MapGenerator (this creates the player)
    initializeMap(initial_map);

    // Don't set anything visible initially - FOV will handle visibility
}

GameManager::~GameManager() = default;

void GameManager::initializeMap(MapType type) {
    // Track the map type
    current_map_type = type;

    // Generate seed if not set (0 means random)
    if (current_map_seed == 0 && type == MapType::PROCEDURAL) {
        current_map_seed = std::random_device{}();
        LOG_INFO("Generated map seed: " + std::to_string(current_map_seed));
    }

    // Generate the map with seed
    if (type == MapType::PROCEDURAL) {
        MapGenerator::generate(*map, type, current_map_seed);
    } else {
        MapGenerator::generate(*map, type);
    }
    
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
    
    // Set player spawn point
    Point spawn = MapGenerator::getDefaultSpawnPoint(type);
    
    // Verify spawn point is walkable
    if (!Map::getTileProperties(map->getTile(spawn.x, spawn.y)).walkable) {
        // Fallback to finding any safe spawn point
        spawn = MapGenerator::findSafeSpawnPoint(*map);
        message_log->addSystemMessage("Using fallback spawn point");
    }
    
    // Create player entity at spawn point
    if (use_ecs && ecs_world) {
        // Create player using ECS only
        [[maybe_unused]] auto player_id = ecs_world->createPlayer(spawn.x, spawn.y);

        // No legacy player in ECS mode

        // Skip legacy monster spawning when using ECS
        // ECS handles monster creation and spawning
    } else {
        // Legacy system removed - ECS is required
        LOG_ERROR("Cannot create player without ECS enabled");
    }

    // Update deprecated variables for compatibility
    if (use_ecs && ecs_world) {
        // Get player position from ECS
        auto player_entity = ecs_world->getPlayerEntity();
        if (player_entity) {
            auto* pos = player_entity->getComponent<ecs::PositionComponent>();
            auto* health = player_entity->getComponent<ecs::HealthComponent>();
            if (pos) {
                player_x = pos->position.x;
                player_y = pos->position.y;
            }
            if (health) {
                player_hp = health->hp;
                player_max_hp = health->max_hp;
            }
        }
    } else {
        // Legacy player removed - ECS is required
        player_x = 0;
        player_y = 0;
        player_hp = 0;
        player_max_hp = 0;
    }
    
    // Log map statistics
    message_log->addSystemMessage("Map: " + std::to_string(validation.walkable_tiles) +
                                 " walkable tiles, " + std::to_string(validation.room_count) +
                                 " rooms");

    // Item spawning now handled by ECS world
    // TODO: Implement ECS-based item spawning when needed

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
    
    // After player acts, check for dynamic spawning
    if (use_ecs) {
        // ECS mode - skip legacy spawning, ECS handles it
    } else {
        // Legacy dynamic spawning removed - ECS handles spawning
    }
}

Player* GameManager::getPlayer() {
    // Player class removed - use ECS instead
    return nullptr;
}

void GameManager::update([[maybe_unused]] double deltaTime) {
    // Skip updates if not playing
    if (current_state != GameState::PLAYING) {
        return;
    }

    // Update ECS if enabled (PRIMARY)
    if (use_ecs && ecs_world) {
        ecs_world->update(deltaTime);
        ecs_world->removeDeadEntities();

        // Update deprecated player position variables from ECS
        auto player_entity = ecs_world->getPlayerEntity();
        if (player_entity) {
            auto* pos = player_entity->getComponent<ecs::PositionComponent>();
            auto* health = player_entity->getComponent<ecs::HealthComponent>();
            if (pos) {
                player_x = pos->position.x;
                player_y = pos->position.y;
            }
            if (health) {
                player_hp = health->hp;
                player_max_hp = health->max_hp;
            }
        }

        // ECS is now authoritative - skip legacy updates
        return;
    }

    // Only update legacy entities if ECS is disabled

    // Player class removed - player data comes from ECS
    
    // Update game systems
    // Future: Update animations, particles, etc.
    // For now, the turn system handles its own timing
}

void GameManager::processInput() {
    // Input is handled by FTXUI events for now
    // This is a placeholder for future input processing
}

void GameManager::updateFOV() {
    if (!map) return;

    Point playerPos;

    if (use_ecs && ecs_world) {
        // Use ECS player position
        playerPos = Point(player_x, player_y);  // These are synced from ECS
    } else {
        // Legacy mode removed - ECS is required
        return;
    }

    // Calculate FOV from player position
    FOV::calculate(*map, playerPos, Config::getInstance().getFOVRadius(), current_fov);
    
    // Check if player entered a new room
    const Room* new_room = map->getRoomAt(playerPos);
    if (new_room != current_room) {
        // Player entered a different room (or left a room)
        const Room* old_room = current_room;
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
    
    // Legacy entity visibility removed - ECS handles visibility

    // Update ECS FOV if enabled
    if (use_ecs && ecs_world) {
        ecs_world->updateFOV(current_fov);
    }
}

void GameManager::updateMonsters() {
    // ECS handles monster updates through AISystem in its update() call
    // This method is now a no-op as monster AI is handled by ECS
}

bool GameManager::saveGame(int slot) {
    if (!serializer) {
        LOG_ERROR("GameManager: Serializer not initialized");
        return false;
    }

    bool success = serializer->saveGame(slot);
    if (success) {
        message_log->addMessage("Game saved to slot " + std::to_string(slot));
    } else {
        message_log->addMessage("Failed to save game!");
    }
    return success;
}

bool GameManager::loadGame(int slot) {
    if (!serializer) {
        LOG_ERROR("GameManager: Serializer not initialized");
        return false;
    }

    bool success = serializer->loadGame(slot);
    if (success) {
        message_log->addMessage("Game loaded from slot " + std::to_string(slot));
        setState(GameState::PLAYING);
        updateFOV();  // Recalculate FOV after loading
    } else {
        message_log->addMessage("Failed to load game!");
    }
    return success;
}

void GameManager::initializeECS(bool migrate_existing) {
    // Create ECS world if not already created
    if (!ecs_world) {
        ecs_world = std::make_unique<ecs::GameWorld>(
            message_log.get(),
            map.get()
        );
    }

    // Initialize the ECS world
    ecs_world->initialize(migrate_existing);

    // Update FOV in ECS
    if (!current_fov.empty()) {
        ecs_world->updateFOV(current_fov);
    }

    // Enable ECS mode
    use_ecs = true;
}

