/**
 * @file game_screen.h
 * @brief Main game screen UI component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include "game_state.h"
#include "input_handler.h"
#include <memory>

class MapRenderer;
class StatusBar;
class LayoutSystem;
class InventoryRenderer;

namespace controllers {
    class GameController;
}

namespace ui {
    class GameView;
}

/**
 * @class GameScreen
 * @brief Main game screen UI component that orchestrates all game display elements
 *
 * The GameScreen class creates and manages the primary game interface, combining
 * the map view, status information, message log, and inventory display into a
 * cohesive FTXUI component. It handles input processing, layout management,
 * and coordinates between different UI systems.
 *
 * UI Components managed:
 * - Map panel (game world visualization)
 * - Status panel (player stats, HP, etc.)
 * - Message log panel (game events)
 * - Inventory panel (items and equipment)
 *
 * @see MapRenderer
 * @see StatusBar
 * @see LayoutSystem
 * @see InventoryRenderer
 */
class GameScreen {
public:
    /**
     * @brief Construct game screen
     * @param manager Game manager for state access
     * @param screen FTXUI interactive screen reference
     */
    GameScreen(GameManager* manager, ftxui::ScreenInteractive* screen);

    /**
     * @brief Destructor
     * @note Required due to forward-declared renderer types
     */
    ~GameScreen();

    /**
     * @brief Create the main game UI component
     * @return Complete FTXUI component for game display
     */
    ftxui::Component Create();

    /**
     * @brief Set authentication information for player creation
     * @param user_id Database user ID (0 for guest)
     * @param session_token Authentication session token
     */
    void setAuthenticationInfo(int user_id, const std::string& session_token);

private:
    // Authentication state
    int auth_user_id = 0;
    std::string auth_session_token;
    GameManager* game_manager;                              ///< Game state manager
    [[maybe_unused]] ftxui::ScreenInteractive* screen_ref; ///< Screen reference
    std::unique_ptr<MapRenderer> renderer;                  ///< Map rendering system
    std::unique_ptr<StatusBar> status_bar;                  ///< Status bar component
    std::unique_ptr<LayoutSystem> layout_system;            ///< Layout management
    std::unique_ptr<InventoryRenderer> inventory_renderer;  ///< Inventory display

    // MVC components
    std::unique_ptr<controllers::GameController> controller; ///< Game controller for business logic
    std::unique_ptr<ui::GameView> view;                     ///< Game view for UI rendering

    // Directional action state
    bool awaiting_direction = false;                        ///< Waiting for direction input
    std::string direction_prompt;                           ///< Current direction prompt message

    // UI component creation methods

    /**
     * @brief Create map display panel
     * @return FTXUI component for map visualization
     */
    ftxui::Component CreateMapPanel();

    /**
     * @brief Create message log panel
     * @return FTXUI component for game messages
     */
    ftxui::Component CreateLogPanel();

    /**
     * @brief Create status display panel
     * @return FTXUI component for player status
     */
    ftxui::Component CreateStatusPanel();

    /**
     * @brief Create inventory panel
     * @return FTXUI component for inventory display
     */
    ftxui::Component CreateInventoryPanel();

    /**
     * @brief Update layout based on terminal size changes
     * @note Called automatically when terminal is resized
     */
    void updateLayout();

    /**
     * @brief Handle player movement and combat
     * @param dx X-axis movement delta
     * @param dy Y-axis movement delta
     * @param direction Movement direction name (for messages)
     * @return true if action was processed
     */
    bool handlePlayerMovement(int dx, int dy, const std::string& direction);

    /**
     * @brief Handle door opening/closing interactions
     * @return true if door interaction occurred
     */
    bool handleDoorInteraction();

    /**
     * @brief Handle directional door opening/closing
     * @param dx X direction (-1, 0, 1)
     * @param dy Y direction (-1, 0, 1)
     * @return true if door interaction occurred
     */
    bool handleDirectionalDoorInteraction(int dx, int dy);

    /**
     * @brief Handle stair usage for level transitions
     * @param going_down true for down stairs, false for up stairs
     * @return true if stair interaction occurred
     */
    bool handleStairInteraction(bool going_down);

    /**
     * @brief Process inventory-related input
     * @param action The input action to process
     * @param event Raw FTXUI event data
     * @return true if input was handled
     */
    bool handleInventoryInput(InputAction action, const ftxui::Event& event);
};