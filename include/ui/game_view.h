/**
 * @file game_view.h
 * @brief Pure view component for game screen
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <string>
#include <functional>
#include <vector>

class MapRenderer;
class StatusBar;
class InventoryRenderer;
class GameManager;

namespace ui {

/**
 * @class GameView
 * @brief Pure view component for the main game display
 *
 * The GameView class handles all UI rendering for the game screen,
 * including the map, status bar, message log, and inventory.
 * It follows the MVC pattern as a pure view with no business logic.
 */
class GameView {
public:
    /**
     * @brief Controller callbacks for handling user input
     */
    struct ControllerCallbacks {
        std::function<bool(const ftxui::Event&)> onInput;
        std::function<void()> onRefresh;
        std::function<void()> onExit;
    };

    /**
     * @brief View modes
     */
    enum class Mode {
        NORMAL,     ///< Normal gameplay view
        INVENTORY,  ///< Inventory overlay
        HELP,       ///< Help overlay
        DIALOG      ///< Dialog/prompt overlay
    };

    /**
     * @brief Constructor
     * @param game_manager Game manager for state access
     * @param screen Screen reference for rendering
     */
    GameView(GameManager* game_manager, ftxui::ScreenInteractive* screen);

    /**
     * @brief Destructor
     */
    ~GameView();

    /**
     * @brief Set controller callbacks
     * @param callbacks Callback functions
     */
    void setControllerCallbacks(const ControllerCallbacks& callbacks) {
        controller_callbacks = callbacks;
    }

    /**
     * @brief Create the main UI component
     * @return FTXUI component
     */
    ftxui::Component createComponent();

    /**
     * @brief Set the current view mode
     * @param mode View mode to set
     */
    void setMode(Mode mode) { current_mode = mode; }

    /**
     * @brief Get the current view mode
     * @return Current mode
     */
    Mode getMode() const { return current_mode; }

    /**
     * @brief Show a message in the UI
     * @param message Message to display
     */
    void showMessage(const std::string& message);

    /**
     * @brief Show a prompt/dialog
     * @param prompt Prompt text
     */
    void showPrompt(const std::string& prompt);

    /**
     * @brief Clear any active prompt
     */
    void clearPrompt();

    /**
     * @brief Update the display
     */
    void refresh();

    /**
     * @brief Show/hide inventory overlay
     * @param show true to show, false to hide
     */
    void showInventory(bool show);

    /**
     * @brief Update status display
     */
    void updateStatus();

    /**
     * @brief Update map display
     */
    void updateMap();

private:
    // Core components
    GameManager* game_manager;
    ftxui::ScreenInteractive* screen_ref;

    // View state
    Mode current_mode = Mode::NORMAL;
    std::string current_prompt;
    std::vector<std::string> message_buffer;
    bool show_prompt = false;

    // Callbacks
    ControllerCallbacks controller_callbacks;

    // Renderers
    std::unique_ptr<MapRenderer> map_renderer;
    std::unique_ptr<StatusBar> status_bar;
    std::unique_ptr<InventoryRenderer> inventory_renderer;

    // Component creation helpers
    ftxui::Component createMapPanel();
    ftxui::Component createStatusPanel();
    ftxui::Component createMessagePanel();
    ftxui::Component createInventoryOverlay();
    ftxui::Component createHelpOverlay();
    ftxui::Component createPromptOverlay();

    // Layout helpers
    ftxui::Element renderGameLayout();
    ftxui::Element renderNormalMode();
    ftxui::Element renderInventoryMode();
    ftxui::Element renderHelpMode();
    ftxui::Element renderDialogMode();
};

} // namespace ui