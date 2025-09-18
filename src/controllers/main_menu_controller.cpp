#include "controllers/main_menu_controller.h"
#include "game_state.h"
#include "login_screen.h"
#include "auth/authentication_service.h"
#include "db/database_manager.h"
#include "log.h"
#include <sstream>

namespace controllers {

MainMenuController::MainMenuController(GameManager* game_manager,
                                     auth::AuthenticationService* auth_service,
                                     LoginScreen* login_screen)
    : game_manager(game_manager)
    , auth_service(auth_service)
    , login_screen(login_screen)
    , user_id(0)
    , show_about(false) {
}

MainMenuController::~MainMenuController() = default;

void MainMenuController::setViewCallbacks(const ViewCallbacks& callbacks) {
    view_callbacks = callbacks;
}

bool MainMenuController::isAuthenticated() const {
    return user_id > 0 && !session_token.empty();
}

int MainMenuController::getUserId() const {
    return user_id;
}

std::string MainMenuController::getUsername() const {
    return username;
}

std::string MainMenuController::getAuthStatus() const {
    if (isAuthenticated()) {
        return " | Logged in as: " + username;
    }
    return "";
}

void MainMenuController::setAuthenticationInfo(int user_id, const std::string& session_token, const std::string& username) {
    this->user_id = user_id;
    this->session_token = session_token;
    this->username = username;
    LOG_INFO("MainMenuController authentication updated: " + username + " (ID=" + std::to_string(user_id) + ")");
}

void MainMenuController::handleAuthenticatedSelection(AuthenticatedOption option) {
    switch (option) {
        case AuthenticatedOption::NEW_GAME:
            startNewGame();
            break;
        case AuthenticatedOption::CONTINUE:
            continueSavedGame();
            break;
        case AuthenticatedOption::CLOUD_SAVES:
            openCloudSaves();
            break;
        case AuthenticatedOption::LEADERBOARDS:
            openLeaderboards();
            break;
        case AuthenticatedOption::SETTINGS:
            openSettings();
            break;
        case AuthenticatedOption::PROFILE:
            openProfile();
            break;
        case AuthenticatedOption::LOGOUT:
            logout();
            break;
        case AuthenticatedOption::ABOUT:
            toggleAbout();
            break;
        case AuthenticatedOption::QUIT:
            quitApplication();
            break;
    }
}

void MainMenuController::handleUnauthenticatedSelection(UnauthenticatedOption option) {
    switch (option) {
        case UnauthenticatedOption::LOGIN:
            processLogin();
            break;
        case UnauthenticatedOption::REGISTER:
            processRegistration();
            break;
        case UnauthenticatedOption::ABOUT:
            toggleAbout();
            break;
        case UnauthenticatedOption::QUIT:
            quitApplication();
            break;
    }
}

bool MainMenuController::processLogin() {
    if (!login_screen) {
        LOG_ERROR("LoginScreen not initialized - check database connection");
        if (view_callbacks.showError) {
            view_callbacks.showError("Login system unavailable");
        }
        return false;
    }

    LOG_INFO("Login option selected");
    login_screen->setMode(LoginScreen::Mode::LOGIN);
    LOG_INFO("Launching LoginScreen in LOGIN mode");

    // Run the login screen synchronously
    auto result = login_screen->run();
    if (result == LoginScreen::Result::SUCCESS) {
        user_id = login_screen->getUserId();
        session_token = login_screen->getSessionToken();
        fetchUsernameFromDatabase();

        LOG_INFO("User authenticated successfully: " + username + " (ID=" + std::to_string(user_id) + ")");

        // Transition to main menu after successful authentication
        game_manager->setState(GameState::MENU);

        if (view_callbacks.showMessage) {
            view_callbacks.showMessage("Welcome back, " + username + "!");
        }

        if (view_callbacks.refreshMenu) {
            view_callbacks.refreshMenu();
        }

        return true;
    }

    return false;
}

bool MainMenuController::processRegistration() {
    if (!login_screen) {
        LOG_ERROR("LoginScreen not initialized - check database connection");
        if (view_callbacks.showError) {
            view_callbacks.showError("Registration system unavailable");
        }
        return false;
    }

    LOG_INFO("Register option selected");
    login_screen->setMode(LoginScreen::Mode::REGISTER);
    LOG_INFO("Launching LoginScreen in REGISTER mode");

    // Run the login screen synchronously
    auto result = login_screen->run();
    if (result == LoginScreen::Result::SUCCESS) {
        user_id = login_screen->getUserId();
        session_token = login_screen->getSessionToken();
        fetchUsernameFromDatabase();

        LOG_INFO("User registered successfully: " + username + " (ID=" + std::to_string(user_id) + ")");

        // Transition to main menu after successful registration
        game_manager->setState(GameState::MENU);

        if (view_callbacks.showMessage) {
            view_callbacks.showMessage("Welcome to Veyrm, " + username + "!");
        }

        if (view_callbacks.refreshMenu) {
            view_callbacks.refreshMenu();
        }

        return true;
    }

    return false;
}

void MainMenuController::logout() {
    LOG_INFO("User logging out: " + username);
    clearAuthenticationData();

    if (view_callbacks.showMessage) {
        view_callbacks.showMessage("Logged out successfully");
    }

    if (view_callbacks.refreshMenu) {
        view_callbacks.refreshMenu();
    }
}

void MainMenuController::toggleAbout() {
    show_about = !show_about;
}

void MainMenuController::startNewGame() {
    LOG_INFO("Starting new game");
    game_manager->setState(GameState::PLAYING);

    // Auto-save after successful game start
    game_manager->autoSave();
}

void MainMenuController::continueSavedGame() {
    LOG_INFO("*** CONTINUE BUTTON PRESSED ***");
    LOG_INFO("Auto-restoring saved game");
    if (game_manager->autoRestore()) {
        LOG_INFO("*** AUTO-RESTORE SUCCEEDED - SWITCHING TO PLAYING STATE ***");
        game_manager->setState(GameState::PLAYING);
    } else {
        LOG_ERROR("*** AUTO-RESTORE FAILED ***");
        LOG_ERROR("Failed to restore saved game");
        // Stay on menu if restore fails
    }
}

void MainMenuController::openCloudSaves() {
    LOG_INFO("Cloud saves not yet implemented");
    if (view_callbacks.showMessage) {
        view_callbacks.showMessage("Cloud saves coming soon!");
    }
}

void MainMenuController::openLeaderboards() {
    LOG_INFO("Leaderboards not yet implemented");
    if (view_callbacks.showMessage) {
        view_callbacks.showMessage("Leaderboards coming soon!");
    }
}

void MainMenuController::openSettings() {
    LOG_INFO("Settings not yet implemented");
    if (view_callbacks.showMessage) {
        view_callbacks.showMessage("Settings menu coming soon!");
    }
}

void MainMenuController::openProfile() {
    LOG_INFO("Profile not yet implemented");
    if (view_callbacks.showMessage) {
        view_callbacks.showMessage("Profile screen coming soon!");
    }
}

void MainMenuController::quitApplication() {
    LOG_INFO("Quitting application");

    // Auto-save before quit if currently playing
    if (game_manager->getCurrentState() == GameState::PLAYING) {
        game_manager->autoSave();
    }

    game_manager->setState(GameState::QUIT);

    if (view_callbacks.exitApplication) {
        view_callbacks.exitApplication();
    }
}

void MainMenuController::fetchUsernameFromDatabase() {
    if (user_id <= 0) {
        return;
    }

    try {
        username = db::DatabaseManager::getInstance().executeQuery([this](db::Connection& conn) {
            auto result = conn.execParams(
                "SELECT username FROM users WHERE id = $1",
                {std::to_string(user_id)}
            );
            if (result.isOk() && result.numRows() > 0) {
                return result.getValue(0, 0);
            }
            return std::string("");
        });
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get username: " + std::string(e.what()));
        username = "User" + std::to_string(user_id);
    }
}

void MainMenuController::clearAuthenticationData() {
    user_id = 0;
    session_token.clear();
    username.clear();
}

} // namespace controllers