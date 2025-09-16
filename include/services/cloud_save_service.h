/**
 * @file cloud_save_service.h
 * @brief Cloud save synchronization service for ECS game state
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <nlohmann/json.hpp>

namespace db {
    class SaveGameRepository;
    struct SaveGame;
    struct SaveConflict;
    enum class ResolutionType;
}

namespace ecs {
    class SaveLoadSystem;
    class GameWorld;
}

class GameSerializer;
class AuthenticationService;

// Include SyncStatus enum
#include "services/sync_status.h"

/**
 * @enum ConflictResolution
 * @brief How to resolve save conflicts
 */
enum class ConflictResolution {
    USE_LOCAL,      ///< Keep local version
    USE_CLOUD,      ///< Use cloud version
    MERGE_SMART,    ///< Intelligent merge (future)
    BACKUP_BOTH,    ///< Keep both as separate saves
    CANCEL          ///< Cancel operation
};

/**
 * @struct SaveInfo
 * @brief Information about a save game
 */
struct CloudSaveInfo {
    std::string id;                    ///< UUID for cloud saves
    int slot_number;                   ///< Slot number (1-9 manual, -1 to -3 auto)
    std::string character_name;        ///< Character name
    int character_level;               ///< Character level
    int map_depth;                     ///< Current dungeon depth
    int play_time;                     ///< Play time in seconds
    int turn_count;                    ///< Number of turns played

    std::chrono::system_clock::time_point last_played;
    std::string device_name;           ///< Device that last modified
    SyncStatus sync_status;            ///< Current sync status
    bool is_local;                     ///< Save exists locally
    bool is_cloud;                     ///< Save exists in cloud
};

/**
 * @struct SyncResult
 * @brief Result of synchronization operation
 */
struct SyncResult {
    bool success;
    int saves_uploaded;
    int saves_downloaded;
    int conflicts_detected;
    std::vector<std::string> errors;
};

/**
 * @class CloudSaveService
 * @brief Service for cloud save synchronization with ECS integration
 *
 * This service handles:
 * - Synchronizing ECS game state with cloud storage
 * - Managing save conflicts between devices
 * - Auto-save to cloud with configurable intervals
 * - Offline mode with automatic sync when online
 * - Integration with existing ECS SaveLoadSystem
 */
class CloudSaveService {
public:
    /**
     * @brief Construct cloud save service
     * @param save_repo SaveGameRepository for database operations
     * @param game_serializer GameSerializer for local saves
     * @param auth_service Authentication service for user info
     * @param ecs_world ECS game world for direct state access
     */
    CloudSaveService(db::SaveGameRepository* save_repo,
                     GameSerializer* game_serializer,
                     AuthenticationService* auth_service,
                     ecs::GameWorld* ecs_world);

    ~CloudSaveService();

    // === Core Save Operations ===

    /**
     * @brief Save ECS game state to cloud
     * @param slot Slot number (1-9 for manual, -1 to -3 for auto)
     * @param force_upload Upload even if no changes detected
     * @return true if successful
     */
    bool saveToCloud(int slot, bool force_upload = false);

    /**
     * @brief Load ECS game state from cloud
     * @param slot Slot number
     * @param prefer_cloud If conflict, prefer cloud version
     * @return true if successful
     */
    bool loadFromCloud(int slot, bool prefer_cloud = true);

    /**
     * @brief Save current ECS world state to cloud
     * @param slot Target save slot
     * @return true if successful
     */
    bool saveECSWorldToCloud(int slot);

    /**
     * @brief Load ECS world state from cloud
     * @param slot Source save slot
     * @return true if successful
     */
    bool loadECSWorldFromCloud(int slot);

    // === Synchronization ===

    /**
     * @brief Sync all saves between local and cloud
     * @return Sync result with statistics
     */
    SyncResult syncAllSaves();

    /**
     * @brief Sync specific slot
     * @param slot Slot to sync
     * @return Sync status after operation
     */
    SyncStatus syncSlot(int slot);

    /**
     * @brief Upload local save to cloud
     * @param slot Slot to upload
     * @return true if successful
     */
    bool uploadLocalSave(int slot);

    /**
     * @brief Download cloud save to local
     * @param slot Slot to download
     * @return true if successful
     */
    bool downloadCloudSave(int slot);

    // === Conflict Resolution ===

    /**
     * @brief Detect conflict between local and cloud
     * @param slot Slot to check
     * @return Conflict type if any
     */
    std::optional<ConflictResolution> detectConflict(int slot);

