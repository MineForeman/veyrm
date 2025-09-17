#pragma once

#include "models/save_game_models.h"
#include <memory>
#include <functional>

class GameManager;
class CloudSaveService;  // Not in services namespace

namespace services {
    class SaveGameService;
}

namespace controllers {

/**
 * @class SaveLoadController
 * @brief Business logic controller for save/load operations
 *
 * This controller orchestrates save/load operations between the UI and services.
 * It handles error management, progress tracking, and cloud sync coordination.
 */
class SaveLoadController {
public:
    /**
     * @brief Mode of operation
     */
    enum class Mode {
        SAVE,
        LOAD
    };
    
    /**
     * @brief Callbacks for communicating with the view layer
     */
    struct ViewCallbacks {
        std::function<void(const std::string&)> showMessage;
        std::function<void(const std::string&)> showError;
        std::function<void(float)> updateProgress;
        std::function<void()> refreshSlotDisplay;
        std::function<void()> exitScreen;
        std::function<bool(const std::string&)> confirmAction;
    };
    
    /**
     * @brief Constructor
     * @param game_manager Reference to game manager
     * @param save_service Save game service
     * @param cloud_service Optional cloud save service
     */
    explicit SaveLoadController(GameManager* game_manager,
                               std::shared_ptr<services::SaveGameService> save_service,
                               CloudSaveService* cloud_service = nullptr);
    
    ~SaveLoadController() = default;
    
    /**
     * @brief Set the current mode (save or load)
     * @param mode Operation mode
     */
    void setMode(Mode mode) { current_mode = mode; }
    
    /**
     * @brief Get the current mode
     * @return Current operation mode
     */
    Mode getMode() const { return current_mode; }
    
    /**
     * @brief Set view callbacks for UI communication
     * @param callbacks Callback functions for view updates
     */
    void setViewCallbacks(const ViewCallbacks& callbacks) {
        view_callbacks = callbacks;
    }
    
    /**
     * @brief Get list of all save slots
     * @return Save game list with all slots
     */
    models::SaveGameList getSaveSlots();
    
    /**
     * @brief Handle save operation to a slot
     * @param slot_number Slot number (1-based)
     */
    void handleSave(int slot_number);
    
    /**
     * @brief Handle load operation from a slot
     * @param slot_number Slot number (1-based)
     */
    void handleLoad(int slot_number);
    
    /**
     * @brief Handle delete operation for a slot
     * @param slot_number Slot number (1-based)
     */
    void handleDelete(int slot_number);
    
    /**
     * @brief Handle cloud sync for a slot
     * @param slot_number Slot number (1-based)
     * @param upload True to upload, false to download
     */
    void handleCloudSync(int slot_number, bool upload);
    
    /**
     * @brief Refresh the save slot list
     */
    void refreshSlots();
    
    /**
     * @brief Check if a slot can be saved to
     * @param slot_number Slot number (1-based)
     * @return True if slot is available for saving
     */
    bool canSaveToSlot(int slot_number) const;
    
    /**
     * @brief Check if a slot can be loaded from
     * @param slot_number Slot number (1-based)
     * @return True if slot contains a valid save
     */
    bool canLoadFromSlot(int slot_number) const;
    
    /**
     * @brief Get formatted description for a slot
     * @param slot_number Slot number (1-based)
     * @return Formatted slot description
     */
    std::string getSlotDescription(int slot_number) const;
    
private:
    GameManager* game_manager;
    std::shared_ptr<services::SaveGameService> save_service;
    CloudSaveService* cloud_service;
    Mode current_mode;
    ViewCallbacks view_callbacks;
    models::SaveGameList cached_slots;
    
    /**
     * @brief Show message to view if callback is set
     * @param message Message to display
     */
    void showMessage(const std::string& message);
    
    /**
     * @brief Show error to view if callback is set
     * @param error Error message to display
     */
    void showError(const std::string& error);
    
    /**
     * @brief Update progress in view if callback is set
     * @param progress Progress value (0.0-1.0)
     */
    void updateProgress(float progress);
    
    /**
     * @brief Request confirmation from view
     * @param message Confirmation message
     * @return True if confirmed, false otherwise
     */
    bool confirmAction(const std::string& message);
    
    /**
     * @brief Exit the screen through view callback
     */
    void exitScreen();
};

} // namespace controllers