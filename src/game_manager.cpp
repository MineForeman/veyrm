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
#include "config.h"
#include "spawn_manager.h"
#include "monster_factory.h"
#include "monster_ai.h"
#include "monster.h"
#include "combat_system.h"
#include "log.h"
#include "item_manager.h"
#include "item_factory.h"
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
      entity_manager(std::make_unique<EntityManager>()),
      spawn_manager(std::make_unique<SpawnManager>(this)),
      monster_ai(std::make_unique<MonsterAI>()),
      combat_system(std::make_unique<CombatSystem>(message_log.get())),
      item_manager(std::make_unique<ItemManager>(map.get())),
      serializer(std::make_unique<GameSerializer>(this)),
      debug_mode(false) {
    
    // Initialize color scheme with auto-detection
    ColorScheme::setCurrentTheme(TerminalTheme::AUTO_DETECT);
    
    // Load monster data
    MonsterFactory::getInstance().loadFromFile(
        Config::getInstance().getDataFilePath("monsters.json")
    );

    // Load item data
    ItemFactory::getInstance().loadFromJson(
        Config::getInstance().getDataFilePath("items.json")
    );

    // Load ECS data
    ecs::DataLoader::getInstance().loadAllData(
        Config::getInstance().getDataDir()
    );

    // Initialize map with MapGenerator
    initializeMap(initial_map);

    // Initialize ECS system
    initializeECS(false);  // false = don't migrate existing entities yet

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
    std::shared_ptr<Player> player;
    if (use_ecs && ecs_world) {
        // Create player using ECS
        [[maybe_unused]] auto player_id = ecs_world->createPlayer(spawn.x, spawn.y);
        // Get legacy player for compatibility
        player = entity_manager->getPlayer();
    } else {
        // Create player using legacy system
        player = entity_manager->createPlayer(spawn.x, spawn.y);
    }
    
    // Spawn initial monsters after player placement
    spawn_manager->spawnInitialMonsters(*map, *entity_manager, player.get(), current_depth);
    
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

    // Spawn initial items
    if (item_manager) {
        item_manager->clear();  // Clear any existing items

        // Spawn items in rooms
        const auto& rooms = map->getRooms();
        if (!rooms.empty()) {
            // Spawn 5-10 random items
            int item_count = 5 + (rand() % 6);
            for (int i = 0; i < item_count; i++) {
                const Room& room = rooms[rand() % rooms.size()];

                // Find random position in room
                int x = room.x + 1 + (rand() % (room.width - 2));
                int y = room.y + 1 + (rand() % (room.height - 2));

                // Make sure position is walkable and not occupied
                if (map->isWalkable(x, y)) {
                    item_manager->spawnRandomItem(x, y, current_depth);
                }
            }

            // Spawn some gold piles
            int gold_count = 3 + (rand() % 4);
            for (int i = 0; i < gold_count; i++) {
                const Room& room = rooms[rand() % rooms.size()];
                int x = room.x + 1 + (rand() % (room.width - 2));
                int y = room.y + 1 + (rand() % (room.height - 2));

                if (map->isWalkable(x, y)) {
                    int amount = 10 + (rand() % 41);  // 10-50 gold
                    item_manager->spawnGold(x, y, amount);
                }
            }

            LOG_INFO("Spawned " + std::to_string(item_manager->getItemCount()) + " items");
        }
    }

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
    Player* player = getPlayer();
    if (player && spawn_manager) {
        spawn_manager->update(*map, *entity_manager, player, current_depth);
    }
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

    // Update ECS if enabled
    if (use_ecs && ecs_world) {
        ecs_world->update(deltaTime);
        ecs_world->removeDeadEntities();
    }

    // Update all entities (legacy or synced)
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
    
    // Update entity visibility based on FOV
    if (entity_manager) {
        entity_manager->updateEntityVisibility(current_fov);
    }

    // Update ECS FOV if enabled
    if (use_ecs && ecs_world) {
        ecs_world->updateFOV(current_fov);
    }
}

void GameManager::updateMonsters() {
    if (!entity_manager || !monster_ai) {
        return;
    }

    Player* player = getPlayer();
    if (!player) {
        return;
    }

    auto monsters = entity_manager->getMonsters();
    for (auto& monster_ptr : monsters) {
        if (!monster_ptr || !monster_ptr->canAct()) {
            continue;
        }

        Monster* monster = dynamic_cast<Monster*>(monster_ptr.get());
        if (!monster) {
            continue;
        }

        // Update AI state
        monster_ai->updateMonsterAI(*monster, *player, *map);

        // Get next move from AI
        Point next_pos = monster_ai->getNextMove(*monster, *player, *map);

        LOG_AI("Monster " + monster->name + " AI suggests move from (" +
               std::to_string(monster->x) + "," + std::to_string(monster->y) + ") to (" +
               std::to_string(next_pos.x) + "," + std::to_string(next_pos.y) + ")");

        // Check if the move is valid (not blocked by another entity)
        if (next_pos != monster->getPosition()) {
            bool blocked = false;

            // Check if player is at target position (attack)
            if (next_pos == player->getPosition()) {
                LOG_AI("Monster " + monster->name + " at (" + std::to_string(monster->x) + "," + std::to_string(monster->y) + ") attacking player at (" + std::to_string(player->x) + "," + std::to_string(player->y) + ")");

                // Use CombatSystem for attack resolution
                auto result = combat_system->processAttack(*monster, *player);

                LOG_AI("Attack result: hit=" + std::string(result.hit ? "true" : "false") +
                      ", damage=" + std::to_string(result.damage) +
                      ", critical=" + std::string(result.critical ? "true" : "false") +
                      ", fatal=" + std::string(result.fatal ? "true" : "false"));

                // Handle player death
                if (result.fatal) {
                    LOG_ERROR("=== PLAYER DEATH ===");
                    LOG_ERROR("Player has been killed by " + monster->name + "!");
                    LOG_ERROR("Final player HP: " + std::to_string(player->hp));
                    LOG_ERROR("Game Over - Setting state to DEATH");
                    LOG_ERROR("=== GAME OVER ===");
                    setState(GameState::DEATH);
                }

                continue;
            }

            // Check if another monster is blocking
            for (const auto& other : monsters) {
                if (other.get() != monster && other->getPosition() == next_pos) {
                    blocked = true;
                    break;
                }
            }

            // Move if not blocked
            if (!blocked) {
                LOG_MOVEMENT("Monster " + monster->name + " moving to (" + std::to_string(next_pos.x) + "," + std::to_string(next_pos.y) + ")");
                monster->moveTo(next_pos.x, next_pos.y);
            } else {
                LOG_AI("Monster " + monster->name + " movement blocked");
            }
        } else {
            LOG_AI("Monster " + monster->name + " AI returned current position - no movement");
        }
    }
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
            entity_manager.get(),
            combat_system.get(),
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

    // Log the transition
    if (message_log) {
        message_log->addSystemMessage("ECS mode enabled");
    }
}

