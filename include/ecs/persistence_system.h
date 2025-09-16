#pragma once

#include "ecs/entity.h"
#include "ecs/component.h"
#include "db/database_manager.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <vector>
#include <chrono>

namespace ecs {

class World;
class Entity;

// System responsible for persisting ECS data to database
class PersistenceSystem {
private:
    db::DatabaseManager* db = nullptr;
    bool enabled = false;

public:
    PersistenceSystem();
    ~PersistenceSystem() = default;

    void update(float deltaTime, World& world);

    // Character persistence
    struct CharacterData {
        std::string character_id;
        std::string player_id;
        std::string name;
        int level;
        int experience;
        int gold;
        int current_depth;
        nlohmann::json game_state;
    };

    // Save current game state
    bool saveCharacter(World& world, Entity& player_entity, const std::string& character_id);

    // Load game state
    std::optional<Entity*> loadCharacter(World& world, const std::string& character_id);

    // Save a specific entity to database
    bool saveEntity(World& world, Entity& entity);

    // Load entities from database
    std::vector<Entity*> loadEntities(World& world, const std::string& character_id);

    // Monster and item data persistence
    bool saveMonsterTemplate(const nlohmann::json& monster_data);
    bool saveItemTemplate(const nlohmann::json& item_data);

    std::optional<nlohmann::json> loadMonsterTemplate(const std::string& monster_code);
    std::optional<nlohmann::json> loadItemTemplate(const std::string& item_code);

    // Telemetry
    void logEvent(const std::string& event_type, const nlohmann::json& event_data);
    void logEncounter(EntityID player, EntityID monster, const nlohmann::json& details);
    void logItemPickup(EntityID player, const std::string& item_code, int quantity);

    // Leaderboard
    struct LeaderboardEntry {
        std::string player_name;
        int score;
        int depth_reached;
        int play_time;
        std::string death_reason;
        std::chrono::system_clock::time_point submitted_at;
    };

    bool submitScore(const LeaderboardEntry& entry);
    std::vector<LeaderboardEntry> getLeaderboard(int limit = 100, int offset = 0);
    std::optional<int> getPlayerRank(const std::string& player_id);

    // Achievements
    struct Achievement {
        std::string code;
        std::string name;
        std::string description;
        int points;
        bool unlocked;
        std::optional<std::chrono::system_clock::time_point> unlocked_at;
    };

    std::vector<Achievement> getPlayerAchievements(const std::string& player_id);
    bool unlockAchievement(const std::string& player_id, const std::string& achievement_code);

    // Initialize database connection
    bool initialize();
    bool isEnabled() const { return enabled; }

private:
    // Helper functions
    nlohmann::json serializeEntity(World& world, EntityID entity);
    EntityID deserializeEntity(World& world, const nlohmann::json& data);

    nlohmann::json serializeComponent(IComponent* comp);
    void deserializeComponent(World& world, EntityID entity, const nlohmann::json& data);

    std::string getEntityType(World& world, EntityID entity);
};

} // namespace ecs