/**
 * @file cloud_save_service.cpp
 * @brief Implementation of cloud save synchronization service
 * @author Veyrm Team
 * @date 2025
 */

#include "services/cloud_save_service.h"
#include "db/save_game_repository.h"
#include "auth/authentication_service.h"
#include "game_serializer.h"
#include "ecs/game_world.h"
#include "ecs/save_load_system.h"
#include "ecs/entity.h"
#include "ecs/player_component.h"
#include "ecs/position_component.h"
#include "ecs/health_component.h"
#include "ecs/stats_component.h"
#include "message_log.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>

#ifdef __APPLE__
    #include <sys/types.h>
    #include <sys/sysctl.h>
#elif __linux__
    #include <unistd.h>
    #include <sys/types.h>
#elif _WIN32
    #include <windows.h>
#endif

using namespace std::chrono_literals;

CloudSaveService::CloudSaveService(db::SaveGameRepository* save_repo,
                                   GameSerializer* game_serializer,
                                   AuthenticationService* auth_service,
                                   ecs::GameWorld* ecs_world)
    : save_repository(save_repo)
    , game_serializer(game_serializer)
    , auth_service(auth_service)
    , ecs_world(ecs_world)
    , ecs_save_system(std::make_unique<ecs::SaveLoadSystem>())
{
}

CloudSaveService::~CloudSaveService() {
    disableAutoSync();
}

// === Core Save Operations ===

bool CloudSaveService::saveToCloud(int slot, bool /*force_upload*/) {
    if (!isAuthenticated()) {
        last_error = "Not authenticated - cloud save requires login";
        return false;
    }

    try {
        // First save locally using GameSerializer
        if (!game_serializer->saveGame(slot)) {
            last_error = "Failed to save game locally";
            return false;
        }

        // Then upload to cloud if online
        if (isOnline()) {
            return uploadLocalSave(slot);
        } else {
            // Mark as pending upload for when we're back online
            updateStatusCache(slot, SyncStatus::PENDING_UPLOAD);
            return true; // Local save succeeded
        }
    } catch (const std::exception& e) {
        last_error = "Save to cloud failed: " + std::string(e.what());
        return false;
    }
}

bool CloudSaveService::loadFromCloud(int slot, bool prefer_cloud) {
    if (!isAuthenticated()) {
        // Fall back to local load
        return game_serializer->loadGame(slot);
    }

    try {
        auto conflict = detectConflict(slot);
        if (conflict.has_value()) {
            // Handle conflict
            if (prefer_cloud) {
                return downloadCloudSave(slot);
            } else {
                return game_serializer->loadGame(slot);
            }
        }

        // No conflict - get latest version
        auto status = getSyncStatus(slot);
        if (status == SyncStatus::PENDING_DOWNLOAD ||
            (status == SyncStatus::SYNCED && isOnline())) {
            return downloadCloudSave(slot);
        } else {
            return game_serializer->loadGame(slot);
        }
    } catch (const std::exception& e) {
        last_error = "Load from cloud failed: " + std::string(e.what());
        return game_serializer->loadGame(slot); // Fall back to local
    }
}

bool CloudSaveService::saveECSWorldToCloud(int slot) {
    if (!ecs_world) {
        last_error = "ECS world not initialized";
        return false;
    }

    // Serialize ECS world to JSON
    auto json_data = serializeECSWorld();
    if (json_data.empty()) {
        last_error = "Failed to serialize ECS world";
        return false;
    }

    // Create save game entry
    db::SaveGame save;
    save.user_id = getCurrentUserId();
    save.slot_number = slot;
    save.save_data = json_data;
    save.save_version = GameSerializer::SAVE_VERSION;
    save.game_version = GameSerializer::GAME_VERSION;
    save.device_id = getDeviceId();
    save.device_name = getDeviceName();

    // Extract metadata from ECS
    auto metadata = getECSMetadata();
    save.character_name = metadata.value("character_name", "Unknown");
    save.character_level = metadata.value("character_level", 1);
    save.map_depth = metadata.value("map_depth", 1);
    save.play_time = metadata.value("play_time", 0);
    save.turn_count = metadata.value("turn_count", 0);

    // Save to database
    auto existing = save_repository->findByUserAndSlot(save.user_id, slot);
    bool success = false;

    if (existing.has_value()) {
        save.id = existing->id;
        success = save_repository->update(save);
    } else {
        auto created = save_repository->create(save);
        success = created.has_value();
    }

    if (success) {
        updateStatusCache(slot, SyncStatus::SYNCED);
    }

    return success;
}

