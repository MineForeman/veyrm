/**
 * @file save_game_repository.h
 * @brief Repository for cloud save game management
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace db {

// Forward declarations
class DatabaseManager;
class Result;

/**
 * @struct SaveGame
 * @brief Represents a saved game in the database
 */
struct SaveGame {
    std::string id;                    ///< UUID
    int user_id = 0;                   ///< User who owns this save
    int slot_number = 0;               ///< Slot number (1-9 manual, -1 to -3 auto)
    std::string character_name;        ///< Character name
    int character_level = 1;           ///< Character level
    int map_depth = 1;                 ///< Current dungeon depth
    int play_time = 0;                 ///< Play time in seconds
    int turn_count = 0;                ///< Number of turns played

    nlohmann::json save_data;          ///< Complete game state as JSON
    std::string save_version;          ///< Save format version
    std::string game_version;          ///< Game version that created this save

    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::chrono::system_clock::time_point last_played_at;

    std::string device_id;             ///< Device that created/modified this save
    std::string device_name;           ///< Human-readable device name
    std::string sync_status = "synced"; ///< synced, pending, conflict

    // Helper methods
    bool isAutoSave() const { return slot_number < 0; }
    bool isManualSave() const { return slot_number > 0; }
    bool isSynced() const { return sync_status == "synced"; }
    bool hasConflict() const { return sync_status == "conflict"; }
};

/**
 * @struct SaveConflict
 * @brief Represents a save game conflict
 */
struct SaveConflict {
    std::string id;                    ///< UUID
    std::string save_id;               ///< Reference to save_games
    nlohmann::json conflicting_data;   ///< Conflicting save data
    std::string device_id;             ///< Device that created conflict
    std::string device_name;           ///< Human-readable device name
    std::chrono::system_clock::time_point created_at;
    bool resolved = false;             ///< Has conflict been resolved
    std::string resolution_type;       ///< How conflict was resolved
};

/**
 * @struct SaveBackup
 * @brief Represents a backup of a save game
 */
struct SaveBackup {
    std::string id;                    ///< UUID
    std::string save_id;               ///< Reference to save_games
    nlohmann::json backup_data;        ///< Backed up game data
    std::string backup_reason;         ///< Why backup was created
    std::chrono::system_clock::time_point created_at;
};

/**
 * @enum ResolutionType
 * @brief Types of conflict resolution
 */
enum class ResolutionType {
    LOCAL_WINS,     ///< Keep local version
    CLOUD_WINS,     ///< Keep cloud version
    MERGE,          ///< Merge both versions
    BACKUP_BOTH     ///< Keep both as separate saves
};

/**
 * @class SaveGameRepository
 * @brief Repository for managing cloud save games
 */
class SaveGameRepository {
public:
    /**
     * @brief Construct repository with database manager
     * @param db_manager Database manager instance
     */
    explicit SaveGameRepository(DatabaseManager& db_manager);

    ~SaveGameRepository() = default;

    // === CRUD Operations ===

    /**
     * @brief Create a new save game
     * @param save Save game to create
     * @return Created save with ID, or nullopt on failure
     */
    std::optional<SaveGame> create(const SaveGame& save);

    /**
     * @brief Find save by ID
     * @param id Save game UUID
     * @return Save game if found
     */
    std::optional<SaveGame> findById(const std::string& id);

    /**
     * @brief Find all saves for a user
     * @param user_id User ID
     * @return Vector of saves
     */
    std::vector<SaveGame> findByUserId(int user_id);

    /**
     * @brief Find save by user and slot
     * @param user_id User ID
     * @param slot Slot number
     * @return Save game if found
     */
    std::optional<SaveGame> findByUserAndSlot(int user_id, int slot);

    /**
     * @brief Update existing save
     * @param save Save game to update
     * @return true if successful
     */
    bool update(const SaveGame& save);

    /**
     * @brief Delete save by ID
     * @param id Save game UUID
     * @return true if successful
     */
    bool deleteById(const std::string& id);

    /**
     * @brief Delete save by user and slot
     * @param user_id User ID
     * @param slot Slot number
     * @return true if successful
     */
    bool deleteByUserAndSlot(int user_id, int slot);

    // === Sync Operations ===

    /**
     * @brief Get all unsynced saves for a user
     * @param user_id User ID
     * @return Vector of unsynced saves
     */
    std::vector<SaveGame> getUnsyncedSaves(int user_id);

    /**
     * @brief Mark save as synced
     * @param save_id Save game UUID
     * @return true if successful
     */
    bool markAsSynced(const std::string& save_id);

    /**
     * @brief Mark save as having conflict
     * @param save_id Save game UUID
     * @return true if successful
     */
    bool markAsConflicted(const std::string& save_id);

    // === Conflict Resolution ===

    /**
     * @brief Create a save conflict record
     * @param conflict Conflict to create
     * @return true if successful
     */
    bool createConflict(const SaveConflict& conflict);

    /**
     * @brief Get unresolved conflicts for a user
     * @param user_id User ID
     * @return Vector of conflicts
     */
    std::vector<SaveConflict> getUnresolvedConflicts(int user_id);

    /**
     * @brief Resolve a conflict
     * @param conflict_id Conflict UUID
     * @param type Resolution type
     * @return true if successful
     */
    bool resolveConflict(const std::string& conflict_id, ResolutionType type);

    // === Backup Operations ===

    /**
     * @brief Create a backup of a save
     * @param save_id Save game UUID
     * @param reason Reason for backup
     * @return true if successful
     */
    bool createBackup(const std::string& save_id, const std::string& reason);

    /**
     * @brief Get backups for a save
     * @param save_id Save game UUID
     * @param limit Maximum number of backups to return
     * @return Vector of backups
     */
    std::vector<SaveBackup> getBackups(const std::string& save_id, int limit = 5);

    /**
     * @brief Delete old backups, keeping only the most recent
     * @param save_id Save game UUID
     * @param keep_count Number of backups to keep
     * @return Number of backups deleted
     */
    int pruneOldBackups(const std::string& save_id, int keep_count = 5);

    // === Statistics ===

    /**
     * @brief Get save statistics for a user
     * @param user_id User ID
     * @return Map of statistics
     */
    std::map<std::string, int> getSaveStatistics(int user_id);

    /**
     * @brief Get total save data size for a user
     * @param user_id User ID
     * @return Size in bytes
     */
    size_t getTotalSaveSize(int user_id);

private:
    DatabaseManager& db_manager;

    // Helper methods
    SaveGame rowToSaveGame(const Result& row, int rowIndex) const;
    SaveConflict rowToConflict(const Result& row, int rowIndex) const;
    SaveBackup rowToBackup(const Result& row, int rowIndex) const;
    std::string jsonToString(const nlohmann::json& json) const;
    nlohmann::json stringToJson(const std::string& str) const;
    std::string timestampToString(const std::chrono::system_clock::time_point& tp) const;
    std::chrono::system_clock::time_point stringToTimestamp(const std::string& str) const;
};

} // namespace db