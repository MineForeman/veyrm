#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <memory>
#include <functional>

// Forward declarations
namespace auth {
    class AuthenticationService;
}

namespace db {
    class PlayerRepository;
}

class GameManager;

/**
 * @class AccountScreen
 * @brief Account management screen for authenticated users
 *
 * Provides functionality for:
 * - Viewing account information
 * - Changing password
 * - Updating email
 * - Viewing statistics
 * - Managing cloud save settings
 * - Account deletion
 */
class AccountScreen {
public:
    /**
     * @brief Constructor
     * @param game_manager Reference to game manager
     * @param auth_service Reference to authentication service (may be null)
     * @param player_repo Reference to player repository (may be null)
     */
    AccountScreen(GameManager* game_manager,
                  auth::AuthenticationService* auth_service = nullptr,
                  db::PlayerRepository* player_repo = nullptr);

    ~AccountScreen() = default;

    /**
     * @brief Create the FTXUI component
     * @return Component for rendering and handling input
     */
    ftxui::Component create();

    /**
     * @brief Handle input events
     * @param event The input event to process
     * @return true if event was handled
     */
    bool handleInput(ftxui::Event event);

    /**
     * @brief Refresh account data from database
     */
    void refreshAccountData();

private:
    GameManager* game_manager;
    auth::AuthenticationService* auth_service;
    db::PlayerRepository* player_repo;

    // UI State
    int selected_tab = 0;
    bool show_change_password = false;
    bool show_change_email = false;
    bool show_delete_confirmation = false;

    // Form fields
    std::string current_password;
    std::string new_password;
    std::string confirm_password;
    std::string new_email;

    // Account data
    struct AccountInfo {
        int user_id = 0;
        std::string username;
        std::string email;
        std::string created_date;
        std::string last_login;
        int total_playtime_hours = 0;
        int total_characters = 0;
        int cloud_saves_used = 0;
        int cloud_saves_limit = 10;
        bool email_verified = false;
    } account_info;

    // Statistics data
    struct PlayerStats {
        int total_games_played = 0;
        int highest_level = 0;
        int deepest_depth = 0;
        int total_monsters_killed = 0;
        int total_gold_collected = 0;
        int total_items_found = 0;
        int achievements_unlocked = 0;
        int achievements_total = 0;
        std::string favorite_character_class;
        double average_game_duration_minutes = 0;
    } player_stats;

    // Messages
    std::string status_message;
    std::string error_message;
    bool show_status = false;
    bool show_error = false;

    // Helper methods
    void handleChangePassword();
    void handleChangeEmail();
    void handleDeleteAccount();
    void loadAccountInfo();
    void loadStatistics();

    ftxui::Component createAccountInfoTab();
    ftxui::Component createStatisticsTab();
    ftxui::Component createSettingsTab();
    ftxui::Component createSecurityTab();
};