#include "game_serializer.h"
#include "game_state.h"
#include "map.h"
#include "message_log.h"
#include "log.h"
#include "turn_manager.h"
#include "services/cloud_save_service.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>

GameSerializer::GameSerializer(GameManager* gm)
    : game_manager(gm)
    , save_directory("saves")
    , session_start_time(std::chrono::steady_clock::now()) {
    ensureSaveDirectoryExists();
}

bool GameSerializer::ensureSaveDirectoryExists() const {
    try {
        if (!std::filesystem::exists(save_directory)) {
            std::filesystem::create_directories(save_directory);
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create save directory: " + std::string(e.what()));
        return false;
    }
}

std::string GameSerializer::getSlotFilename(int slot) const {
    if (slot < -3 || slot > 9) {
        return "";
    }

    if (slot < 0) {
        // Auto-save slots
        return "autosave_" + std::to_string(-slot) + ".sav";
    } else {
        // Regular save slots
        return "save_" + std::to_string(slot) + ".sav";
    }
}

std::string GameSerializer::getSavePath(const std::string& filename) const {
    return (save_directory / filename).string();
}

std::string GameSerializer::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int GameSerializer::calculatePlayTime() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - session_start_time);
    return static_cast<int>(duration.count());
}

json GameSerializer::serializeMetadata() const {
    json meta;
    meta["player_name"] = "Hero";  // TODO: Get actual player name when implemented
    meta["level"] = 1;  // TODO: Get dungeon level from map
    meta["turn_count"] = 0;  // TODO: Get from turn manager when accessible
    meta["play_time"] = calculatePlayTime();
    return meta;
}

json GameSerializer::serializeGameState() const {
    json state;
    state["current_state"] = static_cast<int>(game_manager->getState());
    state["turn"] = 0;  // TODO: Get from turn manager when accessible
    state["current_depth"] = game_manager->getCurrentDepth();
    state["map_type"] = static_cast<int>(game_manager->getCurrentMapType());
    state["map_seed"] = game_manager->getCurrentMapSeed();
    return state;
}

json GameSerializer::serializeMap() const {
    Map* map = game_manager->getMap();
    if (!map) return json();

    json map_data;

    // We don't need to save tiles since we regenerate from seed!
    // Only save explored areas for fog of war

    // Save only the explored tiles as a list of coordinates
    json explored = json::array();
    for (int y = 0; y < map->getHeight(); ++y) {
        for (int x = 0; x < map->getWidth(); ++x) {
            if (map->isExplored(x, y)) {
                json coord = json::array();
                coord.push_back(x);
                coord.push_back(y);
                explored.push_back(coord);
            }
        }
    }
    map_data["explored"] = explored;

    // Save map dimensions for validation (optional, could regenerate)
    map_data["width"] = map->getWidth();
    map_data["height"] = map->getHeight();

    return map_data;
}

json GameSerializer::serializePlayer() const {
    // Player class removed - use ECS data from game_manager
    json player_data;

    player_data["x"] = game_manager->player_x;
    player_data["y"] = game_manager->player_y;
    player_data["name"] = "Player";

    player_data["hp"] = game_manager->player_hp;
    player_data["max_hp"] = game_manager->player_max_hp;
    player_data["attack"] = 6;  // Default values for now
    player_data["defense"] = 2;
    player_data["gold"] = 0;

    // Inventory will be serialized from ECS

    return player_data;
}

json GameSerializer::serializeEntities() const {
    // EntityManager removed - using ECS
    json entities = json::array();

    // ECS entities will be serialized here

    return entities;
}

json GameSerializer::serializeItems() const {
    json items = json::array();
    // Items will be serialized from ECS world

    return items;
}

json GameSerializer::serializeMessageLog() const {
    MessageLog* log = game_manager->getMessageLog();
    if (!log) return json::array();

    json messages = json::array();

    // Get recent messages (last 100)
    const auto& all_messages = log->getMessages();
    int start = std::max(0, static_cast<int>(all_messages.size()) - 100);

    for (size_t i = start; i < all_messages.size(); ++i) {
        json msg;
        msg["text"] = all_messages[i];
        // msg["color"] = all_messages[i].color;  // MessageLog stores strings, not colored messages
        messages.push_back(msg);
    }

    return messages;
}