    /**
     * @brief Resolve save conflict
     * @param slot Slot with conflict
     * @param resolution How to resolve
     * @return true if resolved
     */
    bool resolveConflict(int slot, ConflictResolution resolution);

    /**
     * @brief Get conflict details for UI
     * @param slot Slot with conflict
     * @return Conflict information
     */
    std::optional<db::SaveConflict> getConflictInfo(int slot);

    // === Auto-Save Management ===

    /**
     * @brief Enable automatic cloud sync
     * @param interval_seconds Sync interval (default 5 minutes)
     * @return true if enabled
     */
    bool enableAutoSync(int interval_seconds = 300);

    /**
     * @brief Disable automatic cloud sync
     */
    void disableAutoSync();

    /**
     * @brief Perform auto-save to cloud
     * @return true if successful
     */
    bool performAutoSave();

    /**
     * @brief Check if auto-sync is enabled
     * @return true if enabled
     */
    bool isAutoSyncEnabled() const { return auto_sync_enabled; }

    // === Status and Information ===

    /**
     * @brief Get sync status for slot
     * @param slot Slot to check
     * @return Current sync status
     */
    SyncStatus getSyncStatus(int slot);

    /**
     * @brief Get all save information
     * @param include_local Include local-only saves
     * @param include_cloud Include cloud-only saves
     * @return Vector of save information
     */
    std::vector<CloudSaveInfo> getAllSaveInfo(bool include_local = true,
                                              bool include_cloud = true);

    /**
     * @brief Get save info for specific slot
     * @param slot Slot to check
     * @return Save information if exists
     */
    std::optional<CloudSaveInfo> getSaveInfo(int slot);

    /**
     * @brief Check if currently online
     * @return true if cloud connection available
     */
    bool isOnline() const;

    /**
     * @brief Check if user is authenticated
     * @return true if user logged in
     */
    bool isAuthenticated() const;

    /**
     * @brief Get current user ID
     * @return User ID or 0 if not authenticated
     */
    int getCurrentUserId() const;

    // === ECS Integration ===

    /**
     * @brief Set ECS world for direct state access
     * @param world ECS game world
     */
    void setECSWorld(ecs::GameWorld* world) { ecs_world = world; }

    /**
     * @brief Set current user ID
     * @param user_id User ID
     */
    void setUserId(int user_id) { current_user_id = user_id; }

    /**
     * @brief Serialize ECS world to JSON
     * @return JSON representation of world state
     */
    nlohmann::json serializeECSWorld() const;

    /**
     * @brief Deserialize JSON to ECS world
     * @param data JSON data to load
     * @return true if successful
     */
    bool deserializeECSWorld(const nlohmann::json& data);

    /**
     * @brief Get metadata from current ECS world
     * @return Metadata JSON
     */
    nlohmann::json getECSMetadata() const;

    // === Utility ===

    /**
     * @brief Get device identifier
     * @return Unique device ID
     */
    std::string getDeviceId() const;

    /**
     * @brief Get device name
     * @return Human-readable device name
     */
    std::string getDeviceName() const;

    /**
     * @brief Convert slot to filename
     * @param slot Slot number
     * @return Filename for slot
     */
    std::string getSlotFilename(int slot) const;

    /**
     * @brief Get last sync error
     * @return Error message if any
     */
    std::string getLastError() const { return last_error; }

private:
    // Services
    db::SaveGameRepository* save_repository;
    GameSerializer* game_serializer;
    AuthenticationService* auth_service;
    ecs::GameWorld* ecs_world;
    std::unique_ptr<ecs::SaveLoadSystem> ecs_save_system;

    // Auto-sync thread
    std::unique_ptr<std::thread> sync_thread;
    std::atomic<bool> auto_sync_enabled{false};
    std::atomic<bool> sync_thread_running{false};
    std::mutex sync_mutex;
    int auto_sync_interval = 300; // seconds

    // State
    std::string last_error;
    std::map<int, SyncStatus> slot_status_cache;
    std::chrono::system_clock::time_point last_sync_time;
    int current_user_id = 0;

    // Helper methods
    void syncThreadLoop();
    bool compareLocalAndCloud(int slot, db::SaveGame& cloud_save);
    nlohmann::json mergeConflictingData(const nlohmann::json& local,
                                        const nlohmann::json& cloud);
    void updateStatusCache(int slot, SyncStatus status);
    bool validateSaveData(const nlohmann::json& data) const;
    std::string generateDeviceId() const;
    int calculatePlayTime(const nlohmann::json& data) const;
};