bool CloudSaveService::loadECSWorldFromCloud(int slot) {
    if (!ecs_world) {
        last_error = "ECS world not initialized";
        return false;
    }

    auto save = save_repository->findByUserAndSlot(getCurrentUserId(), slot);
    if (!save.has_value()) {
        last_error = "No cloud save found for slot " + std::to_string(slot);
        return false;
    }

    return deserializeECSWorld(save->save_data);
}

// === Synchronization ===

SyncResult CloudSaveService::syncAllSaves() {
    SyncResult result{true, 0, 0, 0, {}};

    if (!isAuthenticated() || !isOnline()) {
        result.success = false;
        result.errors.push_back("Not authenticated or offline");
        return result;
    }

    // Sync all slots (1-9 manual, -1 to -3 auto)
    std::vector<int> slots = {1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -2, -3};

    for (int slot : slots) {
        auto status = syncSlot(slot);

        switch (status) {
            case SyncStatus::SYNCED:
                // Already in sync
                break;
            case SyncStatus::PENDING_UPLOAD:
                if (uploadLocalSave(slot)) {
                    result.saves_uploaded++;
                } else {
                    result.errors.push_back("Failed to upload slot " + std::to_string(slot));
                }
                break;
            case SyncStatus::PENDING_DOWNLOAD:
                if (downloadCloudSave(slot)) {
                    result.saves_downloaded++;
                } else {
                    result.errors.push_back("Failed to download slot " + std::to_string(slot));
                }
                break;
            case SyncStatus::CONFLICT:
                result.conflicts_detected++;
                break;
            default:
                break;
        }
    }

    result.success = result.errors.empty();
    last_sync_time = std::chrono::system_clock::now();
    return result;
}

SyncStatus CloudSaveService::syncSlot(int slot) {
    if (!isAuthenticated()) {
        return SyncStatus::OFFLINE;
    }

    try {
        // Get cloud save
        auto cloud_save = save_repository->findByUserAndSlot(getCurrentUserId(), slot);

        // Check if local save exists
        bool local_exists = game_serializer->saveExists(slot);

        if (!cloud_save.has_value() && !local_exists) {
            return SyncStatus::SYNCED; // Both empty
        }

        if (!cloud_save.has_value() && local_exists) {
            return SyncStatus::PENDING_UPLOAD; // Local only
        }

        if (cloud_save.has_value() && !local_exists) {
            return SyncStatus::PENDING_DOWNLOAD; // Cloud only
        }

        // Both exist - check for conflicts
        if (compareLocalAndCloud(slot, cloud_save.value())) {
            return SyncStatus::SYNCED;
        } else {
            // Check timestamps to determine conflict type
            std::string local_file = game_serializer->getSlotFilename(slot);
            auto local_time = std::filesystem::last_write_time(local_file);

            auto cloud_time = cloud_save->updated_at;
            auto local_sys_time = std::chrono::system_clock::now() -
                (std::filesystem::file_time_type::clock::now() - local_time);

            if (local_sys_time > cloud_time) {
                return SyncStatus::PENDING_UPLOAD;
            } else if (cloud_time > local_sys_time) {
                return SyncStatus::PENDING_DOWNLOAD;
            } else {
                return SyncStatus::CONFLICT;
            }
        }
    } catch (const std::exception& e) {
        last_error = "Sync slot failed: " + std::string(e.what());
        return SyncStatus::ERROR;
    }
}

