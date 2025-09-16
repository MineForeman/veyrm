/**
 * @file game_serializer.h
 * @brief Game state serialization and save/load system
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <chrono>
#include <nlohmann/json.hpp>
#include "serializable.h"
#include "services/sync_status.h"

using json = nlohmann::json;

// Forward declarations
class GameManager;
class Map;
class MessageLog;

/**
 * @class GameSerializer
 * @brief Handles saving and loading of complete game state
 *
 * The GameSerializer manages all aspects of game persistence, including
 * manual saves, auto-saves, save file management, and version compatibility.
 * It serializes all game components to JSON format and provides utilities
 * for save file organization and validation.
 *
 * Features:
 * - Manual save to numbered slots (1-9)
 * - Auto-save with rotation (-1, -2, -3)
 * - Save file browsing and management
 * - Version compatibility checking
 * - Complete game state serialization
 * - Error handling and validation
 * - Cloud save integration
 *
 * @see ISerializable
 * @see SaveInfo
 * @see SaveLoadScreen
 * @see CloudSaveService
 */
class GameSerializer {
public:
    explicit GameSerializer(GameManager* game_manager);
    ~GameSerializer() = default;

    // Save operations
    bool saveGame(const std::string& filename);
    bool saveGame(int slot);  // Save to numbered slot (1-9)
    bool autoSave();   // Save to auto-save slots (-1, -2, -3)

    // Load operations
    bool loadGame(const std::string& filename);
    bool loadGame(int slot);  // Load from numbered slot

    // Save management
    std::vector<SaveInfo> getSaveFiles() const;
    SaveInfo getSaveInfo(const std::string& filename) const;
    bool deleteSave(const std::string& filename);
    bool deleteSave(int slot);
    bool validateSave(const std::string& filename) const;
    bool saveExists(int slot) const;
    std::string getSlotFilename(int slot) const;  // Get filename for a slot

    // Settings
    void setAutoSaveEnabled(bool enabled) { auto_save_enabled = enabled; }
    void setAutoSaveInterval(int turns) { auto_save_interval = turns; }
    bool isAutoSaveEnabled() const { return auto_save_enabled; }
    int getAutoSaveInterval() const { return auto_save_interval; }

    // Version info
    static constexpr const char* SAVE_VERSION = "1.0.0";
    static constexpr const char* GAME_VERSION = "0.12.1";

    // Cloud save integration
    void setCloudSaveService(class CloudSaveService* service) { cloud_service = service; }
    void setUserId(int user_id) { current_user_id = user_id; }
    int getUserId() const { return current_user_id; }

    // Enhanced save/load with cloud support
    bool saveGameWithCloud(int slot, bool upload_to_cloud = true);
    bool loadGameWithCloud(int slot, bool prefer_cloud = true);

    // Sync operations
    bool syncWithCloud();
    SyncStatus getCloudSyncStatus(int slot);
    bool isCloudSyncEnabled() const { return cloud_service != nullptr; }

private:
    // Helper methods for serialization
    json serializeMetadata() const;
    json serializeGameState() const;
    json serializeMap() const;
    json serializePlayer() const;
    json serializeEntities() const;
    json serializeItems() const;
    json serializeMessageLog() const;

    // Helper methods for deserialization
    bool deserializeMetadata(const json& data);
    bool deserializeGameState(const json& data);
    bool deserializeMap(const json& data);
    bool deserializePlayer(const json& data);
    bool deserializeEntities(const json& data);
    bool deserializeItems(const json& data);
    bool deserializeMessageLog(const json& data);

    // Utility methods
    std::string getSavePath(const std::string& filename) const;
    bool ensureSaveDirectoryExists() const;
    std::string getCurrentTimestamp() const;
    int calculatePlayTime() const;

    // Auto-save management
    void rotateAutoSaves();
    int getCurrentAutoSaveSlot() const;

    // Data members
    GameManager* game_manager;
    std::filesystem::path save_directory;
    bool auto_save_enabled = true;
    int auto_save_interval = 100;  // turns
    // int auto_save_counter = 0;  // For future auto-save feature
    int current_auto_save_slot = -1;
    std::chrono::time_point<std::chrono::steady_clock> session_start_time;

    // Cloud save support
    class CloudSaveService* cloud_service = nullptr;
    int current_user_id = 0;
};