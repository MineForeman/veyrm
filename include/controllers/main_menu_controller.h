/**
 * @file main_menu_controller.h
 * @brief Controller for main menu business logic
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <memory>
#include <functional>

// Forward declarations
class GameManager;
class LoginScreen;
namespace auth {
    class AuthenticationService;
}
namespace db {
    class DatabaseManager;
}

namespace controllers {

/**
 * @class MainMenuController
 * @brief Handles main menu business logic and game state transitions
 */
class MainMenuController {
public:
    /**
     * @brief Menu options for authenticated users
     */
    enum class AuthenticatedOption {
        NEW_GAME = 0,
        CONTINUE,
        CLOUD_SAVES,
        LEADERBOARDS,
        SETTINGS,
        PROFILE,
        LOGOUT,
        ABOUT,
        QUIT
    };

    /**
     * @brief Menu options for unauthenticated users
     */
    enum class UnauthenticatedOption {
        LOGIN = 0,
        REGISTER,
        ABOUT,
        QUIT
    };

    /**
     * @brief Callbacks for view layer communication
     */
    struct ViewCallbacks {
        std::function<void(const std::string&)> showMessage;
        std::function<void(const std::string&)> showError;
        std::function<void()> refreshMenu;
        std::function<void()> exitApplication;
    };

    /**
     * @brief Constructor
     * @param game_manager Game manager reference
     * @param auth_service Authentication service (optional)
     * @param login_screen Login screen for authentication flow (optional)
     */
    MainMenuController(GameManager* game_manager,
                      auth::AuthenticationService* auth_service = nullptr,
                      LoginScreen* login_screen = nullptr);

    ~MainMenuController();

    /**
     * @brief Set view callbacks for UI updates
     * @param callbacks View callback functions
     */
    void setViewCallbacks(const ViewCallbacks& callbacks);

    /**
     * @brief Check if user is authenticated
     * @return True if user is logged in
     */
    bool isAuthenticated() const;

    /**
     * @brief Get current user ID
     * @return User ID or 0 if not authenticated
     */
    int getUserId() const;

    /**
     * @brief Get current username
     * @return Username or empty string if not authenticated
     */
    std::string getUsername() const;

    /**
     * @brief Get authentication status string
     * @return Status string for display
     */
    std::string getAuthStatus() const;

    /**
     * @brief Set authentication info from main login
     * @param user_id User ID from authentication
     * @param session_token Session token from authentication
     * @param username Username from authentication
     */
    void setAuthenticationInfo(int user_id, const std::string& session_token, const std::string& username);

    /**
     * @brief Handle menu selection for authenticated user
     * @param option Selected menu option
     */
    void handleAuthenticatedSelection(AuthenticatedOption option);

    /**
     * @brief Handle menu selection for unauthenticated user
     * @param option Selected menu option
     */
    void handleUnauthenticatedSelection(UnauthenticatedOption option);

    /**
     * @brief Process login attempt
     * @return True if login successful
     */
    bool processLogin();

    /**
     * @brief Process registration attempt
     * @return True if registration successful
     */
    bool processRegistration();

    /**
     * @brief Process logout
     */
    void logout();

    /**
     * @brief Toggle about section visibility
     */
    void toggleAbout();

    /**
     * @brief Check if about section should be shown
     * @return True if about is visible
     */
    bool isAboutVisible() const { return show_about; }

    /**
     * @brief Start new game
     */
    void startNewGame();

    /**
     * @brief Continue saved game
     */
    void continueSavedGame();

    /**
     * @brief Open cloud saves menu
     */
    void openCloudSaves();

    /**
     * @brief Open leaderboards
     */
    void openLeaderboards();

    /**
     * @brief Open settings menu
     */
    void openSettings();

    /**
     * @brief Open player profile
     */
    void openProfile();

    /**
     * @brief Quit application
     */
    void quitApplication();

private:
    GameManager* game_manager;
    auth::AuthenticationService* auth_service;
    LoginScreen* login_screen;

    // Authentication state
    int user_id;
    std::string session_token;
    std::string username;

    // UI state
    bool show_about;

    // View callbacks
    ViewCallbacks view_callbacks;

    /**
     * @brief Fetch username from database after authentication
     */
    void fetchUsernameFromDatabase();

    /**
     * @brief Clear authentication data
     */
    void clearAuthenticationData();
};

} // namespace controllers