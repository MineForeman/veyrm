#include "ecs/persistence_system.h"
#include "ecs/game_world.h"
#include "ecs/position_component.h"
#include "ecs/health_component.h"
#include "ecs/renderable_component.h"
#include "ecs/ai_component.h"
#include "ecs/player_component.h"
#include "ecs/stats_component.h"
#include "ecs/experience_component.h"
#include "ecs/item_component.h"
#include "log.h"
#include <random>

namespace ecs {

PersistenceSystem::PersistenceSystem() {
    initialize();
}

bool PersistenceSystem::initialize() {
    db = &db::DatabaseManager::getInstance();
    enabled = db->isInitialized();

    if (enabled) {
        Log::info("PersistenceSystem: Database persistence enabled");
        // Ensure data is loaded automatically
        db->ensureDataLoaded();
    } else {
        Log::warn("PersistenceSystem: Database not initialized, persistence disabled");
    }
    return enabled;
}

void PersistenceSystem::update(float /* deltaTime */, World& /* world */) {
    // Periodic auto-save could be implemented here
    // For now, saves are triggered explicitly
}

bool PersistenceSystem::saveCharacter(World& world, Entity& player_entity,
                                     const std::string& character_id) {
    if (!enabled) return false;

    try {
        // Get player components
        auto* pos = player_entity.getComponent<PositionComponent>();
        auto* stats = player_entity.getComponent<StatsComponent>();
        auto* health = player_entity.getComponent<HealthComponent>();
        // auto* exp = player_entity.getComponent<ExperienceComponent>();

        if (!pos || !stats) {
            Log::error("PersistenceSystem: Player missing required components");
            return false;
        }

        // Serialize game state
        nlohmann::json game_state;
        game_state["position"] = {{"x", pos->position.x}, {"y", pos->position.y}};
        game_state["health"] = {
            {"current", health ? health->hp : 0},
            {"max", health ? health->max_hp : 0}
        };
        game_state["stats"] = {
            {"strength", stats->strength},
            {"dexterity", stats->dexterity},
            {"intelligence", stats->intelligence},
            {"constitution", stats->constitution}
        };

        // Serialize all entities
        game_state["entities"] = nlohmann::json::array();
        for (const auto& entity : world.getEntities()) {
            if (entity.get() != &player_entity) {
                game_state["entities"].push_back(serializeEntity(world, entity->getID()));
            }
        }

        // Save to database
        return db->executeTransaction([&](db::Connection& conn) {
            // Update or insert save game
            auto result = conn.execParams(
                "INSERT INTO save_games (character_id, game_state, updated_at) "
                "VALUES ($1, $2, CURRENT_TIMESTAMP) "
                "ON CONFLICT (character_id) "
                "DO UPDATE SET game_state = $2, updated_at = CURRENT_TIMESTAMP",
                {character_id, game_state.dump()}
            );

            if (!result.isOk()) {
                throw db::QueryException("INSERT save_games", result.getError());
            }

            Log::info("PersistenceSystem: Saved character " + character_id);
            return true;
        });

    } catch (const std::exception& e) {
        Log::error("PersistenceSystem: Failed to save character: " + std::string(e.what()));
        return false;
    }
}

std::optional<Entity*> PersistenceSystem::loadCharacter(World& world,
                                                       const std::string& character_id) {
    if (!enabled) return std::nullopt;

    try {
        auto game_state_str = db->executeQuery([&](db::Connection& conn) {
            auto result = conn.execParams(
                "SELECT game_state FROM save_games WHERE character_id = $1",
                {character_id}
            );

            if (!result.isOk() || result.numRows() == 0) {
                return std::string("");
            }

            return result.getValue(0, 0);
        });

        if (game_state_str.empty()) {
            Log::warn("PersistenceSystem: No save found for character " + character_id);
            return std::nullopt;
        }

        auto game_state = nlohmann::json::parse(game_state_str);

        // Create player entity
        Entity& player = world.createEntity();

        // Restore player components
        if (game_state.contains("position")) {
            player.addComponent<PositionComponent>(
                game_state["position"]["x"].get<int>(),
                game_state["position"]["y"].get<int>()
            );
        }

        if (game_state.contains("health")) {
            player.addComponent<HealthComponent>(
                game_state["health"]["max"].get<int>(),
                game_state["health"]["current"].get<int>()
            );
        }

        if (game_state.contains("stats")) {
            auto& s = game_state["stats"];
            auto& stats = player.addComponent<StatsComponent>();
            stats.strength = s["strength"].get<int>();
            stats.dexterity = s["dexterity"].get<int>();
            stats.intelligence = s["intelligence"].get<int>();
            stats.constitution = s["constitution"].get<int>();
        }

        // Add player tag
        player.addComponent<PlayerComponent>();

        // Restore other entities
        if (game_state.contains("entities")) {
            for (const auto& entity_data : game_state["entities"]) {
                deserializeEntity(world, entity_data);
            }
        }

        Log::info("PersistenceSystem: Loaded character " + character_id);
        return &player;

    } catch (const std::exception& e) {
        Log::error("PersistenceSystem: Failed to load character: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool PersistenceSystem::saveMonsterTemplate(const nlohmann::json& monster_data) {
    if (!enabled) return false;

    try {
        return db->executeTransaction([&](db::Connection& conn) {
            auto result = conn.execParams(
                "INSERT INTO monsters (code, name, glyph, base_hp, base_attack, "
                "base_defense, base_speed, base_xp, threat_level, spawn_depth_min, "
                "spawn_depth_max, version) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, '0.0.3') "
                "ON CONFLICT (code) DO UPDATE SET "
                "name = $2, glyph = $3, base_hp = $4, base_attack = $5, "
                "base_defense = $6, base_speed = $7, base_xp = $8",
                {
                    monster_data["code"].get<std::string>(),
                    monster_data["name"].get<std::string>(),
                    monster_data["glyph"].get<std::string>(),
                    std::to_string(monster_data["hp"].get<int>()),
                    std::to_string(monster_data["attack"].get<int>()),
                    std::to_string(monster_data["defense"].get<int>()),
                    std::to_string(monster_data["speed"].get<int>()),
                    std::to_string(monster_data["xp"].get<int>()),
                    monster_data["threat_level"].get<std::string>(),
                    std::to_string(monster_data.value("spawn_depth_min", 1)),
                    std::to_string(monster_data.value("spawn_depth_max", 100))
                }
            );

            if (!result.isOk()) {
                throw db::QueryException("INSERT monsters", result.getError());
            }

            Log::info("PersistenceSystem: Saved monster template " +
                     monster_data["code"].get<std::string>());
            return true;
        });
    } catch (const std::exception& e) {
        Log::error("PersistenceSystem: Failed to save monster template: " + std::string(e.what()));
        return false;
    }
}

bool PersistenceSystem::submitScore(const LeaderboardEntry& entry) {
    if (!enabled) return false;

    try {
        return db->executeTransaction([&](db::Connection& conn) {
            auto result = conn.execParams(
                "INSERT INTO leaderboards (player_name, score, depth_reached, "
                "play_time, death_reason, submitted_at) "
                "VALUES ($1, $2, $3, $4, $5, CURRENT_TIMESTAMP)",
                {
                    entry.player_name,
                    std::to_string(entry.score),
                    std::to_string(entry.depth_reached),
                    std::to_string(entry.play_time),
                    entry.death_reason
                }
            );

            if (!result.isOk()) {
                throw db::QueryException("INSERT leaderboards", result.getError());
            }

            Log::info("PersistenceSystem: Submitted leaderboard score for " + entry.player_name);
            return true;
        });
    } catch (const std::exception& e) {
        Log::error("PersistenceSystem: Failed to submit score: " + std::string(e.what()));
        return false;
    }
}

std::vector<PersistenceSystem::LeaderboardEntry>
PersistenceSystem::getLeaderboard(int limit, int offset) {
    std::vector<LeaderboardEntry> entries;

    if (!enabled) return entries;

    try {
        db->executeQuery([&](db::Connection& conn) {
            auto result = conn.execParams(
                "SELECT player_name, score, depth_reached, play_time, "
                "death_reason, submitted_at "
                "FROM leaderboards "
                "ORDER BY score DESC "
                "LIMIT $1 OFFSET $2",
                {std::to_string(limit), std::to_string(offset)}
            );

            if (result.isOk()) {
                for (int i = 0; i < result.numRows(); ++i) {
                    LeaderboardEntry entry;
                    entry.player_name = result.getValue(i, 0);
                    entry.score = std::stoi(result.getValue(i, 1));
                    entry.depth_reached = std::stoi(result.getValue(i, 2));
                    entry.play_time = std::stoi(result.getValue(i, 3));
                    entry.death_reason = result.getValue(i, 4);
                    // Parse timestamp - simplified for now
                    entries.push_back(entry);
                }
            }

            return true;
        });

    } catch (const std::exception& e) {
        Log::error("PersistenceSystem: Failed to get leaderboard: " + std::string(e.what()));
    }

    return entries;
}

void PersistenceSystem::logEvent(const std::string& event_type,
                                const nlohmann::json& event_data) {
    if (!enabled) return;

    try {
        db->executeTransaction([&](db::Connection& conn) {
            auto result = conn.execParams(
                "INSERT INTO telemetry (event_type, event_data, game_version, created_at) "
                "VALUES ($1, $2, '0.0.3', CURRENT_TIMESTAMP)",
                {event_type, event_data.dump()}
            );
            return result.isOk();
        });
    } catch (const std::exception& e) {
        // Don't log errors for telemetry to avoid spam
    }
}

nlohmann::json PersistenceSystem::serializeEntity(World& world, EntityID entity_id) {
    nlohmann::json data;
    data["id"] = entity_id;

    Entity* entity = world.getEntity(entity_id);
    if (!entity) return data;

    // Serialize all components
    if (auto* pos = entity->getComponent<PositionComponent>()) {
        data["position"] = {{"x", pos->position.x}, {"y", pos->position.y}};
    }

    if (auto* health = entity->getComponent<HealthComponent>()) {
        data["health"] = {{"current", health->hp}, {"max", health->max_hp}};
    }

    if (auto* render = entity->getComponent<RenderableComponent>()) {
        data["renderable"] = {
            {"glyph", render->glyph}
        };
    }

    if (auto* ai = entity->getComponent<AIComponent>()) {
        data["ai"] = {
            {"behavior", static_cast<int>(ai->behavior)},
            {"vision_range", ai->vision_range},
            {"target_id", ai->target_id}
        };
    }

    // Add entity type
    data["type"] = getEntityType(world, entity_id);

    return data;
}

EntityID PersistenceSystem::deserializeEntity(World& world, const nlohmann::json& data) {
    Entity& entity = world.createEntity();
    EntityID entity_id = entity.getID();

    // Deserialize components
    if (data.contains("position")) {
        entity.addComponent<PositionComponent>(
            data["position"]["x"].get<int>(),
            data["position"]["y"].get<int>()
        );
    }

    if (data.contains("health")) {
        entity.addComponent<HealthComponent>(
            data["health"]["max"].get<int>(),
            data["health"]["current"].get<int>()
        );
    }

    if (data.contains("renderable")) {
        auto& render = entity.addComponent<RenderableComponent>();
        render.glyph = data["renderable"]["glyph"].get<std::string>();
    }

    if (data.contains("ai")) {
        auto& ai = entity.addComponent<AIComponent>();
        ai.behavior = static_cast<AIBehavior>(data["ai"]["behavior"].get<int>());
        ai.vision_range = data["ai"]["vision_range"].get<int>();
        ai.target_id = data["ai"]["target_id"].get<EntityID>();
    }

    return entity_id;
}

std::string PersistenceSystem::getEntityType(World& world, EntityID entity_id) {
    Entity* entity = world.getEntity(entity_id);
    if (!entity) return "unknown";

    if (entity->getComponent<PlayerComponent>()) {
        return "player";
    } else if (entity->getComponent<AIComponent>()) {
        return "monster";
    } else if (entity->getComponent<ItemComponent>()) {
        return "item";
    }
    return "unknown";
}

} // namespace ecs