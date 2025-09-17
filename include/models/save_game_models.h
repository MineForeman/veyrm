#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>

namespace models {

/**
 * @brief Information about a save game slot
 */
struct SaveSlot {
    int slot_number = 0;
    bool exists = false;
    std::string filename;
    
    // Game state info
    std::string player_name;
    int player_level = 1;
    int player_hp = 0;
    int player_max_hp = 0;
    int depth = 1;
    std::string location_name;
    
    // Metadata
    std::string timestamp;
    std::chrono::system_clock::time_point save_time;
    size_t file_size = 0;
    std::string game_version;
    
    // Cloud sync status
    bool is_cloud_synced = false;
    std::optional<std::string> cloud_sync_id;
    std::optional<std::chrono::system_clock::time_point> last_sync_time;
};

/**
 * @brief Save/Load operation request
 */
struct SaveOperation {
    enum Type {
        SAVE,
        LOAD,
        DELETE,
        SYNC_TO_CLOUD,
        SYNC_FROM_CLOUD
    };
    
    Type type;
    int slot_number;
    std::optional<std::string> cloud_save_id;
};

/**
 * @brief Result of a save/load operation
 */
struct SaveOperationResult {
    bool success = false;
    std::string message;
    std::optional<std::string> error_details;
    std::optional<SaveSlot> updated_slot;
};

/**
 * @brief Configuration for save game system
 */
struct SaveGameConfig {
    int max_slots = 9;
    int max_cloud_saves = 10;
    bool auto_cloud_sync = false;
    bool compress_saves = true;
    std::string save_directory = "saves";
    std::string cloud_save_directory = "cloud_saves";
};

/**
 * @brief Save game list with metadata
 */
struct SaveGameList {
    std::vector<SaveSlot> slots;
    int total_local_saves = 0;
    int total_cloud_saves = 0;
    size_t total_size_bytes = 0;
    std::optional<std::chrono::system_clock::time_point> last_refresh_time;
};

} // namespace models