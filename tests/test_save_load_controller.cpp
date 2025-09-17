#include <catch2/catch_test_macros.hpp>
#include "controllers/save_load_controller.h"
#include "services/save_game_service.h"
#include "game_state.h"
#include "game_serializer.h"
#include <memory>

// Mock SaveGameService for testing
class MockSaveGameService : public services::SaveGameService {
private:
    bool should_succeed = true;
    models::SaveGameList mock_list;
    
public:
    MockSaveGameService() : SaveGameService(nullptr) {}
    
    void setMockBehavior(bool succeed) {
        should_succeed = succeed;
    }
    
    void setMockSlots(const models::SaveGameList& list) {
        mock_list = list;
    }
    
    models::SaveGameList getSaveSlots() {
        return mock_list;
    }

    models::SaveOperationResult saveToSlot(int slot_number, GameManager* /*gm*/) {
        models::SaveOperationResult result;
        result.success = should_succeed;

        if (should_succeed) {
            result.message = "Save successful to slot " + std::to_string(slot_number);
        } else {
            result.message = "Save failed";
            result.error_details = "Mock error";
        }

        return result;
    }

    models::SaveOperationResult loadFromSlot(int slot_number, GameManager* /*gm*/) {
        models::SaveOperationResult result;
        result.success = should_succeed;

        if (should_succeed) {
            result.message = "Load successful from slot " + std::to_string(slot_number);
        } else {
            result.message = "Load failed";
            result.error_details = "Mock error";
        }

        return result;
    }

    models::SaveOperationResult deleteSlot(int slot_number) {
        models::SaveOperationResult result;
        result.success = should_succeed;

        if (should_succeed) {
            result.message = "Delete successful for slot " + std::to_string(slot_number);
        } else {
            result.message = "Delete failed";
            result.error_details = "Mock error";
        }

        return result;
    }

    bool slotExists(int slot_number) const {
        if (slot_number > 0 && slot_number <= static_cast<int>(mock_list.slots.size())) {
            return mock_list.slots[slot_number - 1].exists;
        }
        return false;
    }
};

TEST_CASE("SaveLoadController Tests", "[save_load][controller]") {
    // Create mock services
    auto mock_save_service = std::make_shared<MockSaveGameService>();
    GameManager* game_manager = nullptr; // Mock or nullptr for basic tests
    
    // Create controller
    controllers::SaveLoadController controller(game_manager, mock_save_service);
    
    // Set up mock slots
    models::SaveGameList mock_list;
    for (int i = 1; i <= 9; i++) {
        models::SaveSlot slot;
        slot.slot_number = i;
        slot.exists = (i <= 3); // First 3 slots have saves
        
        if (slot.exists) {
            slot.player_name = "Player" + std::to_string(i);
            slot.player_hp = 50 + i * 10;
            slot.player_max_hp = 100;
            slot.depth = i;
            slot.timestamp = "2025-01-" + std::to_string(10 + i);
        }
        
        mock_list.slots.push_back(slot);
    }
    mock_save_service->setMockSlots(mock_list);
    
    SECTION("Get save slots") {
        auto slots = controller.getSaveSlots();
        REQUIRE(slots.slots.size() == 9);
        REQUIRE(slots.slots[0].exists == true);
        REQUIRE(slots.slots[3].exists == false);
    }
    
    SECTION("Check slot availability") {
        REQUIRE(controller.canSaveToSlot(1) == true);
        REQUIRE(controller.canSaveToSlot(9) == true);
        REQUIRE(controller.canSaveToSlot(10) == false); // Out of range
        
        REQUIRE(controller.canLoadFromSlot(1) == true);
        REQUIRE(controller.canLoadFromSlot(3) == true);
        REQUIRE(controller.canLoadFromSlot(4) == false); // No save
    }
    
    SECTION("Get slot descriptions") {
        std::string desc1 = controller.getSlotDescription(1);
        REQUIRE(desc1.find("Slot 1:") != std::string::npos);
        REQUIRE(desc1.find("Player1") != std::string::npos);
        REQUIRE(desc1.find("Level 1") != std::string::npos);
        
        std::string desc4 = controller.getSlotDescription(4);
        REQUIRE(desc4.find("Empty") != std::string::npos);
    }
    
    SECTION("Handle save operation") {
        bool message_shown = false;
        std::string last_message;
        
        controllers::SaveLoadController::ViewCallbacks callbacks;
        callbacks.showMessage = [&](const std::string& msg) {
            message_shown = true;
            last_message = msg;
        };
        controller.setViewCallbacks(callbacks);
        
        mock_save_service->setMockBehavior(true);
        controller.handleSave(5);
        
        REQUIRE(message_shown == true);
        REQUIRE(last_message.find("successful") != std::string::npos);
    }
    
    SECTION("Handle load operation") {
        bool error_shown = false;
        std::string last_error;
        
        controllers::SaveLoadController::ViewCallbacks callbacks;
        callbacks.showError = [&](const std::string& msg) {
            error_shown = true;
            last_error = msg;
        };
        controller.setViewCallbacks(callbacks);
        
        // Try to load from empty slot
        controller.handleLoad(5);
        
        REQUIRE(error_shown == true);
        REQUIRE(last_error.find("No save found") != std::string::npos);
    }
    
    SECTION("Handle delete operation") {
        bool confirm_called = false;
        
        controllers::SaveLoadController::ViewCallbacks callbacks;
        callbacks.confirmAction = [&](const std::string& /*msg*/) {
            confirm_called = true;
            return true; // Confirm deletion
        };
        controller.setViewCallbacks(callbacks);
        
        mock_save_service->setMockBehavior(true);
        controller.handleDelete(1);
        
        REQUIRE(confirm_called == true);
    }
    
    SECTION("Mode switching") {
        REQUIRE(controller.getMode() == controllers::SaveLoadController::Mode::LOAD);
        
        controller.setMode(controllers::SaveLoadController::Mode::SAVE);
        REQUIRE(controller.getMode() == controllers::SaveLoadController::Mode::SAVE);
    }
}

TEST_CASE("SaveGameService Tests", "[save_game][service]") {
    // Test the service with mock serializer
    services::SaveGameService service(nullptr);
    
    SECTION("Configuration") {
        auto config = service.getConfig();
        REQUIRE(config.max_slots == 9);
        REQUIRE(config.save_directory == "saves");
    }
    
    SECTION("Valid slot checking") {
        REQUIRE(service.slotExists(1) == false); // No serializer, no saves
        REQUIRE(service.getSlotFilename(1) == ""); // No serializer
    }
    
    SECTION("Progress callback") {
        float last_progress = 0.0f;
        service.setProgressCallback([&](float p) {
            last_progress = p;
        });
        
        // Progress callback would be called during save/load operations
        // Can't test directly without real operations
    }
}