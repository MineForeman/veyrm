/**
 * @file main_menu_view.h
 * @brief View component for main menu UI
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>
#include <functional>

namespace ui {

/**
 * @class MainMenuView
 * @brief Pure view component for main menu display
 */
class MainMenuView {
public:
    /**
     * @brief Result of menu interaction
     */
    enum class Result {
        NONE,
        SELECTION_MADE,
        CANCELLED
    };

    /**
     * @brief Controller callbacks for handling user actions
     */
    struct ControllerCallbacks {
        std::function<void(int)> onMenuSelect;
        std::function<void()> onAboutToggle;
        std::function<void()> onExit;
        std::function<bool()> isAuthenticated;
        std::function<std::string()> getUsername;
        std::function<std::string()> getAuthStatus;
    };

    /**
     * @brief Constructor
     */
    MainMenuView();

    ~MainMenuView();

    /**
     * @brief Set controller callbacks
     * @param callbacks Callback functions
     */
    void setControllerCallbacks(const ControllerCallbacks& callbacks);

    /**
     * @brief Run the menu view
     * @return Result of menu interaction
     */
    Result run();

    /**
     * @brief Get selected menu index
     * @return Currently selected menu item
     */
    int getSelectedIndex() const { return selected_index; }

    /**
     * @brief Set whether to show authenticated or unauthenticated menu
     * @param authenticated True for authenticated menu
     */
    void setAuthenticated(bool authenticated);

    /**
     * @brief Show status message
     * @param message Status message to display
     */
    void showMessage(const std::string& message);

    /**
     * @brief Show error message
     * @param error Error message to display
     */
    void showError(const std::string& error);

    /**
     * @brief Refresh the menu display
     */
    void refresh();

    /**
     * @brief Set whether to show about section
     * @param show True to show about section
     */
    void setShowAbout(bool show) { show_about = show; }

    /**
     * @brief Exit the menu loop
     */
    void exit();

private:
    // UI state
    int selected_index;
    bool is_authenticated;
    bool show_about;
    Result result;

    // Messages
    std::string status_message;
    std::string error_message;
    bool show_status;
    bool show_error;

    // Menu items
    std::vector<std::string> menu_entries;

    // FTXUI components
    ftxui::ScreenInteractive screen;
    ftxui::Component main_component;

    // Controller callbacks
    ControllerCallbacks controller_callbacks;

    /**
     * @brief Create the main menu component
     * @return FTXUI component
     */
    ftxui::Component createMainComponent();

    /**
     * @brief Create the title banner
     * @return FTXUI element for title
     */
    ftxui::Element createTitleBanner() const;

    /**
     * @brief Create the about section
     * @return FTXUI element for about box
     */
    ftxui::Element createAboutBox() const;

    /**
     * @brief Update menu entries based on authentication state
     */
    void updateMenuEntries();

    /**
     * @brief Handle menu selection
     */
    void handleSelection();

    /**
     * @brief Clear all messages
     */
    void clearMessages();
};

} // namespace ui