bool GameSerializer::saveGame(const std::string& filename) {
    try {
        json save_data;

        // Version and timestamp
        save_data["version"] = SAVE_VERSION;
        save_data["game_version"] = GAME_VERSION;
        save_data["timestamp"] = getCurrentTimestamp();

        // Metadata
        save_data["metadata"] = serializeMetadata();

        // Game state
        save_data["game_state"] = serializeGameState();

        // Map
        save_data["map"] = serializeMap();

        // Player
        save_data["player"] = serializePlayer();

        // Entities
        save_data["entities"] = serializeEntities();

        // Items
        save_data["items"] = serializeItems();

        // Message log
        save_data["message_log"] = serializeMessageLog();

        // Write to file
        std::string path = getSavePath(filename);
        std::ofstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open save file: " + path);
            return false;
        }

        file << save_data.dump(2);  // Pretty print with 2-space indentation
        file.close();

        LOG_INFO("Game saved to: " + path);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save game: " + std::string(e.what()));
        return false;
    }
}

bool GameSerializer::saveGame(int slot) {
    if (slot < 0 || slot > 9) {
        LOG_ERROR("Invalid save slot: " + std::to_string(slot));
        return false;
    }

    std::string filename = getSlotFilename(slot);
    return saveGame(filename);
}

bool GameSerializer::autoSave() {
    // Rotate auto-save slots
    rotateAutoSaves();

    std::string filename = getSlotFilename(current_auto_save_slot);
    bool success = saveGame(filename);

    if (success) {
        LOG_INFO("Auto-saved to slot " + std::to_string(-current_auto_save_slot));
    }

    return success;
}

void GameSerializer::rotateAutoSaves() {
    // Cycle through auto-save slots -1, -2, -3
    current_auto_save_slot--;
    if (current_auto_save_slot < -3) {
        current_auto_save_slot = -1;
    }
}

int GameSerializer::getCurrentAutoSaveSlot() const {
    return current_auto_save_slot;
}

bool GameSerializer::loadGame(const std::string& filename) {
    try {
        std::string path = getSavePath(filename);
        std::ifstream file(path);

        if (!file.is_open()) {
            LOG_ERROR("Failed to open save file: " + path);
            return false;
        }

        json save_data;
        file >> save_data;
        file.close();

        // Validate version
        if (!save_data.contains("version")) {
            LOG_ERROR("Save file missing version information");
            return false;
        }

        // TODO: Implement version migration if needed

        // Deserialize components
        if (!deserializeGameState(save_data["game_state"])) {
            LOG_ERROR("Failed to deserialize game state");
            return false;
        }

        if (!deserializeMap(save_data["map"])) {
            LOG_ERROR("Failed to deserialize map");
            return false;
        }

        if (!deserializePlayer(save_data["player"])) {
            LOG_ERROR("Failed to deserialize player");
            return false;
        }

        if (!deserializeEntities(save_data["entities"])) {
            LOG_ERROR("Failed to deserialize entities");
            return false;
        }

        if (!deserializeItems(save_data["items"])) {
            LOG_ERROR("Failed to deserialize items");
            return false;
        }

        if (!deserializeMessageLog(save_data["message_log"])) {
            LOG_ERROR("Failed to deserialize message log");
            return false;
        }

        LOG_INFO("Game loaded from: " + path);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load game: " + std::string(e.what()));
        return false;
    }
}

bool GameSerializer::loadGame(int slot) {
    if (slot < -3 || slot > 9) {
        LOG_ERROR("Invalid save slot: " + std::to_string(slot));
        return false;
    }

    std::string filename = getSlotFilename(slot);
    return loadGame(filename);
}

bool GameSerializer::deserializeGameState(const json& data) {
    if (data.is_null()) return false;

    try {
        if (data.contains("current_state")) {
            game_manager->setState(static_cast<GameState>(data["current_state"].get<int>()));
        }

        if (data.contains("turn")) {
            // TODO: Set turn count when method is available
        }

        if (data.contains("current_depth")) {
            game_manager->setCurrentDepth(data["current_depth"]);
        }

        if (data.contains("map_type")) {
            game_manager->setCurrentMapType(static_cast<MapType>(data["map_type"].get<int>()));
        }

        if (data.contains("map_seed")) {
            game_manager->setCurrentMapSeed(data["map_seed"]);
        }

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to deserialize game state: " + std::string(e.what()));
        return false;
    }
}