bool CloudSaveService::uploadLocalSave(int slot) {
    try {
        // Load local save data
        std::string filename = game_serializer->getSlotFilename(slot);
        std::ifstream file(filename);
        if (!file) {
            last_error = "Failed to open local save file";
            return false;
        }

        nlohmann::json local_data;
        file >> local_data;
        file.close();

        // Create or update cloud save
        db::SaveGame save;
        save.user_id = getCurrentUserId();
        save.slot_number = slot;
        save.save_data = local_data;
        save.save_version = GameSerializer::SAVE_VERSION;
        save.game_version = GameSerializer::GAME_VERSION;
        save.device_id = getDeviceId();
        save.device_name = getDeviceName();
        save.sync_status = "synced";

        // Extract metadata
        if (local_data.contains("metadata")) {
            auto& meta = local_data["metadata"];
            save.character_name = meta.value("character_name", "Unknown");
            save.character_level = meta.value("character_level", 1);
            save.map_depth = meta.value("map_depth", 1);
            save.play_time = meta.value("play_time", 0);
            save.turn_count = meta.value("turn_count", 0);
        }

        // Check if exists
        auto existing = save_repository->findByUserAndSlot(save.user_id, slot);
        bool success = false;

        if (existing.has_value()) {
            save.id = existing->id;
            success = save_repository->update(save);
        } else {
            auto created = save_repository->create(save);
            success = created.has_value();
        }

        if (success) {
            updateStatusCache(slot, SyncStatus::SYNCED);
        }

        return success;
    } catch (const std::exception& e) {
        last_error = "Upload failed: " + std::string(e.what());
        return false;
    }
}

bool CloudSaveService::downloadCloudSave(int slot) {
    try {
        auto cloud_save = save_repository->findByUserAndSlot(getCurrentUserId(), slot);
        if (!cloud_save.has_value()) {
            last_error = "No cloud save found";
            return false;
        }

        // Write to local file
        std::string filename = game_serializer->getSlotFilename(slot);
        std::ofstream file(filename);
        if (!file) {
            last_error = "Failed to create local save file";
            return false;
        }

        file << cloud_save->save_data.dump(2);
        file.close();

        updateStatusCache(slot, SyncStatus::SYNCED);
        return true;
    } catch (const std::exception& e) {
        last_error = "Download failed: " + std::string(e.what());
        return false;
    }
}

// === Conflict Resolution ===

std::optional<ConflictResolution> CloudSaveService::detectConflict(int slot) {
    auto status = getSyncStatus(slot);
    if (status == SyncStatus::CONFLICT) {
        return ConflictResolution::CANCEL; // Default - let user decide
    }
    return std::nullopt;
}

bool CloudSaveService::resolveConflict(int slot, ConflictResolution resolution) {
    switch (resolution) {
        case ConflictResolution::USE_LOCAL:
            return uploadLocalSave(slot);

        case ConflictResolution::USE_CLOUD:
            return downloadCloudSave(slot);

        case ConflictResolution::BACKUP_BOTH: {
            // Create backup of cloud version
            auto cloud_save = save_repository->findByUserAndSlot(getCurrentUserId(), slot);
            if (cloud_save.has_value()) {
                save_repository->createBackup(cloud_save->id, "conflict_resolution");
            }
            // Then upload local version
            return uploadLocalSave(slot);
        }

        case ConflictResolution::MERGE_SMART:
            // Future implementation
            last_error = "Smart merge not yet implemented";
            return false;

        case ConflictResolution::CANCEL:
        default:
            return false;
    }
}

std::optional<db::SaveConflict> CloudSaveService::getConflictInfo(int slot) {
    auto conflicts = save_repository->getUnresolvedConflicts(getCurrentUserId());
    for (const auto& conflict : conflicts) {
        auto save = save_repository->findById(conflict.save_id);
        if (save.has_value() && save->slot_number == slot) {
            return conflict;
        }
    }
    return std::nullopt;
}

// === Auto-Save Management ===

bool CloudSaveService::enableAutoSync(int interval_seconds) {
    if (auto_sync_enabled) {
        return true; // Already enabled
    }

    auto_sync_interval = interval_seconds;
    auto_sync_enabled = true;
    sync_thread_running = true;

    // Start sync thread
    sync_thread = std::make_unique<std::thread>(&CloudSaveService::syncThreadLoop, this);

    return true;
}

