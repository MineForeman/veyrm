#include "game_state.h"
#include "input_handler.h"
#include "turn_manager.h"
#include "message_log.h"
#include "frame_stats.h"
#include "map.h"
#include "room.h"
#include "color_scheme.h"
#include "map_generator.h"
#include "map_validator.h"
#include "fov.h"
#include "map_memory.h"
#include "config.h"
#include "log.h"
#include "ecs/position_component.h"
#include "ecs/data_loader.h"
#include "ecs/health_component.h"
#include "ecs/renderable_component.h"
#include "ecs/game_world.h"
#include "db/database_manager.h"
#include "db/save_game_repository.h"
#include "db/game_entity_repository.h"
#include <boost/json.hpp>
#include <random>

GameManager::GameManager(MapType initial_map) 
    : current_state(GameState::MENU),
      previous_state(GameState::MENU),
      input_handler(std::make_unique<InputHandler>()),
      turn_manager(std::make_unique<TurnManager>(this)),
      message_log(std::make_unique<MessageLog>()),
      frame_stats(std::make_unique<FrameStats>()),
      map(std::make_unique<Map>(Config::getInstance().getMapWidth(), Config::getInstance().getMapHeight())),
      debug_mode(false) {

    // Initialize color scheme with auto-detection
    ColorScheme::setCurrentTheme(TerminalTheme::AUTO_DETECT);

    // Initialize database for auto-save
    initializeDatabase();
    
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

    // Update stairs based on current depth
    MapGenerator::updateStairsForDepth(*map, current_depth);

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
    
    // Clear existing entities from previous level
    if (ecs_world) {
        ecs_world->clearEntities();
        LOG_INFO("Cleared all entities for level transition");
    }

    // Set player spawn point
    Point spawn = MapGenerator::getDefaultSpawnPoint(type);
    
    // Verify spawn point is walkable
    if (!Map::getTileProperties(map->getTile(spawn.x, spawn.y)).walkable) {
        // Fallback to finding any safe spawn point
        spawn = MapGenerator::findSafeSpawnPoint(*map);
        message_log->addSystemMessage("Using fallback spawn point");
    }
    
    // Create player entity at spawn point
    if (ecs_world) {
        // Create player using ECS with authentication info if available
        [[maybe_unused]] auto player_id = ecs_world->createPlayer(
            spawn.x, spawn.y,
            auth_user_id,
            auth_session_token,
            auth_player_name
        );

        // Spawn monsters and items in rooms
        spawnEntities();
    } else {
        LOG_ERROR("ECS world not available");
    }

    // Sync player position from ECS for compatibility
    if (ecs_world) {
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
        player_x = 0;
        player_y = 0;
        player_hp = 0;
        player_max_hp = 0;
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
    
    // After player acts, check for dynamic spawning
    // ECS handles spawning
}

void* GameManager::getPlayer() {
    if (ecs_world) {
        return ecs_world->getPlayerEntity();
    }
    return nullptr;
}

void GameManager::update([[maybe_unused]] double deltaTime) {
    // Skip updates if not playing
    if (current_state != GameState::PLAYING) {
        return;
    }

    // Update ECS systems
    if (ecs_world) {
        // Don't update AI every frame - only when updateMonsters() is called
        // ecs_world->update(deltaTime);  // DISABLED - AI should be turn-based

        // Update only render system for visual display
        ecs_world->updateRenderSystem();

        ecs_world->removeDeadEntities();

        // Check for player death
        if (ecs_world->isPlayerDead()) {
            // Set death cause and turn
            setDeathCause("combat");  // Default cause
            if (turn_manager) {
                setDeathTurn(turn_manager->getCurrentTurn());
            }
            setState(GameState::DEATH);
            return;
        }

        // Sync player position from ECS
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

        // ECS is authoritative
        return;
    }

    // Player data comes from ECS
    
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

    if (ecs_world) {
        // Use ECS player position
        playerPos = Point(player_x, player_y);  // These are synced from ECS
    } else {
        return;  // ECS is required
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

    // Update ECS FOV
    if (ecs_world) {
        ecs_world->updateFOV(current_fov);
    }
}

void GameManager::updateMonsters() {
    // Update ECS AI system for one turn
    if (ecs_world) {
        // Only update the AI system, not the entire world
        ecs_world->processMonsterAI();
    }
}

bool GameManager::autoSave() {
    try {
        if (!ecs_world) {
            LOG_WARN("Auto-save not available - missing ECS world");
            return false;
        }

        // Create entity repository
        db::GameEntityRepository entity_repo;

        // Serialize ECS world to entity data
        auto& world = ecs_world->getWorld();
        int user_id = auth_user_id > 0 ? auth_user_id : 1;
        int save_slot = -1; // Auto-save slot

        auto entities = db::GameEntityRepository::serializeWorld(world, user_id, save_slot);

        // Create save metadata
        db::GameSaveData save_data;
        save_data.user_id = user_id;
        save_data.save_slot = save_slot;
        save_data.character_name = auth_player_name.empty() ? "Auto-saved Hero" : auth_player_name;
        save_data.character_level = 1; // TODO: Get from ECS stats component
        save_data.map_level = current_depth;
        save_data.play_time_seconds = 0; // TODO: Calculate actual play time
        save_data.game_version = "1.0.0";
        save_data.save_version = "1.0";
        save_data.device_id = "local_device";
        save_data.device_name = "Local Game Client";
        save_data.map_width = map->getWidth();
        save_data.map_height = map->getHeight();
        save_data.world_seed = 0; // TODO: Add getSeed() method to Map class

        // Save complete game state to PostgreSQL
        if (entity_repo.saveGameState(save_data, entities)) {
            LOG_INFO("Auto-save completed successfully: " + std::to_string(entities.size()) + " entities saved to PostgreSQL");
            if (message_log) {
                message_log->addMessage("Game auto-saved (" + std::to_string(entities.size()) + " entities)");
            }
            return true;
        } else {
            LOG_ERROR("Auto-save failed for user " + std::to_string(user_id));
            if (message_log) {
                message_log->addMessage("Auto-save failed");
            }
            return false;
        }

    } catch (const std::exception& e) {
        std::string error_msg = "Auto-save failed: " + std::string(e.what());
        if (message_log) {
            message_log->addMessage(error_msg);
        }
        LOG_ERROR(error_msg);
        return false;
    }
}

bool GameManager::autoRestore() {
    try {
        if (!ecs_world) {
            LOG_WARN("Auto-restore not available - missing ECS world");
            return false;
        }

        // Create entity repository
        db::GameEntityRepository entity_repo;

        // Try to load from auto-save slot (-1)
        int user_id = auth_user_id > 0 ? auth_user_id : 1;
        int save_slot = -1;

        auto game_state = entity_repo.loadGameState(user_id, save_slot);
        if (!game_state.has_value()) {
            LOG_INFO("No auto-save found in PostgreSQL");
            if (message_log) {
                message_log->addMessage("No saved game found");
            }
            return false;
        }

        auto& [save_data, entities] = game_state.value();
        LOG_INFO("DEBUG: Loaded " + std::to_string(entities.size()) + " entities from database");

        // Clear existing entities
        LOG_INFO("DEBUG: Clearing existing ECS entities");
        ecs_world->clearEntities();

        // Restore entities to ECS world with proper player tracking
        LOG_INFO("DEBUG: Starting deserialization of " + std::to_string(entities.size()) + " entities");
        int restored_count = db::GameEntityRepository::deserializeWorld(entities, *ecs_world);
        LOG_INFO("DEBUG: Deserialization complete, restored " + std::to_string(restored_count) + " entities");
        LOG_INFO("DEBUG: Player entity ID after restore: " + std::to_string(ecs_world->getPlayerID()));

        // Log restore results
        LOG_INFO("Auto-restore completed: " + std::to_string(restored_count) + "/" +
                std::to_string(entities.size()) + " entities restored from PostgreSQL");
        LOG_INFO("Restored map size: " + std::to_string(save_data.map_width) + "x" + std::to_string(save_data.map_height));

        if (message_log) {
            message_log->addMessage("Game state restored from PostgreSQL");
            message_log->addMessage("Character: " + save_data.character_name);
            message_log->addMessage("Entities restored: " + std::to_string(restored_count));
        }

        return true;

    } catch (const std::exception& e) {
        std::string error_msg = "Auto-restore failed: " + std::string(e.what());
        if (message_log) {
            message_log->addMessage(error_msg);
        }
        LOG_ERROR(error_msg);
        return false;
    }
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

void GameManager::spawnEntities() {
    LOG_SPAWN("spawnEntities() called");
    if (!map || !ecs_world) {
        LOG_SPAWN("Early return - missing map or ecs_world");
        return;
    }

    // Check if data is loaded
    auto& data_loader = ecs::DataLoader::getInstance();
    if (!data_loader.isLoaded()) {
        LOG_SPAWN("DataLoader not loaded - attempting to load data");
        if (!data_loader.loadAllData("data")) {
            LOG_SPAWN("Failed to load data - cannot spawn entities");
            return;
        }
    }

    // Use the map seed for consistent spawning
    std::mt19937 rng(current_map_seed != 0 ? current_map_seed : std::random_device{}());

    const auto& rooms = map->getRooms();
    LOG_SPAWN("Found " + std::to_string(rooms.size()) + " rooms for spawning");
    if (rooms.empty()) {
        LOG_SPAWN("No rooms found for spawning");
        return;
    }

    // Get current depth (default to 1 for now)
    int depth = getCurrentDepth();

    // Monster spawn tables by depth
    std::vector<std::pair<std::string, int>> depth_1_monsters = {
        {"gutter_rat", 40},
        {"cave_spider", 30},
        {"goblin", 20},
        {"zombie", 10}
    };

    std::vector<std::pair<std::string, int>> depth_2_monsters = {
        {"gutter_rat", 20},
        {"cave_spider", 25},
        {"goblin", 30},
        {"zombie", 15},
        {"orc_rookling", 10}
    };

    // Item spawn tables - using IDs from items.json
    std::vector<std::pair<std::string, int>> common_items = {
        {"potion_minor", 40},
        {"food_ration", 20},
        {"gold", 30},
        {"scroll_identify", 15},
        {"dagger", 10}
    };

    // Select spawn table based on depth
    auto& monster_table = (depth <= 1) ? depth_1_monsters : depth_2_monsters;

    // Calculate total weights
    int total_monster_weight = 0;
    for (const auto& [type, weight] : monster_table) {
        total_monster_weight += weight;
    }

    int total_item_weight = 0;
    for (const auto& [type, weight] : common_items) {
        total_item_weight += weight;
    }

    // Spawn monsters in rooms (skip first room where player spawns)
    LOG_SPAWN("Starting spawn loop for " + std::to_string(rooms.size() - 1) + " rooms");
    for (size_t i = 1; i < rooms.size(); ++i) {
        const Room& room = rooms[i];
        LOG_SPAWN("Processing room " + std::to_string(i) + " at (" + std::to_string(room.x) + "," + std::to_string(room.y) + ")");

        // Determine number of monsters for this room (1-3 based on room size)
        int room_area = room.width * room.height;
        int max_monsters = std::min(3, std::max(1, room_area / 20));
        std::uniform_int_distribution<> monster_count_dist(1, max_monsters);
        int monster_count = monster_count_dist(rng);

        // Spawn monsters in this room
        for (int j = 0; j < monster_count; ++j) {
            // Select monster type based on weighted probability
            std::uniform_int_distribution<> weight_dist(0, total_monster_weight - 1);
            int roll = weight_dist(rng);

            std::string monster_type;
            int cumulative = 0;
            for (const auto& [type, weight] : monster_table) {
                cumulative += weight;
                if (roll < cumulative) {
                    monster_type = type;
                    break;
                }
            }

            // Find random position in room
            std::uniform_int_distribution<> x_dist(room.x + 1, room.x + room.width - 2);
            std::uniform_int_distribution<> y_dist(room.y + 1, room.y + room.height - 2);

            int attempts = 10;
            while (attempts-- > 0) {
                int x = x_dist(rng);
                int y = y_dist(rng);

                // Check if position is walkable and not occupied
                if (map->isWalkable(x, y)) {
                    ecs_world->createMonster(monster_type, x, y);
                    break;
                }
            }
        }

        // Spawn items (100% chance for testing)
        std::uniform_int_distribution<> item_chance(1, 100);
        if (item_chance(rng) <= 100) {  // Guaranteed spawn for testing
            // Select item type
            std::uniform_int_distribution<> item_weight_dist(0, total_item_weight - 1);
            int roll = item_weight_dist(rng);

            std::string item_type;
            int cumulative = 0;
            for (const auto& [type, weight] : common_items) {
                cumulative += weight;
                if (roll < cumulative) {
                    item_type = type;
                    break;
                }
            }

            // Find random position in room
            std::uniform_int_distribution<> x_dist(room.x + 1, room.x + room.width - 2);
            std::uniform_int_distribution<> y_dist(room.y + 1, room.y + room.height - 2);

            int attempts = 10;
            while (attempts-- > 0) {
                int x = x_dist(rng);
                int y = y_dist(rng);

                if (map->isWalkable(x, y)) {
                    ecs_world->createItem(item_type, x, y);
                    break;
                }
            }
        }
    }

    // Log spawn summary
    std::string spawn_msg = "Spawned monsters and items in " + std::to_string(rooms.size() - 1) + " rooms";
    message_log->addSystemMessage(spawn_msg);
    LOG_SPAWN(spawn_msg);
}

unsigned int GameManager::getSeedForDepth(int depth) const {
    // Use a deterministic hash function to generate depth-specific seeds
    // This ensures the same base seed always generates the same sequence of levels
    const unsigned int DEPTH_MULTIPLIER = 0x9E3779B9; // Golden ratio based hash constant
    const unsigned int BASE_OFFSET = 0x85EBCA6B;      // Large prime for mixing

    unsigned int base = (current_map_seed == 0) ? 12345 : current_map_seed;

    // Combine base seed with depth using multiplication and XOR for good distribution
    unsigned int depth_seed = base;
    depth_seed ^= (depth * DEPTH_MULTIPLIER) + BASE_OFFSET;
    depth_seed ^= depth_seed >> 16;  // Mix upper and lower bits
    depth_seed *= 0x45D9F3B;        // Multiply by another prime
    depth_seed ^= depth_seed >> 16;  // Mix again

    return depth_seed;
}

void GameManager::initializeDatabase() {
    try {
        // Initialize database connection for auto-save
        db::DatabaseConfig config;
        config.host = "localhost";
        config.port = 5432;
        config.database = "veyrm_db";
        config.username = "veyrm_admin";
        config.password = "changeme_to_secure_password";

        try {
            db::DatabaseManager::getInstance().initialize(config);
            if (db::DatabaseManager::getInstance().isInitialized()) {
                save_repository = std::make_unique<db::SaveGameRepository>(db::DatabaseManager::getInstance());
                LOG_INFO("Database initialized successfully for auto-save");
            } else {
                LOG_WARN("Database initialization failed - auto-save will be disabled");
            }
        } catch (const std::exception& e) {
            LOG_WARN("Database initialization error: " + std::string(e.what()) + " - auto-save will be disabled");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Database initialization failed: " + std::string(e.what()));
        save_repository.reset();
    }
}