bool GameSerializer::deserializeMap(const json& data) {
    if (data.is_null()) {
        LOG_ERROR("Map data is null");
        return false;
    }

    try {
        // Get map type and seed from game state (already loaded)
        MapType map_type = game_manager->getCurrentMapType();
        unsigned int seed = game_manager->getCurrentMapSeed();

        LOG_INFO("Regenerating map: type=" + std::to_string(static_cast<int>(map_type)) +
                 ", seed=" + std::to_string(seed));

        // Regenerate the map using the saved seed
        Map* map = game_manager->getMap();
        if (!map) {
            LOG_ERROR("No map available");
            return false;
        }

        // Clear and regenerate
        map->fill(TileType::VOID);
        MapGenerator::generate(*map, map_type, seed);

        // Restore explored tiles if saved
        if (data.contains("explored") && data["explored"].is_array()) {
            const json& explored = data["explored"];
            for (const auto& coord : explored) {
                if (coord.is_array() && coord.size() >= 2) {
                    int x = coord[0];
                    int y = coord[1];
                    if (x >= 0 && x < map->getWidth() && y >= 0 && y < map->getHeight()) {
                        map->setExplored(x, y, true);
                    }
                }
            }
            LOG_INFO("Restored " + std::to_string(explored.size()) + " explored tiles");
        }

        // Handle old save format for compatibility
        else if (data.contains("visible") && data["visible"].is_array()) {
            const json& visible = data["visible"];
            int explored_count = 0;
            for (size_t y = 0; y < static_cast<size_t>(map->getHeight()) && y < visible.size(); ++y) {
                if (!visible[y].is_array()) continue;
                const json& row = visible[y];
                for (size_t x = 0; x < static_cast<size_t>(map->getWidth()) && x < row.size(); ++x) {
                    if (row[x].is_boolean() && row[x].get<bool>()) {
                        map->setExplored(static_cast<int>(x), static_cast<int>(y), true);
                        explored_count++;
                    }
                }
            }
            LOG_INFO("Restored " + std::to_string(explored_count) + " explored tiles from old format");
        }

        LOG_INFO("Map regenerated successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to deserialize map: " + std::string(e.what()));
        return false;
    }
}

bool GameSerializer::deserializePlayer(const json& data) {
    if (data.is_null()) {
        LOG_ERROR("Player data is null");
        return false;
    }

    try {
        // EntityManager removed - deserialize to game_manager variables
        // These will be used when creating the ECS player
        game_manager->player_x = data.value("x", 1);
        game_manager->player_y = data.value("y", 1);

        if (data.contains("hp")) {
            game_manager->player_hp = data["hp"];
        }
        if (data.contains("max_hp")) {
            game_manager->player_max_hp = data["max_hp"];
        }

        // TODO: Store other player data for ECS creation
        // attack, defense, gold, inventory

        LOG_INFO("Player data deserialized: pos=(" + std::to_string(game_manager->player_x) + "," +
                 std::to_string(game_manager->player_y) + "), hp=" + std::to_string(game_manager->player_hp) + "/" +
                 std::to_string(game_manager->player_max_hp));

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to deserialize player: " + std::string(e.what()));
        return false;
    }
}

bool GameSerializer::deserializeEntities(const json& data) {
    if (data.is_null() || !data.is_array()) return false;

    try {
        // EntityManager removed - ECS handles entities

        // Recreate entities
        for (const auto& entity_data : data) {
            if (entity_data["type"] == "monster") {
                std::string species = entity_data["species"];
                // int x = entity_data["x"];
                // int y = entity_data["y"];

                // TODO: Create monster through factory with saved HP
                // For now, just spawn new monsters
                // MonsterFactory::createMonster(species, x, y);
            }
        }

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to deserialize entities: " + std::string(e.what()));
        return false;
    }
}

bool GameSerializer::deserializeItems(const json& data) {
    if (data.is_null() || !data.is_array()) return false;

    try {
        // ItemManager removed - using ECS
        // ItemManager* im = game_manager->getItemManager();
        // if (!im) return false;

        // TODO: Deserialize items into ECS world

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to deserialize items: " + std::string(e.what()));
        return false;
    }
}

bool GameSerializer::deserializeMessageLog(const json& data) {
    if (data.is_null() || !data.is_array()) return true;  // Empty log is valid

    try {
        MessageLog* log = game_manager->getMessageLog();
        if (!log) return false;

        // Clear existing messages
        log->clear();

        // Restore messages
        for (const auto& msg_data : data) {
            std::string text = msg_data["text"];
            // MessageLog doesn't support colored messages via public API
            log->addMessage(text);
        }

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to deserialize message log: " + std::string(e.what()));
        return false;
    }
}

bool GameSerializer::deserializeMetadata(const json& /*data*/) {
    // Metadata is for display only, no need to deserialize
    return true;
}

