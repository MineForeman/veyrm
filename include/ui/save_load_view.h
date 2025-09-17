#pragma once

#include "models/save_game_models.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>
#include <functional>

namespace ui {

/**
 * @class SaveLoadView
 * @brief Pure UI view for save/load operations
 *
 * This class handles only UI rendering and user interaction for save/load operations.
 * All business logic is delegated to the controller through callbacks.
 */
class SaveLoadView {
public:
    /**
     * @brief Result of the save/load screen interaction
     */
    enum class Result {
        SUCCESS,
        CANCELLED,
        FAILED
    };
    
    /**
     * @brief Display mode for the view
     */
    enum class Mode {
        SAVE,
        LOAD
    };
    
    /**
     * @brief Callbacks to controller for business logic
     */
    struct ControllerCallbacks {
        std::function<models::SaveGameList()> getSaveSlots;
        std::function<void(int)> onSave;
        std::function<void(int)> onLoad;
        std::function<void(int)> onDelete;
        std::function<void(int, bool)> onCloudSync;
        std::function<void()> onRefresh;
        std::function<std::string(int)> getSlotDescription;
        std::function<bool(int)> canSaveToSlot;
        std::function<bool(int)> canLoadFromSlot;
    };
    
    /**
     * @brief Constructor
     */
    SaveLoadView();
    
    /**
     * @brief Destructor
     */
    ~SaveLoadView() = default;
    
    /**
     * @brief Set the display mode
     * @param mode Save or load mode
     */
    void setMode(Mode mode) { current_mode = mode; }
    
    /**
     * @brief Get the current mode
     * @return Current display mode
     */
    Mode getMode() const { return current_mode; }
    
    /**
     * @brief Set controller callbacks
     * @param callbacks Callbacks to business logic
     */
    void setControllerCallbacks(const ControllerCallbacks& callbacks) {
        controller_callbacks = callbacks;
    }
    
    /**
     * @brief Run the save/load view
     * @return Result of the interaction
     */
    Result run();
    
    /**
     * @brief Show a message to the user
     * @param message Message to display
     */
    void showMessage(const std::string& message);
    
    /**
     * @brief Show an error message to the user
     * @param error Error message to display
     */
    void showError(const std::string& error);
    
    /**
     * @brief Update progress indicator
     * @param progress Progress value (0.0-1.0)
     */
    void updateProgress(float progress);
    
    /**
     * @brief Refresh the slot display
     */
    void refreshSlotDisplay();
    
    /**
     * @brief Exit the screen with a result
     * @param result Result to set
     */
    void exitWithResult(Result result);
    
    /**
     * @brief Show confirmation dialog
     * @param message Confirmation message
     * @return True if confirmed, false otherwise
     */
    bool confirmAction(const std::string& message);
    
    /**
     * @brief Set selected slot
     * @param slot Slot number (0-based)
     */
    void setSelectedSlot(int slot) { selected_slot = slot; }
    
    /**
     * @brief Get selected slot
     * @return Selected slot number (0-based)
     */
    int getSelectedSlot() const { return selected_slot; }
    
private:
    // UI State
    Mode current_mode = Mode::LOAD;
    Result result = Result::CANCELLED;
    int selected_slot = 0;
    int max_slots = 9;
    
    // Display state
    std::string status_message;
    std::string error_message;
    bool show_status = false;
    bool show_error = false;
    bool show_progress = false;
    float progress_value = 0.0f;
    bool show_confirmation = false;
    std::string confirmation_message;
    
    // Cached data
    models::SaveGameList cached_slots;
    
    // UI Components
    ftxui::Component container;
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
    
    // Controller callbacks
    ControllerCallbacks controller_callbacks;
    
    // UI Helper Methods
    
    /**
     * @brief Create the main component
     * @return FTXUI component
     */
    ftxui::Component createMainComponent();
    
    /**
     * @brief Create slot list component
     * @return FTXUI component for slot list
     */
    ftxui::Component createSlotList();
    
    /**
     * @brief Create action buttons component
     * @return FTXUI component for action buttons
     */
    ftxui::Component createActionButtons();
    
    /**
     * @brief Create progress bar component
     * @return FTXUI component for progress bar
     */
    ftxui::Component createProgressBar();
    
    /**
     * @brief Handle slot selection
     * @param slot Slot number (0-based)
     */
    void handleSlotSelection(int slot);
    
    /**
     * @brief Handle save action
     */
    void handleSaveAction();
    
    /**
     * @brief Handle load action
     */
    void handleLoadAction();
    
    /**
     * @brief Handle delete action
     */
    void handleDeleteAction();
    
    /**
     * @brief Handle refresh action
     */
    void handleRefreshAction();
    
    /**
     * @brief Format slot display
     * @param slot Save slot information
     * @param index Slot index (0-based)
     * @return Formatted string for display
     */
    std::string formatSlotDisplay(const models::SaveSlot& slot, int index) const;
    
    /**
     * @brief Clear all messages
     */
    void clearMessages();
};

} // namespace ui