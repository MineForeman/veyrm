/**
 * @file game_entity_repository.cpp
 * @brief Implementation of ECS entity repository for PostgreSQL
 * @author Veyrm Team
 * @date 2025
 */

#include "db/game_entity_repository.h"
#include "db/database_manager.h"
#include "ecs/entity.h"
#include "ecs/entity_factory.h"
#include "ecs/system_manager.h"
#include "ecs/game_world.h"
#include "ecs/position_component.h"
#include "ecs/health_component.h"
#include "ecs/renderable_component.h"
#include "ecs/stats_component.h"
#include "ecs/ai_component.h"
#include "ecs/experience_component.h"
#include "ecs/combat_component.h"
#include "ecs/player_component.h"
#include "ecs/input_system.h"
#include "log.h"
#include <sstream>
#include <libpq-fe.h>

namespace db {

GameEntityRepository::GameEntityRepository()
    : db_manager(DatabaseManager::getInstance()) {
    // Constructor - repository uses singleton DatabaseManager
}

bool GameEntityRepository::saveGameState(const GameSaveData& save_data, const std::vector<GameEntityData>& entities) {
    try {
        auto result = db_manager.executeTransaction([this, &save_data, &entities](Connection& conn) -> bool {
            // Save metadata first
            if (!saveMetaWithConn(conn, save_data)) {
                LOG_ERROR("Failed to save game metadata");
                return false;
            }

            // Clear existing entities for this save
            if (!deleteEntitiesWithConn(conn, save_data.user_id, save_data.save_slot)) {
                LOG_ERROR("Failed to clear existing entities");
                return false;
            }

            // Save all entities
            for (const auto& entity : entities) {
                if (!saveEntityWithConn(conn, entity)) {
                    LOG_ERROR("Failed to save entity " + std::to_string(entity.id));
                    return false;
                }
            }

            LOG_INFO("Successfully saved game state: " + std::to_string(entities.size()) + " entities");
            return true;
        });
        return result;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in saveGameState: " + std::string(e.what()));
        return false;
    }
}

bool GameEntityRepository::saveMetaWithConn(Connection& conn, const GameSaveData& save_data) {
    try {
        std::string sql = R"(
            INSERT INTO game_saves (
                user_id, save_slot, character_name, character_level,
                map_level, play_time_seconds, game_version, save_version,
                device_id, device_name, map_width, map_height, world_seed
            ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
            ON CONFLICT (user_id, save_slot) DO UPDATE SET
                character_name = EXCLUDED.character_name,
                character_level = EXCLUDED.character_level,
                map_level = EXCLUDED.map_level,
                play_time_seconds = EXCLUDED.play_time_seconds,
                game_version = EXCLUDED.game_version,
                save_version = EXCLUDED.save_version,
                device_id = EXCLUDED.device_id,
                device_name = EXCLUDED.device_name,
                map_width = EXCLUDED.map_width,
                map_height = EXCLUDED.map_height,
                world_seed = EXCLUDED.world_seed
        )";

        std::vector<std::string> params = {
            std::to_string(save_data.user_id),
            std::to_string(save_data.save_slot),
            save_data.character_name,
            std::to_string(save_data.character_level),
            std::to_string(save_data.map_level),
            std::to_string(save_data.play_time_seconds),
            save_data.game_version,
            save_data.save_version,
            save_data.device_id,
            save_data.device_name,
            std::to_string(save_data.map_width),
            std::to_string(save_data.map_height),
            std::to_string(save_data.world_seed)
        };

        auto result = conn.execParams(sql, params);
        if (!result.isOk()) {
            LOG_ERROR("Failed to save game metadata: " + result.getError());
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in saveMetaWithConn: " + std::string(e.what()));
        return false;
    }
}

bool GameEntityRepository::deleteEntitiesWithConn(Connection& conn, int user_id, int save_slot) {
    try {
        std::string sql = "DELETE FROM game_entities WHERE user_id = $1 AND save_slot = $2";

        std::vector<std::string> params = {
            std::to_string(user_id),
            std::to_string(save_slot)
        };

        auto result = conn.execParams(sql, params);
        if (!result.isOk()) {
            LOG_ERROR("Failed to delete entities: " + result.getError());
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in deleteEntitiesWithConn: " + std::string(e.what()));
        return false;
    }
}

bool GameEntityRepository::saveEntityWithConn(Connection& conn, const GameEntityData& entity_data) {
    try {
        // Convert component data to JSON string
        std::string json_str = boost::json::serialize(entity_data.component_data);

        // Convert tags to PostgreSQL array format
        std::string tags_str = "{";
        for (size_t i = 0; i < entity_data.entity_tags.size(); ++i) {
            if (i > 0) tags_str += ",";
            tags_str += "\"" + entity_data.entity_tags[i] + "\"";
        }
        tags_str += "}";

        std::string sql = R"(
            INSERT INTO game_entities (
                id, user_id, save_slot, entity_type, x, y, map_level,
                is_active, definition_id, definition_type, component_data, entity_tags
            ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12)
            ON CONFLICT (id) DO UPDATE SET
                user_id = EXCLUDED.user_id,
                save_slot = EXCLUDED.save_slot,
                entity_type = EXCLUDED.entity_type,
                x = EXCLUDED.x,
                y = EXCLUDED.y,
                map_level = EXCLUDED.map_level,
                is_active = EXCLUDED.is_active,
                definition_id = EXCLUDED.definition_id,
                definition_type = EXCLUDED.definition_type,
                component_data = EXCLUDED.component_data,
                entity_tags = EXCLUDED.entity_tags
        )";

        std::vector<std::string> params = {
            std::to_string(entity_data.id),
            std::to_string(entity_data.user_id),
            std::to_string(entity_data.save_slot),
            entity_data.entity_type,
            std::to_string(entity_data.x),
            std::to_string(entity_data.y),
            std::to_string(entity_data.map_level),
            entity_data.is_active ? "true" : "false",
            entity_data.definition_id,
            entity_data.definition_type,
            json_str,
            tags_str
        };

        auto result = conn.execParams(sql, params);
        if (!result.isOk()) {
            LOG_ERROR("Failed to save entity: " + result.getError());
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in saveEntityWithConn: " + std::string(e.what()));
        return false;
    }
}

std::optional<std::pair<GameSaveData, std::vector<GameEntityData>>>
GameEntityRepository::loadGameState(int user_id, int save_slot) {
    try {
        auto result = db_manager.executeTransaction([this, user_id, save_slot](Connection& conn) -> std::optional<std::pair<GameSaveData, std::vector<GameEntityData>>> {
            // Load metadata
            auto save_data = loadMetaWithConn(conn, user_id, save_slot);
            if (!save_data.has_value()) {
                LOG_INFO("No save metadata found for user " + std::to_string(user_id) + " slot " + std::to_string(save_slot));
                return std::nullopt;
            }

            // Load entities
            auto entities = loadEntitiesWithConn(conn, user_id, save_slot);

            LOG_INFO("Loaded game state: " + std::to_string(entities.size()) + " entities");
            return std::make_pair(save_data.value(), entities);
        });

        if (result.has_value()) {
            return result.value();
        }
        return std::nullopt;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in loadGameState: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<GameSaveData> GameEntityRepository::loadMetaWithConn(Connection& conn, int user_id, int save_slot) {
    try {
        std::string sql = R"(
            SELECT user_id, save_slot, character_name, character_level,
                   map_level, play_time_seconds, game_version, save_version,
                   device_id, device_name, map_width, map_height, world_seed
            FROM game_saves
            WHERE user_id = $1 AND save_slot = $2
        )";

        std::vector<std::string> params = {
            std::to_string(user_id),
            std::to_string(save_slot)
        };

        auto result = conn.execParams(sql, params);
        if (!result.isOk()) {
            LOG_ERROR("Failed to load game metadata: " + result.getError());
            return std::nullopt;
        }

        if (result.numRows() == 0) {
            return std::nullopt;
        }

        GameSaveData save_data;
        save_data.user_id = std::stoi(result.getValue(0, 0));
        save_data.save_slot = std::stoi(result.getValue(0, 1));
        save_data.character_name = result.getValue(0, 2);
        save_data.character_level = std::stoi(result.getValue(0, 3));
        save_data.map_level = std::stoi(result.getValue(0, 4));
        save_data.play_time_seconds = std::stoi(result.getValue(0, 5));
        save_data.game_version = result.getValue(0, 6);
        save_data.save_version = result.getValue(0, 7);
        save_data.device_id = result.getValue(0, 8);
        save_data.device_name = result.getValue(0, 9);
        save_data.map_width = std::stoi(result.getValue(0, 10));
        save_data.map_height = std::stoi(result.getValue(0, 11));
        save_data.world_seed = std::stoll(result.getValue(0, 12));

        return save_data;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in loadMetaWithConn: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<GameEntityData> GameEntityRepository::loadEntitiesWithConn(Connection& conn, int user_id, int save_slot) {
    std::vector<GameEntityData> entities;

    try {
        std::string sql = R"(
            SELECT id, user_id, save_slot, entity_type, x, y, map_level,
                   is_active, definition_id, definition_type, component_data, entity_tags
            FROM game_entities
            WHERE user_id = $1 AND save_slot = $2 AND is_active = true
            ORDER BY entity_type, id
        )";

        std::vector<std::string> params = {
            std::to_string(user_id),
            std::to_string(save_slot)
        };

        auto result = conn.execParams(sql, params);
        if (!result.isOk()) {
            LOG_ERROR("Failed to load entities: " + result.getError());
            return entities;
        }

        int rows = result.numRows();
        entities.reserve(rows);

        for (int i = 0; i < rows; ++i) {
            GameEntityData entity;
            entity.id = std::stoll(result.getValue(i, 0));
            entity.user_id = std::stoi(result.getValue(i, 1));
            entity.save_slot = std::stoi(result.getValue(i, 2));
            entity.entity_type = result.getValue(i, 3);
            entity.x = std::stoi(result.getValue(i, 4));
            entity.y = std::stoi(result.getValue(i, 5));
            entity.map_level = std::stoi(result.getValue(i, 6));
            entity.is_active = result.getValue(i, 7) == "t";
            entity.definition_id = result.getValue(i, 8);
            entity.definition_type = result.getValue(i, 9);

            // Parse JSON component data
            std::string json_str = result.getValue(i, 10);
            if (!json_str.empty()) {
                try {
                    auto parsed = boost::json::parse(json_str);
                    if (parsed.is_object()) {
                        entity.component_data = parsed.as_object();
                    }
                } catch (const std::exception& e) {
                    LOG_WARN("Failed to parse component JSON for entity " + std::to_string(entity.id) + ": " + e.what());
                }
            }

            // Parse tags array - simplified for now
            std::string tags_str = result.getValue(i, 11);
            // TODO: Parse PostgreSQL array format properly
            // For now, we'll skip tag parsing

            entities.push_back(entity);
        }

        LOG_INFO("Loaded " + std::to_string(entities.size()) + " entities from PostgreSQL");

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in loadEntitiesWithConn: " + std::string(e.what()));
    }

    return entities;
}

// Public interface methods that delegate to transaction-based implementations
bool GameEntityRepository::saveEntity(const GameEntityData& entity_data) {
    try {
        auto result = db_manager.executeTransaction([this, &entity_data](Connection& conn) -> bool {
            return saveEntityWithConn(conn, entity_data);
        });
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in saveEntity: " + std::string(e.what()));
        return false;
    }
}

std::vector<GameEntityData> GameEntityRepository::loadEntities(int user_id, int save_slot) {
    try {
        auto result = db_manager.executeTransaction([this, user_id, save_slot](Connection& conn) -> std::vector<GameEntityData> {
            return loadEntitiesWithConn(conn, user_id, save_slot);
        });
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in loadEntities: " + std::string(e.what()));
        return {};
    }
}

bool GameEntityRepository::deleteEntities(int user_id, int save_slot) {
    try {
        auto result = db_manager.executeTransaction([this, user_id, save_slot](Connection& conn) -> bool {
            return deleteEntitiesWithConn(conn, user_id, save_slot);
        });
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in deleteEntities: " + std::string(e.what()));
        return false;
    }
}

bool GameEntityRepository::saveMeta(const GameSaveData& save_data) {
    try {
        auto result = db_manager.executeTransaction([this, &save_data](Connection& conn) -> bool {
            return saveMetaWithConn(conn, save_data);
        });
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in saveMeta: " + std::string(e.what()));
        return false;
    }
}

std::optional<GameSaveData> GameEntityRepository::loadMeta(int user_id, int save_slot) {
    try {
        auto result = db_manager.executeTransaction([this, user_id, save_slot](Connection& conn) -> std::optional<GameSaveData> {
            return loadMetaWithConn(conn, user_id, save_slot);
        });

        if (result.has_value()) {
            return result.value();
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in loadMeta: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<GameSaveData> GameEntityRepository::listSaves(int /*user_id*/) {
    // TODO: Implement save listing
    return {};
}

bool GameEntityRepository::deleteSave(int user_id, int save_slot) {
    try {
        auto result = db_manager.executeTransaction([this, user_id, save_slot](Connection& conn) -> bool {
            // Delete entities first
            if (!deleteEntitiesWithConn(conn, user_id, save_slot)) {
                return false;
            }

            // Delete save metadata
            std::string sql = "DELETE FROM game_saves WHERE user_id = $1 AND save_slot = $2";
            std::vector<std::string> params = {
                std::to_string(user_id),
                std::to_string(save_slot)
            };

            auto result = conn.execParams(sql, params);
            if (!result.isOk()) {
                LOG_ERROR("Failed to delete save metadata: " + result.getError());
                return false;
            }

            return true;
        });
        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in deleteSave: " + std::string(e.what()));
        return false;
    }
}

GameEntityData GameEntityRepository::entityToData(const ecs::Entity& entity, int user_id, int save_slot) {
    GameEntityData data;
    data.id = entity.getID();
    data.user_id = user_id;
    data.save_slot = save_slot;
    data.map_level = 1; // TODO: Get from context
    data.is_active = true;

    // Determine entity type and definition based on components/tags
    if (entity.hasTag("player")) {
        data.entity_type = "player";
        data.definition_type = "player";
        data.definition_id = "player"; // Single player type
    } else if (entity.hasTag("monster")) {
        data.entity_type = "monster";
        data.definition_type = "monster";
        // Extract monster ID from entity tags (look for monster ID tags, excluding generic "monster" tag)
        data.definition_id = "unknown";
        for (const auto& tag : entity.getTags()) {
            if (tag != "monster" && tag != "hostile" && tag != "enemy") {
                // This should be the monster definition ID
                data.definition_id = tag;
                break;
            }
        }
    } else if (entity.hasTag("item")) {
        data.entity_type = "item";
        data.definition_type = "item";
        // Extract item ID from entity tags (look for specific item IDs, prioritizing compound names)
        data.definition_id = "unknown";
        std::string best_candidate;
        for (const auto& tag : entity.getTags()) {
            if (tag != "item" && tag != "loot" && tag != "equipment" && tag != "food" && tag != "weapon" && tag != "armor") {
                // Prioritize tags with underscores (compound names like "food_ration")
                if (tag.find('_') != std::string::npos) {
                    data.definition_id = tag;
                    break;
                } else if (best_candidate.empty()) {
                    best_candidate = tag;
                }
            }
        }
        if (data.definition_id == "unknown" && !best_candidate.empty()) {
            data.definition_id = best_candidate;
        }
    } else {
        data.entity_type = "unknown";
        data.definition_type = "unknown";
        data.definition_id = "unknown";
    }

    // Get position
    if (auto* pos = entity.getComponent<ecs::PositionComponent>()) {
        data.x = pos->position.x;
        data.y = pos->position.y;
    } else {
        data.x = 0;
        data.y = 0;
    }

    // Convert tags
    for (const auto& tag : entity.getTags()) {
        data.entity_tags.push_back(tag);
    }

    // Serialize components
    data.component_data = serializeComponents(entity);

    return data;
}

boost::json::object GameEntityRepository::serializeComponents(const ecs::Entity& entity) {
    boost::json::object components;

    for (const auto& [type, component] : entity.getComponents()) {
        if (!component) continue;

        boost::json::object comp_obj;
        comp_obj["type"] = component->getTypeName();

        // Serialize component data based on type
        switch (type) {
            case ecs::ComponentType::POSITION: {
                // Position is stored in data.x, data.y - don't duplicate in component_data
                // This prevents position conflicts during deserialization
                continue;
            }
            case ecs::ComponentType::HEALTH: {
                if (auto* health = dynamic_cast<ecs::HealthComponent*>(component.get())) {
                    comp_obj["hp"] = health->hp;
                    comp_obj["max_hp"] = health->max_hp;
                }
                break;
            }
            case ecs::ComponentType::RENDERABLE: {
                if (auto* render = dynamic_cast<ecs::RenderableComponent*>(component.get())) {
                    // Only store dynamic state - static data comes from definitions tables
                    comp_obj["visible"] = render->is_visible;
                    comp_obj["always_visible"] = render->always_visible;
                    comp_obj["render_priority"] = render->render_priority;
                    // Note: glyph, color, name come from monster/item definitions
                }
                break;
            }
            case ecs::ComponentType::COMBAT: {
                if (auto* combat = dynamic_cast<ecs::CombatComponent*>(component.get())) {
                    comp_obj["min_damage"] = combat->min_damage;
                    comp_obj["max_damage"] = combat->max_damage;
                    comp_obj["attack_bonus"] = combat->attack_bonus;
                    comp_obj["defense_bonus"] = combat->defense_bonus;
                }
                break;
            }
            case ecs::ComponentType::STATS: {
                if (auto* stats = dynamic_cast<ecs::StatsComponent*>(component.get())) {
                    comp_obj["strength"] = stats->strength;
                    comp_obj["dexterity"] = stats->dexterity;
                    comp_obj["intelligence"] = stats->intelligence;
                    comp_obj["constitution"] = stats->constitution;
                    comp_obj["wisdom"] = stats->wisdom;
                    comp_obj["charisma"] = stats->charisma;
                    comp_obj["mana"] = stats->mana;
                    comp_obj["stamina"] = stats->stamina;
                }
                break;
            }
            case ecs::ComponentType::AI: {
                if (auto* ai = dynamic_cast<ecs::AIComponent*>(component.get())) {
                    comp_obj["vision_range"] = ai->vision_range;
                    comp_obj["aggro_range"] = ai->aggro_range;
                    comp_obj["target_id"] = static_cast<int64_t>(ai->target_id);
                    comp_obj["has_seen_player"] = ai->has_seen_player;
                    comp_obj["turns_since_player_seen"] = ai->turns_since_player_seen;
                }
                break;
            }
            case ecs::ComponentType::INPUT: {
                // InputComponent doesn't need serialization - it's just a marker component
                // The component will be re-added during entity creation
                comp_obj["marker"] = true;
                break;
            }
            case ecs::ComponentType::CUSTOM: {
                // Handle experience component
                if (auto* exp = dynamic_cast<ecs::ExperienceComponent*>(component.get())) {
                    comp_obj["level"] = exp->level;
                    comp_obj["experience"] = exp->experience;
                    comp_obj["experience_to_next"] = exp->experience_to_next;
                    comp_obj["total_experience"] = exp->total_experience;
                    comp_obj["skill_points"] = exp->skill_points;
                    comp_obj["stat_points"] = exp->stat_points;
                }
                break;
            }
            default:
                // For unknown component types, just save the type name
                comp_obj["data"] = "unserialized";
                break;
        }

        components[std::to_string(static_cast<int>(type))] = comp_obj;
    }

    return components;
}

std::vector<GameEntityData> GameEntityRepository::serializeWorld(const ecs::World& world, int user_id, int save_slot) {
    std::vector<GameEntityData> entities;

    for (const auto& entity_ptr : world.getEntities()) {
        if (!entity_ptr || !entity_ptr->isValid()) continue;
        entities.push_back(entityToData(*entity_ptr, user_id, save_slot));
    }

    return entities;
}

std::unique_ptr<ecs::Entity> GameEntityRepository::dataToEntity(const GameEntityData& data, ecs::World& /* world */) {
    try {
        // Create entity with the saved ID (not auto-generated)
        auto entity = std::make_unique<ecs::Entity>(data.id);

        // Add position component first
        entity->addComponent<ecs::PositionComponent>(data.x, data.y);

        // Create entity based on definition type using factory, then copy components
        std::unique_ptr<ecs::Entity> template_entity;
        if (data.definition_type == "monster") {
            template_entity = ecs::EntityFactory::createMonster(data.definition_id, 0, 0);
        } else if (data.definition_type == "item") {
            template_entity = ecs::EntityFactory::createItem(data.definition_id, 0, 0);
        } else if (data.definition_type == "player") {
            // Use PlayerFactory like normal gameplay does, not EntityFactory
            template_entity = ecs::PlayerFactory().create(0, 0);
        } else {
            LOG_WARN("Unknown entity definition type: " + data.definition_type);
            return nullptr;
        }

        if (!template_entity) {
            LOG_ERROR("Failed to create template entity from definition: " + data.definition_id);
            return nullptr;
        }

        // Copy all components from template to our entity (except position)
        for (const auto& [type, component] : template_entity->getComponents()) {
            if (type != ecs::ComponentType::POSITION) {
                // Clone the component (this is a simplified approach)
                if (auto* health = dynamic_cast<ecs::HealthComponent*>(component.get())) {
                    entity->addComponent<ecs::HealthComponent>(*health);
                } else if (auto* renderable = dynamic_cast<ecs::RenderableComponent*>(component.get())) {
                    entity->addComponent<ecs::RenderableComponent>(*renderable);
                } else if (auto* combat = dynamic_cast<ecs::CombatComponent*>(component.get())) {
                    entity->addComponent<ecs::CombatComponent>(*combat);
                } else if (auto* stats = dynamic_cast<ecs::StatsComponent*>(component.get())) {
                    entity->addComponent<ecs::StatsComponent>(*stats);
                } else if (auto* ai = dynamic_cast<ecs::AIComponent*>(component.get())) {
                    entity->addComponent<ecs::AIComponent>(*ai);
                } else if (auto* player = dynamic_cast<ecs::PlayerComponent*>(component.get())) {
                    entity->addComponent<ecs::PlayerComponent>(*player);
                }
                // Add other component types as needed
            }
        }

        // Restore dynamic component state from saved data
        deserializeComponents(*entity, data.component_data);

        // Restore entity tags
        for (const auto& tag : data.entity_tags) {
            entity->addTag(tag);
        }

        // Ensure essential type tags are present for proper entityToData conversion
        if (data.definition_type == "player" && !entity->hasTag("player")) {
            entity->addTag("player");
        } else if (data.definition_type == "monster" && !entity->hasTag("monster")) {
            entity->addTag("monster");
        } else if (data.definition_type == "item" && !entity->hasTag("item")) {
            entity->addTag("item");
        }

        LOG_DEBUG("Restored entity ID=" + std::to_string(data.id) + " type=" + data.definition_type + " def=" + data.definition_id);
        return entity;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to deserialize entity: " + std::string(e.what()));
        return nullptr;
    }
}

int GameEntityRepository::deserializeWorld(const std::vector<GameEntityData>& entities, ecs::GameWorld& game_world) {
    int restored_count = 0;

    for (const auto& entity_data : entities) {
        try {
            auto& inner_world = game_world.getWorld();
            auto entity = dataToEntity(entity_data, inner_world);
            if (entity) {
                game_world.addEntityWithTracking(std::move(entity));
                restored_count++;
            } else {
                LOG_WARN("Failed to restore entity ID=" + std::to_string(entity_data.id));
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while restoring entity ID=" + std::to_string(entity_data.id) + ": " + std::string(e.what()));
        }
    }

    LOG_INFO("World deserialization completed: " + std::to_string(restored_count) + "/" + std::to_string(entities.size()) + " entities restored");
    return restored_count;
}

void GameEntityRepository::deserializeComponents(ecs::Entity& entity, const boost::json::object& component_data) {
    for (const auto& [type_str, comp_data] : component_data) {
        try {
            int component_type = std::stoi(type_str);
            auto type = static_cast<ecs::ComponentType>(component_type);

            if (!comp_data.is_object()) {
                continue;
            }

            auto comp_obj = comp_data.as_object();

            switch (type) {
                case ecs::ComponentType::HEALTH: {
                    if (auto* health = entity.getComponent<ecs::HealthComponent>()) {
                        if (comp_obj.contains("hp") && comp_obj.at("hp").is_int64()) {
                            health->hp = comp_obj.at("hp").as_int64();
                        }
                        if (comp_obj.contains("max_hp") && comp_obj.at("max_hp").is_int64()) {
                            health->max_hp = comp_obj.at("max_hp").as_int64();
                        }
                    }
                    break;
                }
                case ecs::ComponentType::RENDERABLE: {
                    if (auto* render = entity.getComponent<ecs::RenderableComponent>()) {
                        if (comp_obj.contains("visible") && comp_obj.at("visible").is_bool()) {
                            render->is_visible = comp_obj.at("visible").as_bool();
                        }
                        if (comp_obj.contains("always_visible") && comp_obj.at("always_visible").is_bool()) {
                            render->always_visible = comp_obj.at("always_visible").as_bool();
                        }
                        if (comp_obj.contains("render_priority") && comp_obj.at("render_priority").is_int64()) {
                            render->render_priority = comp_obj.at("render_priority").as_int64();
                        }
                    }
                    break;
                }
                case ecs::ComponentType::STATS: {
                    if (auto* stats = entity.getComponent<ecs::StatsComponent>()) {
                        if (comp_obj.contains("strength") && comp_obj.at("strength").is_int64()) {
                            stats->strength = comp_obj.at("strength").as_int64();
                        }
                        if (comp_obj.contains("dexterity") && comp_obj.at("dexterity").is_int64()) {
                            stats->dexterity = comp_obj.at("dexterity").as_int64();
                        }
                        if (comp_obj.contains("intelligence") && comp_obj.at("intelligence").is_int64()) {
                            stats->intelligence = comp_obj.at("intelligence").as_int64();
                        }
                        if (comp_obj.contains("constitution") && comp_obj.at("constitution").is_int64()) {
                            stats->constitution = comp_obj.at("constitution").as_int64();
                        }
                        if (comp_obj.contains("wisdom") && comp_obj.at("wisdom").is_int64()) {
                            stats->wisdom = comp_obj.at("wisdom").as_int64();
                        }
                        if (comp_obj.contains("charisma") && comp_obj.at("charisma").is_int64()) {
                            stats->charisma = comp_obj.at("charisma").as_int64();
                        }
                        stats->recalculateDerived();
                    }
                    break;
                }
                case ecs::ComponentType::AI: {
                    if (auto* ai = entity.getComponent<ecs::AIComponent>()) {
                        if (comp_obj.contains("vision_range") && comp_obj.at("vision_range").is_int64()) {
                            ai->vision_range = comp_obj.at("vision_range").as_int64();
                        }
                        if (comp_obj.contains("aggro_range") && comp_obj.at("aggro_range").is_int64()) {
                            ai->aggro_range = comp_obj.at("aggro_range").as_int64();
                        }
                        if (comp_obj.contains("target_id") && comp_obj.at("target_id").is_int64()) {
                            ai->target_id = comp_obj.at("target_id").as_int64();
                        }
                        if (comp_obj.contains("has_seen_player") && comp_obj.at("has_seen_player").is_bool()) {
                            ai->has_seen_player = comp_obj.at("has_seen_player").as_bool();
                        }
                    }
                    break;
                }
                case ecs::ComponentType::COMBAT: {
                    if (auto* combat = entity.getComponent<ecs::CombatComponent>()) {
                        if (comp_obj.contains("min_damage") && comp_obj.at("min_damage").is_int64()) {
                            combat->min_damage = comp_obj.at("min_damage").as_int64();
                        }
                        if (comp_obj.contains("max_damage") && comp_obj.at("max_damage").is_int64()) {
                            combat->max_damage = comp_obj.at("max_damage").as_int64();
                        }
                        if (comp_obj.contains("attack_bonus") && comp_obj.at("attack_bonus").is_int64()) {
                            combat->attack_bonus = comp_obj.at("attack_bonus").as_int64();
                        }
                        if (comp_obj.contains("defense_bonus") && comp_obj.at("defense_bonus").is_int64()) {
                            combat->defense_bonus = comp_obj.at("defense_bonus").as_int64();
                        }
                    }
                    break;
                }
                case ecs::ComponentType::INPUT: {
                    // InputComponent is just a marker - no data to restore
                    // The component will be properly initialized during entity creation
                    break;
                }
                case ecs::ComponentType::POSITION: {
                    // Position is set from data.x, data.y during entity creation
                    // Don't override with component_data to prevent conflicts
                    break;
                }
                case ecs::ComponentType::CUSTOM: {
                    // Handle experience component (which uses CUSTOM type)
                    if (auto* exp = entity.getComponent<ecs::ExperienceComponent>()) {
                        if (comp_obj.contains("level") && comp_obj.at("level").is_int64()) {
                            exp->level = comp_obj.at("level").as_int64();
                        }
                        if (comp_obj.contains("experience") && comp_obj.at("experience").is_int64()) {
                            exp->experience = comp_obj.at("experience").as_int64();
                        }
                        if (comp_obj.contains("experience_to_next") && comp_obj.at("experience_to_next").is_int64()) {
                            exp->experience_to_next = comp_obj.at("experience_to_next").as_int64();
                        }
                        if (comp_obj.contains("total_experience") && comp_obj.at("total_experience").is_int64()) {
                            exp->total_experience = comp_obj.at("total_experience").as_int64();
                        }
                        if (comp_obj.contains("skill_points") && comp_obj.at("skill_points").is_int64()) {
                            exp->skill_points = comp_obj.at("skill_points").as_int64();
                        }
                        if (comp_obj.contains("stat_points") && comp_obj.at("stat_points").is_int64()) {
                            exp->stat_points = comp_obj.at("stat_points").as_int64();
                        }
                    }
                    break;
                }
                default:
                    // For unknown component types, we skip restoration
                    break;
            }
        } catch (const std::exception& e) {
            LOG_WARN("Failed to deserialize component " + std::string(type_str) + ": " + std::string(e.what()));
        }
    }
}

} // namespace db