std::vector<SaveInfo> GameSerializer::getSaveFiles() const {
    std::vector<SaveInfo> saves;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(save_directory)) {
            if (entry.path().extension() == ".sav") {
                SaveInfo info = getSaveInfo(entry.path().filename().string());
                if (info.is_valid) {
                    saves.push_back(info);
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to list save files: " + std::string(e.what()));
    }

    return saves;
}

SaveInfo GameSerializer::getSaveInfo(const std::string& filename) const {
    SaveInfo info;
    info.filename = filename;
    info.exists = false;
    info.is_valid = false;

    try {
        std::string path = getSavePath(filename);

        // Check if file exists
        if (!std::filesystem::exists(path)) {
            return info;
        }

        info.exists = true;

        std::ifstream file(path);
        if (!file.is_open()) {
            return info;
        }

        json save_data;
        file >> save_data;
        file.close();

        // Parse basic info
        info = SaveInfo::fromJson(save_data);
        info.filename = filename;
        info.exists = true;

        // Extract additional fields
        if (save_data.contains("game_state")) {
            const json& gs = save_data["game_state"];
            if (gs.contains("current_depth")) info.depth = gs["current_depth"];
            if (gs.contains("turn_count")) info.turn_count = gs["turn_count"];
        }

        if (save_data.contains("player")) {
            const json& player = save_data["player"];
            if (player.contains("name")) info.player_name = player["name"];
            if (player.contains("hp")) info.player_hp = player["hp"];
            if (player.contains("max_hp")) info.player_max_hp = player["max_hp"];
        }

        if (save_data.contains("timestamp")) {
            info.timestamp = save_data["timestamp"];
        }

        info.is_valid = true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to read save info: " + std::string(e.what()));
    }

    return info;
}

bool GameSerializer::validateSave(const std::string& filename) const {
    try {
        SaveInfo info = getSaveInfo(filename);
        return info.is_valid;
    } catch (...) {
        return false;
    }
}

bool GameSerializer::saveExists(int slot) const {
    std::string filename = getSlotFilename(slot);
    std::string path = getSavePath(filename);
    return std::filesystem::exists(path);
}

bool GameSerializer::deleteSave(const std::string& filename) {
    try {
        std::string path = getSavePath(filename);
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
            LOG_INFO("Deleted save file: " + filename);
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete save: " + std::string(e.what()));
        return false;
    }
}

bool GameSerializer::deleteSave(int slot) {
    std::string filename = getSlotFilename(slot);
    return deleteSave(filename);
}

// === Cloud Save Integration ===

bool GameSerializer::saveGameWithCloud(int slot, bool upload_to_cloud) {
    // First save locally
    if (!saveGame(slot)) {
        LOG_ERROR("Failed to save game locally");
        return false;
    }

    // Then upload to cloud if enabled and requested
    if (upload_to_cloud && cloud_service && cloud_service->isAuthenticated()) {
        if (!cloud_service->saveToCloud(slot, true)) {
            LOG_INFO("Failed to upload save to cloud, but local save succeeded");
            // Don't return false since local save worked
        } else {
            LOG_INFO("Save uploaded to cloud successfully");
        }
    }

    return true;
}

bool GameSerializer::loadGameWithCloud(int slot, bool prefer_cloud) {
    if (cloud_service && cloud_service->isAuthenticated()) {
        // Try to load from cloud with preference setting
        if (cloud_service->loadFromCloud(slot, prefer_cloud)) {
            // Cloud load handles both cloud and local versions
            return loadGame(slot);
        }
    }

    // Fall back to local load
    return loadGame(slot);
}

bool GameSerializer::syncWithCloud() {
    if (!cloud_service) {
        LOG_ERROR("Cloud service not initialized");
        return false;
    }

    if (!cloud_service->isAuthenticated()) {
        LOG_INFO("Not authenticated - cloud sync requires login");
        return false;
    }

    auto result = cloud_service->syncAllSaves();
    if (result.success) {
        LOG_INFO("Cloud sync successful - " +
                 std::to_string(result.saves_uploaded) + " uploaded, " +
                 std::to_string(result.saves_downloaded) + " downloaded");
    } else {
        LOG_ERROR("Cloud sync failed with " +
                 std::to_string(result.errors.size()) + " errors");
    }

    return result.success;
}

SyncStatus GameSerializer::getCloudSyncStatus(int slot) {
    if (!cloud_service) {
        return SyncStatus::OFFLINE;
    }

    return cloud_service->getSyncStatus(slot);
}