void CloudSaveService::disableAutoSync() {
    if (!auto_sync_enabled) {
        return;
    }

    auto_sync_enabled = false;
    sync_thread_running = false;

    if (sync_thread && sync_thread->joinable()) {
        sync_thread->join();
    }

    sync_thread.reset();
}

bool CloudSaveService::performAutoSave() {
    if (!isAuthenticated()) {
        return false;
    }

    // Save to auto-save slots (-1, -2, -3)
    static int current_auto_slot = -1;

    bool success = saveToCloud(current_auto_slot);

    // Rotate to next auto-save slot
    current_auto_slot--;
    if (current_auto_slot < -3) {
        current_auto_slot = -1;
    }

    return success;
}

// === Status and Information ===

SyncStatus CloudSaveService::getSyncStatus(int slot) {
    // Check cache first
    auto it = slot_status_cache.find(slot);
    if (it != slot_status_cache.end()) {
        auto cache_age = std::chrono::system_clock::now() - last_sync_time;
        if (cache_age < 30s) {
            return it->second;
        }
    }

    // Refresh status
    auto status = syncSlot(slot);
    updateStatusCache(slot, status);
    return status;
}

std::vector<CloudSaveInfo> CloudSaveService::getAllSaveInfo(bool include_local,
                                                           bool include_cloud) {
    std::vector<CloudSaveInfo> result;
    std::vector<int> slots = {1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -2, -3};

    for (int slot : slots) {
        auto info = getSaveInfo(slot);
        if (info.has_value()) {
            if ((info->is_local && include_local) ||
                (info->is_cloud && include_cloud)) {
                result.push_back(info.value());
            }
        }
    }

    return result;
}

std::optional<CloudSaveInfo> CloudSaveService::getSaveInfo(int slot) {
    CloudSaveInfo info;
    info.slot_number = slot;

    // Check local save
    if (game_serializer->saveExists(slot)) {
        info.is_local = true;
        auto save_info = game_serializer->getSaveInfo(game_serializer->getSlotFilename(slot));
        info.character_name = save_info.player_name;
        info.character_level = save_info.level;
        info.map_depth = save_info.depth;
        info.play_time = save_info.play_time;
        info.turn_count = save_info.turn_count;
    }

    // Check cloud save
    if (isAuthenticated()) {
        auto cloud_save = save_repository->findByUserAndSlot(getCurrentUserId(), slot);
        if (cloud_save.has_value()) {
            info.is_cloud = true;
            info.id = cloud_save->id;
            info.character_name = cloud_save->character_name;
            info.character_level = cloud_save->character_level;
            info.map_depth = cloud_save->map_depth;
            info.play_time = cloud_save->play_time;
            info.turn_count = cloud_save->turn_count;
            info.last_played = cloud_save->last_played_at;
            info.device_name = cloud_save->device_name;
        }
    }

    if (info.is_local || info.is_cloud) {
        info.sync_status = getSyncStatus(slot);
        return info;
    }

    return std::nullopt;
}

bool CloudSaveService::isOnline() const {
    // Simple connectivity check - could be enhanced
    return true; // For now, assume always online if we have network
}

bool CloudSaveService::isAuthenticated() const {
    // For now, check if auth_service exists
    // Full implementation would use AuthenticationService methods
    return auth_service != nullptr && current_user_id > 0;
}

int CloudSaveService::getCurrentUserId() const {
    // Return the cached user ID
    // Full implementation would use AuthenticationService
    return current_user_id;
}

// === ECS Integration ===

nlohmann::json CloudSaveService::serializeECSWorld() const {
    if (!ecs_world) {
        return {};
    }

    nlohmann::json result;

    // Serialize all entities
    std::vector<nlohmann::json> entities;

    // Get all entities from ECS world
    // This would need a method in GameWorld to get all entities
    // For now, using SaveLoadSystem

    result["entities"] = entities;
    result["metadata"] = getECSMetadata();
    result["version"] = GameSerializer::SAVE_VERSION;

    return result;
}

