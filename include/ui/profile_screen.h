#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>
#include <memory>

// Forward declarations
class GameManager;
namespace db {
    class PlayerRepository;
}

/**
 * @class ProfileScreen
 * @brief Player profile and statistics screen
 *
 * Displays:
 * - Player information
 * - Game statistics
 * - Achievement progress
 * - Leaderboard rankings
 * - Character history
 */
class ProfileScreen {
public:
    struct CharacterRecord {
        std::string name;
        int level;
        std::string class_name;
        int depth_reached;
        int monsters_killed;
        int gold_collected;
        std::string death_cause;
        std::string play_time;
        std::string date_played;
    };

    struct Achievement {
        std::string name;
        std::string description;
        bool unlocked;
        std::string unlock_date;
        int points;
    };

    struct LeaderboardEntry {
        int rank;
        std::string player_name;
        int score;
        int depth;
        std::string date;
    };

    /**
     * @brief Constructor
     * @param game_manager Reference to game manager
     * @param player_repo Reference to player repository (may be null)
     */
    ProfileScreen(GameManager* game_manager,
                  db::PlayerRepository* player_repo = nullptr);

    ~ProfileScreen() = default;

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
     * @brief Load profile data for current user
     */
    void loadProfileData();

private:
    GameManager* game_manager;
    db::PlayerRepository* player_repo;

    // UI State
    int selected_tab = 0;
    int selected_character = 0;
    int selected_achievement_page = 0;

    // Profile data
    struct {
        std::string username;
        std::string title;  // e.g., "Dungeon Explorer", "Dragon Slayer"
        int total_playtime_hours;
        int total_games;
        int win_count;
        double win_rate;
        int current_streak;
        int best_streak;
    } profile_info;

    // Statistics
    struct {
        int total_monsters_killed;
        int total_gold_collected;
        int total_items_found;
        int total_depths_explored;
        int highest_level_reached;
        int deepest_depth_reached;
        int total_potions_consumed;
        int total_scrolls_read;
        int total_damage_dealt;
        int total_damage_received;
        std::string most_killed_monster;
        std::string most_common_death;
    } statistics;

    // Collections
    std::vector<CharacterRecord> character_history;
    std::vector<Achievement> achievements;
    std::vector<LeaderboardEntry> personal_bests;
    std::vector<LeaderboardEntry> global_rankings;

    // Helper methods
    ftxui::Component createOverviewTab();
    ftxui::Component createStatisticsTab();
    ftxui::Component createCharactersTab();
    ftxui::Component createAchievementsTab();
    ftxui::Component createLeaderboardTab();

    std::string formatPlaytime(int hours) const;
    std::string formatNumber(int num) const;
    ftxui::Color getTitleColor(const std::string& title) const;
};