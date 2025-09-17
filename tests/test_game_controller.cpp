#include <catch2/catch_test_macros.hpp>
#include "controllers/game_controller.h"
#include "game_state.h"
#include "ecs/game_world.h"
#include "message_log.h"
#include "input_handler.h"
#include "map.h"
#include <memory>

// Mock GameManager for testing
class MockGameManager : public GameManager {
public:
    MockGameManager() : GameManager() {
        // Initialize message log
        message_log_ptr = std::make_unique<MessageLog>();
        input_handler_ptr = std::make_unique<InputHandler>();
    }

    MessageLog* getMessageLog() { return message_log_ptr.get(); }
    InputHandler* getInputHandler() { return input_handler_ptr.get(); }

    // Track state changes
    GameState last_state = GameState::PLAYING;
    bool save_menu_mode = false;

    void setState(GameState state) {
        last_state = state;
        GameManager::setState(state);
    }

    void setSaveMenuMode(bool mode) {
        save_menu_mode = mode;
    }

private:
    std::unique_ptr<MessageLog> message_log_ptr;
    std::unique_ptr<InputHandler> input_handler_ptr;
};

TEST_CASE("GameController Tests", "[controller][game]") {
    auto game_manager = std::make_unique<MockGameManager>();
    MessageLog message_log;
    Map game_map;
    ecs::GameWorld ecs_world(&message_log, &game_map);

    controllers::GameController controller(
        game_manager.get(),
        &ecs_world
    );

    SECTION("Process quit action") {
        bool exit_called = false;

        controllers::GameController::ViewCallbacks callbacks;
        callbacks.exitToMenu = [&]() {
            exit_called = true;
        };
        controller.setViewCallbacks(callbacks);

        // Process quit action
        bool handled = controller.processAction(InputAction::QUIT, ftxui::Event::Character('q'));

        REQUIRE(handled == true);
        REQUIRE(exit_called == true);
        REQUIRE(game_manager->last_state == GameState::MENU);
    }

    SECTION("Process save menu action") {
        bool handled = controller.processAction(InputAction::OPEN_SAVE_MENU, ftxui::Event::Character('S'));

        REQUIRE(handled == true);
        REQUIRE(game_manager->save_menu_mode == true);
        REQUIRE(game_manager->last_state == GameState::SAVE_LOAD);
    }

    SECTION("Process load menu action") {
        bool handled = controller.processAction(InputAction::OPEN_LOAD_MENU, ftxui::Event::Character('L'));

        REQUIRE(handled == true);
        REQUIRE(game_manager->save_menu_mode == false);
        REQUIRE(game_manager->last_state == GameState::SAVE_LOAD);
    }

    SECTION("Process inventory toggle") {
        bool handled = controller.processAction(InputAction::OPEN_INVENTORY, ftxui::Event::Character('i'));

        REQUIRE(handled == true);
        REQUIRE(controller.isInventoryOpen() == true);
        REQUIRE(game_manager->last_state == GameState::INVENTORY);

        // Toggle again to close
        handled = controller.processAction(InputAction::OPEN_INVENTORY, ftxui::Event::Character('i'));

        REQUIRE(handled == true);
        REQUIRE(controller.isInventoryOpen() == false);
        REQUIRE(game_manager->last_state == GameState::PLAYING);
    }

    SECTION("Process help action") {
        bool handled = controller.processAction(InputAction::OPEN_HELP, ftxui::Event::Character('?'));

        REQUIRE(handled == true);
        REQUIRE(game_manager->last_state == GameState::HELP);
    }

    SECTION("Handle directional input when not awaiting") {
        // Should do nothing when not awaiting direction
        controller.handleDirectionalInput(0, -1);

        REQUIRE(controller.isAwaitingDirection() == false);
    }

    SECTION("Authentication info") {
        controller.setAuthenticationInfo(123, "test_token");
        // Just verify it doesn't crash
        REQUIRE(true);
    }

    SECTION("Update function") {
        // Should not crash
        controller.update(0.016f);
        REQUIRE(true);
    }

    SECTION("Show message") {
        std::string last_message;
        bool message_shown = false;

        controllers::GameController::ViewCallbacks callbacks;
        callbacks.showMessage = [&](const std::string& msg) {
            last_message = msg;
            message_shown = true;
        };
        controller.setViewCallbacks(callbacks);

        controller.toggleInventory();

        REQUIRE(message_shown == true);
        REQUIRE(last_message == "Inventory opened.");
    }

    SECTION("Handle unknown action") {
        bool handled = controller.processAction(InputAction::NONE, ftxui::Event::Character('x'));

        REQUIRE(handled == false);
    }

    SECTION("Handle movement actions") {
        // Movement actions should return false (handled by game screen)
        bool handled = controller.processAction(InputAction::MOVE_UP, ftxui::Event::ArrowUp);
        REQUIRE(handled == false);

        handled = controller.processAction(InputAction::MOVE_DOWN, ftxui::Event::ArrowDown);
        REQUIRE(handled == false);
    }
}