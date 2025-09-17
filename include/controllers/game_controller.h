/**
 * @file game_controller.h
 * @brief Controller for main game screen business logic
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <memory>
#include <functional>
#include <string>
#include "input_handler.h"  // For InputAction enum

class GameManager;

namespace ecs {
    class GameWorld;
}

namespace controllers {

/**
 * @class GameController
 * @brief Manages game business logic and coordinates between systems
 *
 * The GameController handles:
 * - Input processing and command execution
 * - State transitions
 * - Coordination between game systems
 * - Inventory management
 * - Direction-based actions
 */
class GameController {
public:
    /**
     * @brief Callbacks for communicating with the view layer
     */
    struct ViewCallbacks {
        std::function<void()> refreshDisplay;
        std::function<void(const std::string&)> showMessage;
        std::function<void(const std::string&)> showPrompt;
        std::function<void()> clearPrompt;
        std::function<void()> exitToMenu;
    };

    /**
     * @brief Constructor
     * @param game_manager Game manager reference
     * @param ecs_world ECS world reference
     */
    explicit GameController(GameManager* game_manager, ecs::GameWorld* ecs_world);

    ~GameController() = default;

    /**
     * @brief Set view callbacks for UI updates
     * @param callbacks View callback functions
     */
    void setViewCallbacks(const ViewCallbacks& callbacks) {
        view_callbacks = callbacks;
    }

    /**
     * @brief Handle input event
     * @param event Input event from the view
     * @return true if event was handled
     */
    bool handleInput(const ftxui::Event& event);

    /**
     * @brief Process an input action
     * @param action Input action to process
     * @param event Original event for additional context
     * @return true if action was processed
     */
    bool processAction(InputAction action, const ftxui::Event& event);

    /**
     * @brief Handle directional input
     * @param dx X direction (-1, 0, 1)
     * @param dy Y direction (-1, 0, 1)
     */
    void handleDirectionalInput(int dx, int dy);

    /**
     * @brief Check if waiting for directional input
     * @return true if awaiting direction
     */
    bool isAwaitingDirection() const { return awaiting_direction; }

    /**
     * @brief Get the current direction prompt
     * @return Prompt message or empty if not awaiting
     */
    std::string getDirectionPrompt() const { return direction_prompt; }

    /**
     * @brief Set authentication information
     * @param user_id User ID for authenticated player
     * @param session_token Session token
     */
    void setAuthenticationInfo(int user_id, const std::string& session_token);

    /**
     * @brief Update game state
     * @param delta_time Time since last update
     */
    void update(float delta_time);

    /**
     * @brief Toggle inventory display
     */
    void toggleInventory();

    /**
     * @brief Check if inventory is open
     * @return true if inventory is displayed
     */
    bool isInventoryOpen() const { return inventory_open; }

private:
    // Core references
    GameManager* game_manager;
    ecs::GameWorld* ecs_world;

    // View callbacks
    ViewCallbacks view_callbacks;

    // Authentication
    int auth_user_id = 0;
    std::string auth_session_token;

    // State
    bool awaiting_direction = false;
    std::string direction_prompt;
    bool inventory_open = false;
    InputAction pending_action = InputAction::NONE;


    // Helper methods
    void startDirectionalAction(InputAction action, const std::string& prompt);
    void cancelDirectionalAction();
    void executeDirectionalCommand(int dx, int dy);
    void showMessage(const std::string& msg);
    void showPrompt(const std::string& prompt);
    void clearPrompt();
};

} // namespace controllers