bool CloudSaveService::deserializeECSWorld(const nlohmann::json& data) {
    if (!ecs_world) {
        return false;
    }

    try {
        // Validate version
        if (data.value("version", "") != GameSerializer::SAVE_VERSION) {
            last_error = "Incompatible save version";
            return false;
        }

        // Clear existing world
        // ecs_world->clear(); // Would need this method

        // Load entities
        if (data.contains("entities")) {
            // Entity loading would iterate here
            // This would use EntityFactory
            // for (const auto& entity_data : data["entities"]) {
            //     Create entity from data
            // }
        }

        return true;
    } catch (const std::exception& e) {
        last_error = "Deserialization failed: " + std::string(e.what());
        return false;
    }
}

nlohmann::json CloudSaveService::getECSMetadata() const {
    nlohmann::json meta;

    if (ecs_world) {
        // Get player entity metadata
        // This would need methods to query player from GameWorld
        meta["character_name"] = "Hero"; // Placeholder
        meta["character_level"] = 1;
        meta["map_depth"] = 1;
        meta["play_time"] = 0;
        meta["turn_count"] = 0;
    }

    return meta;
}

// === Utility ===

std::string CloudSaveService::getDeviceId() const {
    static std::string device_id = generateDeviceId();
    return device_id;
}

std::string CloudSaveService::getDeviceName() const {
    #ifdef __APPLE__
        return "macOS";
    #elif __linux__
        return "Linux";
    #elif _WIN32
        return "Windows";
    #else
        return "Unknown";
    #endif
}

std::string CloudSaveService::getSlotFilename(int slot) const {
    return game_serializer->getSlotFilename(slot);
}

// === Private Helper Methods ===

void CloudSaveService::syncThreadLoop() {
    while (sync_thread_running) {
        std::this_thread::sleep_for(std::chrono::seconds(auto_sync_interval));

        if (!sync_thread_running) break;

        if (isAuthenticated() && isOnline()) {
            std::lock_guard<std::mutex> lock(sync_mutex);
            syncAllSaves();
        }
    }
}

bool CloudSaveService::compareLocalAndCloud(int slot, db::SaveGame& cloud_save) {
    try {
        std::string filename = game_serializer->getSlotFilename(slot);
        std::ifstream file(filename);
        if (!file) return false;

        nlohmann::json local_data;
        file >> local_data;

        // Compare key fields
        if (local_data.contains("metadata") && cloud_save.save_data.contains("metadata")) {
            auto& local_meta = local_data["metadata"];
            auto& cloud_meta = cloud_save.save_data["metadata"];

            return local_meta.value("turn_count", 0) == cloud_meta.value("turn_count", 0) &&
                   local_meta.value("play_time", 0) == cloud_meta.value("play_time", 0);
        }

        return false;
    } catch (...) {
        return false;
    }
}

nlohmann::json CloudSaveService::mergeConflictingData(const nlohmann::json& local,
                                                      const nlohmann::json& /*cloud*/) {
    // Smart merge logic - for future implementation
    // For now, just return local
    return local;
}

void CloudSaveService::updateStatusCache(int slot, SyncStatus status) {
    slot_status_cache[slot] = status;
}

bool CloudSaveService::validateSaveData(const nlohmann::json& data) const {
    // Check required fields
    return data.contains("version") &&
           data.contains("metadata") &&
           data.contains("entities");
}

std::string CloudSaveService::generateDeviceId() const {
    // Generate a unique device ID based on system info
    std::stringstream ss;

    #ifdef __APPLE__
        size_t size = 0;
        sysctlbyname("hw.uuid", nullptr, &size, nullptr, 0);
        if (size > 0) {
            std::vector<char> uuid(size);
            sysctlbyname("hw.uuid", uuid.data(), &size, nullptr, 0);
            ss << std::string(uuid.data());
        }
    #elif __linux__
        std::ifstream machine_id("/etc/machine-id");
        if (machine_id) {
            std::string id;
            machine_id >> id;
            ss << id;
        }
    #elif _WIN32
        // Windows implementation would go here
        ss << "windows-device";
    #endif

    if (ss.str().empty()) {
        // Fallback to timestamp + random
        ss << std::chrono::system_clock::now().time_since_epoch().count();
    }

    return ss.str();
}

int CloudSaveService::calculatePlayTime(const nlohmann::json& data) const {
    if (data.contains("metadata")) {
        return data["metadata"].value("play_time", 0);
    }
    return 0;
}