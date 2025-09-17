#include "services/save_game_service.h"
#include "game_serializer.h"
#include "game_state.h"
#include "log.h"
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace services {

SaveGameService::SaveGameService(GameSerializer* game_serializer,
                                 const models::SaveGameConfig& config)
    : game_serializer(game_serializer)
    , config(config) {
    refreshSaveList();
}

models::SaveGameList SaveGameService::getSaveSlots() {
    return cached_save_list;
}

models::SaveSlot SaveGameService::getSlotInfo(int slot_number) {
    if (!isValidSlot(slot_number)) {
        return models::SaveSlot();
    }
    
    std::string filename = getSlotFilename(slot_number);
    return loadSlotMetadata(filename);
}

models::SaveOperationResult SaveGameService::saveToSlot(int slot_number, GameManager* game_manager) {
    models::SaveOperationResult result;
    
    if (!isValidSlot(slot_number)) {
        result.success = false;
        result.message = "Invalid slot number";
        return result;
    }
    
    if (!game_serializer || !game_manager) {
        result.success = false;
        result.message = "Save system not initialized";
        return result;
    }
    
    try {
        updateProgress(0.1f);
        
        // Check if overwriting existing save
        bool overwriting = slotExists(slot_number);
        if (overwriting) {
            Log::info("Overwriting existing save in slot " + std::to_string(slot_number));
        }
        
        updateProgress(0.3f);
        
        // Perform the save
        std::string filename = getSlotFilename(slot_number);
        bool save_success = game_manager->saveGame(slot_number);
        
        updateProgress(0.8f);
        
        if (save_success) {
            result.success = true;
            result.message = "Game saved to slot " + std::to_string(slot_number);
            
            // Refresh slot info
            refreshSaveList();
            result.updated_slot = getSlotInfo(slot_number);
            
            Log::info("Save successful: " + filename);
        } else {
            result.success = false;
            result.message = "Failed to save game";
            result.error_details = "Save operation failed";
            
            Log::error("Save failed: " + filename);
        }
        
        updateProgress(1.0f);
    } catch (const std::exception& e) {
        result.success = false;
        result.message = "Save error occurred";
        result.error_details = e.what();
        
        Log::error("Save exception: " + std::string(e.what()));
    }
    
    return result;
}

models::SaveOperationResult SaveGameService::loadFromSlot(int slot_number, GameManager* game_manager) {
    models::SaveOperationResult result;
    
    if (!isValidSlot(slot_number)) {
        result.success = false;
        result.message = "Invalid slot number";
        return result;
    }
    
    if (!slotExists(slot_number)) {
        result.success = false;
        result.message = "No save found in slot " + std::to_string(slot_number);
        return result;
    }
    
    if (!game_serializer || !game_manager) {
        result.success = false;
        result.message = "Save system not initialized";
        return result;
    }
    
    try {
        updateProgress(0.1f);
        
        std::string filename = getSlotFilename(slot_number);
        Log::info("Loading save from: " + filename);
        
        updateProgress(0.3f);
        
        // Perform the load
        bool load_success = game_manager->loadGame(slot_number);
        
        updateProgress(0.9f);
        
        if (load_success) {
            result.success = true;
            result.message = "Game loaded from slot " + std::to_string(slot_number);
            result.updated_slot = getSlotInfo(slot_number);
            
            Log::info("Load successful: " + filename);
        } else {
            result.success = false;
            result.message = "Failed to load game";
            result.error_details = "Load operation failed or save file corrupted";
            
            Log::error("Load failed: " + filename);
        }
        
        updateProgress(1.0f);
    } catch (const std::exception& e) {
        result.success = false;
        result.message = "Load error occurred";
        result.error_details = e.what();
        
        Log::error("Load exception: " + std::string(e.what()));
    }
    
    return result;
}

models::SaveOperationResult SaveGameService::deleteSlot(int slot_number) {
    models::SaveOperationResult result;
    
    if (!isValidSlot(slot_number)) {
        result.success = false;
        result.message = "Invalid slot number";
        return result;
    }
    
    if (!slotExists(slot_number)) {
        result.success = false;
        result.message = "No save found in slot " + std::to_string(slot_number);
        return result;
    }
    
    try {
        if (game_serializer && game_serializer->deleteSave(slot_number)) {
            result.success = true;
            result.message = "Save deleted from slot " + std::to_string(slot_number);
            
            // Refresh the list
            refreshSaveList();
            
            Log::info("Delete successful: slot " + std::to_string(slot_number));
        } else {
            result.success = false;
            result.message = "Failed to delete save";
            result.error_details = "Delete operation failed";
            
            Log::error("Delete failed: slot " + std::to_string(slot_number));
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.message = "Delete error occurred";
        result.error_details = e.what();
        
        Log::error("Delete exception: " + std::string(e.what()));
    }
    
    return result;
}

bool SaveGameService::slotExists(int slot_number) const {
    if (!isValidSlot(slot_number) || slot_number > static_cast<int>(cached_save_list.slots.size())) {
        return false;
    }
    
    return cached_save_list.slots[slot_number - 1].exists;
}

std::string SaveGameService::getSlotFilename(int slot_number) const {
    if (!game_serializer) {
        return "";
    }
    
    return game_serializer->getSlotFilename(slot_number);
}

void SaveGameService::refreshSaveList() {
    cached_save_list.slots.clear();
    cached_save_list.total_local_saves = 0;
    cached_save_list.total_size_bytes = 0;
    
    if (!game_serializer) {
        return;
    }
    
    for (int i = 1; i <= config.max_slots; i++) {
        std::string filename = getSlotFilename(i);
        models::SaveSlot slot = loadSlotMetadata(filename);
        slot.slot_number = i;
        slot.filename = filename;
        
        cached_save_list.slots.push_back(slot);
        
        if (slot.exists) {
            cached_save_list.total_local_saves++;
            cached_save_list.total_size_bytes += slot.file_size;
        }
    }
    
    cached_save_list.last_refresh_time = std::chrono::system_clock::now();
}

void SaveGameService::updateProgress(float progress) {
    if (progress_callback) {
        progress_callback(progress);
    }
}

bool SaveGameService::isValidSlot(int slot_number) const {
    return slot_number >= 1 && slot_number <= config.max_slots;
}

models::SaveSlot SaveGameService::loadSlotMetadata(const std::string& filename) {
    models::SaveSlot slot;
    slot.filename = filename;
    
    if (!game_serializer) {
        slot.exists = false;
        return slot;
    }
    
    // Get save info from serializer
    SaveInfo info = game_serializer->getSaveInfo(filename);
    
    // Convert SaveInfo to SaveSlot
    slot.exists = info.exists;
    
    if (slot.exists) {
        slot.player_name = info.player_name;
        slot.player_level = info.level;
        slot.player_hp = info.player_hp;
        slot.player_max_hp = info.player_max_hp;
        slot.depth = info.depth;
        slot.timestamp = info.timestamp;
        
        // Try to get file size
        try {
            if (std::filesystem::exists(filename)) {
                slot.file_size = std::filesystem::file_size(filename);
            }
        } catch (...) {
            slot.file_size = 0;
        }
        
        // TODO: Add cloud sync status when CloudSaveService is integrated
        slot.is_cloud_synced = false;
    }
    
    return slot;
}

} // namespace services