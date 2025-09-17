#include "controllers/save_load_controller.h"
#include "services/save_game_service.h"
#include "services/cloud_save_service.h"  // Include full definition
#include "game_state.h"
#include "message_log.h"
#include "log.h"
#include <sstream>
#include <iomanip>

namespace controllers {

SaveLoadController::SaveLoadController(GameManager* game_manager,
                                     std::shared_ptr<services::SaveGameService> save_service,
                                     CloudSaveService* cloud_service)
    : game_manager(game_manager)
    , save_service(save_service)
    , cloud_service(cloud_service)
    , current_mode(Mode::LOAD) {
    
    // Set up progress callback
    if (save_service) {
        save_service->setProgressCallback([this](float progress) {
            updateProgress(progress);
        });
    }
    
    // Initial refresh
    refreshSlots();
}

models::SaveGameList SaveLoadController::getSaveSlots() {
    return cached_slots;
}

void SaveLoadController::handleSave(int slot_number) {
    if (!save_service || !game_manager) {
        showError("Save system not initialized");
        return;
    }
    
    // Check if overwriting existing save
    if (save_service->slotExists(slot_number)) {
        std::string msg = "Overwrite existing save in slot " + std::to_string(slot_number) + "?";
        if (!confirmAction(msg)) {
            return;
        }
    }
    
    // Perform save
    showMessage("Saving game...");
    auto result = save_service->saveToSlot(slot_number, game_manager);
    
    if (result.success) {
        showMessage(result.message);
        
        // Add to message log
        if (game_manager->getMessageLog()) {
            game_manager->getMessageLog()->addMessage(result.message);
        }
        
        // Update game state and exit
        game_manager->setState(GameState::PLAYING);
        exitScreen();
        
        // Trigger cloud sync if enabled
        if (cloud_service && cloud_service->isAutoSyncEnabled()) {
            handleCloudSync(slot_number, true);
        }
    } else {
        showError(result.message);
        if (result.error_details) {
            Log::error("Save details: " + *result.error_details);
        }
    }
    
    refreshSlots();
}

void SaveLoadController::handleLoad(int slot_number) {
    if (!save_service || !game_manager) {
        showError("Save system not initialized");
        return;
    }
    
    if (!save_service->slotExists(slot_number)) {
        showError("No save found in slot " + std::to_string(slot_number));
        return;
    }
    
    // Confirm load if game is in progress
    if (game_manager->getState() == GameState::PLAYING) {
        std::string msg = "Loading will lose current game progress. Continue?";
        if (!confirmAction(msg)) {
            return;
        }
    }
    
    // Perform load
    showMessage("Loading game...");
    auto result = save_service->loadFromSlot(slot_number, game_manager);
    
    if (result.success) {
        showMessage(result.message);
        
        // Add to message log
        if (game_manager->getMessageLog()) {
            game_manager->getMessageLog()->addMessage(result.message);
        }
        
        // Update game state and exit
        game_manager->setState(GameState::PLAYING);
        exitScreen();
    } else {
        showError(result.message);
        if (result.error_details) {
            Log::error("Load details: " + *result.error_details);
        }
    }
}

void SaveLoadController::handleDelete(int slot_number) {
    if (!save_service) {
        showError("Save system not initialized");
        return;
    }
    
    if (!save_service->slotExists(slot_number)) {
        showError("No save found in slot " + std::to_string(slot_number));
        return;
    }
    
    // Confirm deletion
    std::string msg = "Delete save in slot " + std::to_string(slot_number) + "? This cannot be undone.";
    if (!confirmAction(msg)) {
        return;
    }
    
    // Perform delete
    auto result = save_service->deleteSlot(slot_number);
    
    if (result.success) {
        showMessage(result.message);
        
        // Add to message log
        if (game_manager->getMessageLog()) {
            game_manager->getMessageLog()->addMessage(result.message);
        }
    } else {
        showError(result.message);
        if (result.error_details) {
            Log::error("Delete details: " + *result.error_details);
        }
    }
    
    refreshSlots();
}

void SaveLoadController::handleCloudSync(int slot_number, bool upload) {
    if (!cloud_service) {
        showError("Cloud sync not available");
        return;
    }

    if (!cloud_service->isAutoSyncEnabled()) {
        showError("Cloud sync is disabled");
        return;
    }
    
    try {
        if (upload) {
            showMessage("Uploading to cloud...");
            bool success = cloud_service->uploadLocalSave(slot_number);
            
            if (success) {
                showMessage("Save uploaded to cloud");
            } else {
                showError("Failed to upload save");
            }
        } else {
            showMessage("Downloading from cloud...");
            bool success = cloud_service->downloadCloudSave(slot_number);
            
            if (success) {
                showMessage("Save downloaded from cloud");
                refreshSlots();
            } else {
                showError("Failed to download save");
            }
        }
    } catch (const std::exception& e) {
        showError("Cloud sync error: " + std::string(e.what()));
        Log::error("Cloud sync exception: " + std::string(e.what()));
    }
}

void SaveLoadController::refreshSlots() {
    if (save_service) {
        save_service->refreshSaveList();
        cached_slots = save_service->getSaveSlots();
        
        if (view_callbacks.refreshSlotDisplay) {
            view_callbacks.refreshSlotDisplay();
        }
    }
}

bool SaveLoadController::canSaveToSlot(int slot_number) const {
    if (!save_service) {
        return false;
    }
    
    // Can save to any valid slot (will overwrite if exists)
    return slot_number >= 1 && slot_number <= save_service->getConfig().max_slots;
}

bool SaveLoadController::canLoadFromSlot(int slot_number) const {
    if (!save_service) {
        return false;
    }
    
    // Can only load from slots with saves
    return save_service->slotExists(slot_number);
}

std::string SaveLoadController::getSlotDescription(int slot_number) const {
    if (!save_service || slot_number < 1 || 
        slot_number > static_cast<int>(cached_slots.slots.size())) {
        return "Invalid slot";
    }
    
    const auto& slot = cached_slots.slots[slot_number - 1];
    std::stringstream ss;
    ss << "Slot " << slot_number << ": ";
    
    if (slot.exists) {
        ss << "Level " << slot.depth;
        ss << " - " << slot.player_name;
        ss << " (HP: " << slot.player_hp << "/" << slot.player_max_hp << ")";
        ss << " - " << slot.timestamp;
        
        if (slot.is_cloud_synced) {
            ss << " ☁";
        }
    } else {
        ss << "Empty";
    }
    
    return ss.str();
}

void SaveLoadController::showMessage(const std::string& message) {
    if (view_callbacks.showMessage) {
        view_callbacks.showMessage(message);
    }
    Log::info(message);
}

void SaveLoadController::showError(const std::string& error) {
    if (view_callbacks.showError) {
        view_callbacks.showError(error);
    }
    Log::error(error);
}

void SaveLoadController::updateProgress(float progress) {
    if (view_callbacks.updateProgress) {
        view_callbacks.updateProgress(progress);
    }
}

bool SaveLoadController::confirmAction(const std::string& message) {
    if (view_callbacks.confirmAction) {
        return view_callbacks.confirmAction(message);
    }
    // Default to no confirmation if callback not set
    return false;
}

void SaveLoadController::exitScreen() {
    if (view_callbacks.exitScreen) {
        view_callbacks.exitScreen();
    }
}

} // namespace controllers