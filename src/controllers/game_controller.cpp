#include "controllers/game_controller.h"
#include "game_state.h"
#include "ecs/game_world.h"
#include "ecs/component.h"
#include "message_log.h"
#include "log.h"
#include <ftxui/component/event.hpp>

namespace controllers {

GameController::GameController(GameManager* gm, ecs::GameWorld* world)
    : game_manager(gm)
    , ecs_world(world) {
    // Controller focuses on business logic, not rendering
}

bool GameController::handleInput(const ftxui::Event& event) {
    // Convert event to action using InputHandler
    InputHandler* input_handler = game_manager->getInputHandler();
    if (!input_handler) {
        return false;
    }

    InputAction action = input_handler->processEvent(event);

    // Check if we're waiting for directional input
    if (awaiting_direction) {
        // Handle direction keys
        if (event == ftxui::Event::ArrowUp || event == ftxui::Event::Character('k')) {
            handleDirectionalInput(0, -1);
            return true;
        }
        if (event == ftxui::Event::ArrowDown || event == ftxui::Event::Character('j')) {
            handleDirectionalInput(0, 1);
            return true;
        }
        if (event == ftxui::Event::ArrowLeft || event == ftxui::Event::Character('h')) {
            handleDirectionalInput(-1, 0);
            return true;
        }
        if (event == ftxui::Event::ArrowRight || event == ftxui::Event::Character('l')) {
            handleDirectionalInput(1, 0);
            return true;
        }

        // Diagonal movement (numpad)
        if (event == ftxui::Event::Character('7')) {
            handleDirectionalInput(-1, -1);
            return true;
        }
        if (event == ftxui::Event::Character('9')) {
            handleDirectionalInput(1, -1);
            return true;
        }
        if (event == ftxui::Event::Character('1')) {
            handleDirectionalInput(-1, 1);
            return true;
        }
        if (event == ftxui::Event::Character('3')) {
            handleDirectionalInput(1, 1);
            return true;
        }

        // Cancel
        if (event == ftxui::Event::Escape) {
            cancelDirectionalAction();
            return true;
        }

        return true; // Consume all input while awaiting direction
    }

    // Inventory is toggled through actions, not special mode handling here

    // Normal game input - process the action
    return processAction(action, event);
}

bool GameController::processAction(InputAction action, const ftxui::Event& /*event*/) {
    if (action == InputAction::NONE) {
        return false;
    }

    // Handle state transitions
    switch (action) {
        case InputAction::QUIT:
            if (game_manager) {
                game_manager->setState(GameState::MENU);
            }
            if (view_callbacks.exitToMenu) {
                view_callbacks.exitToMenu();
            }
            return true;

        case InputAction::OPEN_SAVE_MENU:
            if (game_manager) {
                game_manager->setSaveMenuMode(true);
                game_manager->setState(GameState::SAVE_LOAD);
            }
            return true;

        case InputAction::OPEN_LOAD_MENU:
            if (game_manager) {
                game_manager->setSaveMenuMode(false);
                game_manager->setState(GameState::SAVE_LOAD);
            }
            return true;

        case InputAction::OPEN_INVENTORY:
            toggleInventory();
            return true;

        case InputAction::OPEN_HELP:
            if (game_manager) {
                game_manager->setState(GameState::HELP);
            }
            return true;

        // Door interaction is handled through OPEN_DOOR action
        case InputAction::OPEN_DOOR:
            // The current implementation handles open/close toggle
            // This will be handled by the game screen for now
            return false;

        default:
            break;
    }

    // Handle movement and other actions through game manager
    // Let the game manager handle it like the current implementation
    return false;  // Will be handled by GameScreen for now
}

void GameController::handleDirectionalInput(int dx, int dy) {
    if (!awaiting_direction) {
        return;
    }

    executeDirectionalCommand(dx, dy);
    cancelDirectionalAction();
}

void GameController::startDirectionalAction(InputAction action, const std::string& prompt) {
    awaiting_direction = true;
    pending_action = action;
    direction_prompt = prompt;
    showPrompt(prompt);
}

void GameController::cancelDirectionalAction() {
    awaiting_direction = false;
    pending_action = InputAction::NONE;
    direction_prompt.clear();
    clearPrompt();
}

void GameController::executeDirectionalCommand(int dx, int dy) {
    if (!ecs_world) {
        return;
    }

    // Get player position from GameManager for now
    // TODO: Use ECS when properly integrated
    if (!game_manager) {
        return;
    }

    [[maybe_unused]] int target_x = game_manager->player_x + dx;
    [[maybe_unused]] int target_y = game_manager->player_y + dy;

    // Execute the pending action at the target location
    switch (pending_action) {
        case InputAction::OPEN_DOOR:
            showMessage("You interact with the door.");
            // TODO: Implement door interaction logic
            break;

        default:
            break;
    }
}

void GameController::setAuthenticationInfo(int user_id, const std::string& session_token) {
    auth_user_id = user_id;
    auth_session_token = session_token;

    // Pass to ECS world if needed
    if (ecs_world) {
        // TODO: Set player authentication in ECS
    }
}

void GameController::update(float /*delta_time*/) {
    // Update game systems that need per-frame updates
    // Actual updates delegated to appropriate systems
}

void GameController::toggleInventory() {
    inventory_open = !inventory_open;

    if (game_manager) {
        game_manager->setState(inventory_open ? GameState::INVENTORY : GameState::PLAYING);
    }

    if (inventory_open) {
        showMessage("Inventory opened.");
    } else {
        showMessage("Inventory closed.");
    }

    if (view_callbacks.refreshDisplay) {
        view_callbacks.refreshDisplay();
    }
}

void GameController::showMessage(const std::string& msg) {
    if (game_manager && game_manager->getMessageLog()) {
        game_manager->getMessageLog()->addMessage(msg);
    }

    if (view_callbacks.showMessage) {
        view_callbacks.showMessage(msg);
    }
}

void GameController::showPrompt(const std::string& prompt) {
    showMessage(prompt);

    if (view_callbacks.showPrompt) {
        view_callbacks.showPrompt(prompt);
    }
}

void GameController::clearPrompt() {
    if (view_callbacks.clearPrompt) {
        view_callbacks.clearPrompt();
    }
}

} // namespace controllers