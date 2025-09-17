#pragma once

#include "models/save_game_models.h"
#include <memory>
#include <functional>

class GameSerializer;
class GameManager;

namespace services {

/**
 * @class SaveGameService
 * @brief Pure business logic for save game operations
 *
 * This service handles all save/load operations without any UI concerns.
 * It manages file I/O, slot management, and metadata handling.
 */
class SaveGameService {
public:
    /**
     * @brief Constructor
     * @param game_serializer Serializer for game state
     * @param config Save game configuration
     */
    explicit SaveGameService(GameSerializer* game_serializer,
                           const models::SaveGameConfig& config = models::SaveGameConfig());
    
    ~SaveGameService() = default;
    
    /**
     * @brief Get list of all save slots with their information
     * @return List of save slots
     */
    models::SaveGameList getSaveSlots();
    
    /**
     * @brief Get information about a specific save slot
     * @param slot_number Slot number (1-based)
     * @return Save slot information
     */
    models::SaveSlot getSlotInfo(int slot_number);
    
    /**
     * @brief Save game to a specific slot
     * @param slot_number Slot number (1-based)
     * @param game_manager Game manager with current state
     * @return Operation result
     */
    models::SaveOperationResult saveToSlot(int slot_number, GameManager* game_manager);
    
    /**
     * @brief Load game from a specific slot
     * @param slot_number Slot number (1-based)
     * @param game_manager Game manager to load state into
     * @return Operation result
     */
    models::SaveOperationResult loadFromSlot(int slot_number, GameManager* game_manager);
    
    /**
     * @brief Delete a save from a specific slot
     * @param slot_number Slot number (1-based)
     * @return Operation result
     */
    models::SaveOperationResult deleteSlot(int slot_number);
    
    /**
     * @brief Check if a slot has a save
     * @param slot_number Slot number (1-based)
     * @return True if slot contains a save
     */
    bool slotExists(int slot_number) const;
    
    /**
     * @brief Get the filename for a slot
     * @param slot_number Slot number (1-based)
     * @return Filename for the slot
     */
    std::string getSlotFilename(int slot_number) const;
    
    /**
     * @brief Refresh the list of saves (re-scan directory)
     */
    void refreshSaveList();
    
    /**
     * @brief Get the save game configuration
     * @return Current configuration
     */
    const models::SaveGameConfig& getConfig() const { return config; }
    
    /**
     * @brief Set progress callback for long operations
     * @param callback Function to call with progress (0.0-1.0)
     */
    void setProgressCallback(std::function<void(float)> callback) {
        progress_callback = callback;
    }
    
private:
    GameSerializer* game_serializer;
    models::SaveGameConfig config;
    models::SaveGameList cached_save_list;
    std::function<void(float)> progress_callback;
    
    /**
     * @brief Update progress if callback is set
     * @param progress Progress value (0.0-1.0)
     */
    void updateProgress(float progress);
    
    /**
     * @brief Validate slot number
     * @param slot_number Slot number to validate
     * @return True if valid
     */
    bool isValidSlot(int slot_number) const;
    
    /**
     * @brief Load metadata from a save file
     * @param filename Save file path
     * @return Save slot information
     */
    models::SaveSlot loadSlotMetadata(const std::string& filename);
};

} // namespace services