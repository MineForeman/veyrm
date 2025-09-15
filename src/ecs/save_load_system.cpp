/**
 * @file save_load_system.cpp
 * @brief Implementation of save/load system
 */

#include <fstream>
#include <sstream>
#include <filesystem>
#include "ecs/save_load_system.h"
#include "ecs/position_component.h"
#include "ecs/renderable_component.h"
#include "ecs/health_component.h"
#include "ecs/combat_component.h"
#include "ecs/inventory_component.h"
#include "ecs/stats_component.h"
#include "ecs/experience_component.h"
#include "ecs/ai_component.h"
#include "ecs/item_component.h"
#include "ecs/equipment_component.h"
#include "ecs/loot_component.h"
#include "ecs/effects_component.h"

namespace ecs {

SaveLoadSystem::SaveLoadSystem(const std::string& save_directory, ILogger* logger)
    : save_dir(save_directory)
    , logger(logger) {
}

bool SaveLoadSystem::saveEntity(const Entity& entity, nlohmann::json& json) const {
    try {
        // Save entity ID
        json["id"] = entity.getID();

        // Save components
        nlohmann::json components;

        // Position
        if (auto* pos = entity.getComponent<PositionComponent>()) {
            components["position"] = {
                {"x", pos->position.x},
                {"y", pos->position.y}
            };
        }

        // Renderable
        if (auto* render = entity.getComponent<RenderableComponent>()) {
            components["renderable"] = {
                {"glyph", render->glyph},
                {"name", render->name}
                // Color serialization would need custom conversion from ftxui::Color
            };
        }

        // Health
        if (auto* health = entity.getComponent<HealthComponent>()) {
            components["health"] = {
                {"current", health->hp},
                {"max", health->max_hp}
            };
        }

        // Combat
        if (auto* combat = entity.getComponent<CombatComponent>()) {
            components["combat"] = {
                {"min_damage", combat->min_damage},
                {"max_damage", combat->max_damage},
                {"attack_bonus", combat->attack_bonus},
                {"damage_modifier", combat->damage_modifier},
                {"defense_bonus", combat->defense_bonus}
            };
        }

        // Stats
        if (auto* stats = entity.getComponent<StatsComponent>()) {
            components["stats"] = {
                {"strength", stats->strength},
                {"dexterity", stats->dexterity},
                {"intelligence", stats->intelligence},
                {"constitution", stats->constitution},
                {"wisdom", stats->wisdom},
                {"charisma", stats->charisma}
            };
        }

        // Experience
        if (auto* exp = entity.getComponent<ExperienceComponent>()) {
            components["experience"] = {
                {"level", exp->level},
                {"experience", exp->experience},
                {"total_experience", exp->total_experience},
                {"skill_points", exp->skill_points},
                {"stat_points", exp->stat_points}
            };
        }

        // Inventory
        if (auto* inv = entity.getComponent<InventoryComponent>()) {
            nlohmann::json items = nlohmann::json::array();
            // Would need to serialize inventory items
            components["inventory"] = {
                {"max_capacity", inv->max_capacity},
                {"items", items}
            };
        }

        // AI
        if (auto* ai = entity.getComponent<AIComponent>()) {
            components["ai"] = {
                {"behavior", static_cast<int>(ai->behavior)},
                {"aggro_range", ai->aggro_range},
                {"vision_range", ai->vision_range}
            };
        }

        // Item
        if (auto* item = entity.getComponent<ItemComponent>()) {
            components["item"] = {
                {"name", item->name},
                {"item_type", static_cast<int>(item->item_type)},
                {"value", item->value},
                {"weight", item->weight},
                {"stack_size", item->stack_size},
                {"equippable", item->equippable},
                {"consumable", item->consumable}
            };
        }

        // Tags
        nlohmann::json tags = nlohmann::json::array();
        for (const auto& tag : entity.getTags()) {
            tags.push_back(tag);
        }

        json["components"] = components;
        json["tags"] = tags;

        return true;
    } catch (const std::exception& e) {
        if (logger) {
            std::stringstream msg;
            msg << "Failed to save entity: " << e.what();
            logger->logError(msg.str());
        }
        return false;
    }
}

std::unique_ptr<Entity> SaveLoadSystem::loadEntity(const nlohmann::json& json) const {
    try {
        auto entity = std::make_unique<Entity>();

        // Load components
        if (json.contains("components")) {
            const auto& components = json["components"];

            // Position
            if (components.contains("position")) {
                const auto& pos = components["position"];
                entity->addComponent<PositionComponent>(pos["x"], pos["y"]);
            }

            // Renderable
            if (components.contains("renderable")) {
                const auto& render = components["renderable"];
                auto& comp = entity->addComponent<RenderableComponent>();
                comp.glyph = render["glyph"].get<std::string>();
                comp.name = render["name"];
                if (render.contains("color")) {
                    const auto& color = render["color"];
                    comp.color = {color[0], color[1], color[2]};
                }
            }

            // Health
            if (components.contains("health")) {
                const auto& health = components["health"];
                entity->addComponent<HealthComponent>(health["max"], health["current"]);
            }

            // Combat
            if (components.contains("combat")) {
                const auto& combat = components["combat"];
                auto& comp = entity->addComponent<CombatComponent>();
                comp.min_damage = combat["min_damage"];
                comp.max_damage = combat["max_damage"];
                comp.attack_bonus = combat["attack_bonus"];
                comp.damage_modifier = combat["damage_modifier"];
                comp.defense_bonus = combat["defense_bonus"];
            }

            // Stats
            if (components.contains("stats")) {
                const auto& stats = components["stats"];
                auto& comp = entity->addComponent<StatsComponent>();
                comp.strength = stats["strength"];
                comp.dexterity = stats["dexterity"];
                comp.intelligence = stats["intelligence"];
                comp.constitution = stats["constitution"];
                comp.wisdom = stats["wisdom"];
                comp.charisma = stats["charisma"];
                comp.recalculateDerived();
            }

            // Experience
            if (components.contains("experience")) {
                const auto& exp = components["experience"];
                auto& comp = entity->addComponent<ExperienceComponent>();
                comp.level = exp["level"];
                comp.experience = exp["experience"];
                comp.total_experience = exp["total_experience"];
                comp.skill_points = exp["skill_points"];
                comp.stat_points = exp["stat_points"];
            }

            // AI
            if (components.contains("ai")) {
                const auto& ai = components["ai"];
                auto& comp = entity->addComponent<AIComponent>();
                comp.behavior = static_cast<AIBehavior>(ai["behavior"].get<int>());
                comp.aggro_range = ai["aggro_range"];
                comp.vision_range = ai["vision_range"];
            }

            // Item
            if (components.contains("item")) {
                const auto& item = components["item"];
                auto& comp = entity->addComponent<ItemComponent>();
                comp.name = item["name"];
                comp.item_type = static_cast<ItemType>(item["item_type"].get<int>());
                comp.value = item["value"];
                comp.weight = item["weight"];
                comp.stack_size = item["stack_size"];
                comp.equippable = item["equippable"];
                comp.consumable = item["consumable"];
            }
        }

        // Load tags
        if (json.contains("tags")) {
            for (const auto& tag : json["tags"]) {
                entity->addTag(tag);
            }
        }

        return entity;
    } catch (const std::exception& e) {
        if (logger) {
            std::stringstream msg;
            msg << "Failed to load entity: " << e.what();
            logger->logError(msg.str());
        }
        return nullptr;
    }
}

bool SaveLoadSystem::saveWorld(const std::vector<std::unique_ptr<Entity>>& entities,
                               const std::string& filename) const {
    try {
        nlohmann::json world;
        world["version"] = SAVE_VERSION;
        world["timestamp"] = std::time(nullptr);

        nlohmann::json entities_json = nlohmann::json::array();
        for (const auto& entity : entities) {
            if (entity) {
                nlohmann::json entity_json;
                if (saveEntity(*entity, entity_json)) {
                    entities_json.push_back(entity_json);
                }
            }
        }

        world["entities"] = entities_json;

        // Write to file
        std::string filepath = save_dir + "/" + filename;
        std::ofstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open save file: " + filepath);
        }

        file << world.dump(2);  // Pretty print with 2 spaces
        file.close();

        if (logger) {
            std::stringstream msg;
            msg << "World saved to " << filepath << " (" << entities.size() << " entities)";
            logger->logSystem(msg.str());
        }

        return true;
    } catch (const std::exception& e) {
        if (logger) {
            std::stringstream msg;
            msg << "Failed to save world: " << e.what();
            logger->logError(msg.str());
        }
        return false;
    }
}

std::vector<std::unique_ptr<Entity>> SaveLoadSystem::loadWorld(const std::string& filename) const {
    std::vector<std::unique_ptr<Entity>> entities;

    try {
        std::string filepath = save_dir + "/" + filename;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open save file: " + filepath);
        }

        nlohmann::json world;
        file >> world;
        file.close();

        // Check version
        if (world.contains("version") && world["version"] != SAVE_VERSION) {
            if (logger) {
                logger->logWarning("Save file version mismatch");
            }
        }

        // Load entities
        if (world.contains("entities")) {
            for (const auto& entity_json : world["entities"]) {
                auto entity = loadEntity(entity_json);
                if (entity) {
                    entities.push_back(std::move(entity));
                }
            }
        }

        if (logger) {
            std::stringstream msg;
            msg << "World loaded from " << filepath << " (" << entities.size() << " entities)";
            logger->logSystem(msg.str());
        }

    } catch (const std::exception& e) {
        if (logger) {
            std::stringstream msg;
            msg << "Failed to load world: " << e.what();
            logger->logError(msg.str());
        }
        entities.clear();
    }

    return entities;
}

bool SaveLoadSystem::quickSave(const std::vector<std::unique_ptr<Entity>>& entities) const {
    return saveWorld(entities, "quicksave.json");
}

std::vector<std::unique_ptr<Entity>> SaveLoadSystem::quickLoad() const {
    return loadWorld("quicksave.json");
}

std::vector<std::string> SaveLoadSystem::getSaveFiles() const {
    std::vector<std::string> saves;

    try {
        namespace fs = std::filesystem;
        if (fs::exists(save_dir) && fs::is_directory(save_dir)) {
            for (const auto& entry : fs::directory_iterator(save_dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    saves.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception& e) {
        if (logger) {
            logger->logError("Failed to list saves: " + std::string(e.what()));
        }
        // Fallback to known saves
        saves.push_back("quicksave.json");
        saves.push_back("autosave.json");
    }

    return saves;
}

bool SaveLoadSystem::deleteSave(const std::string& filename) const {
    try {
        namespace fs = std::filesystem;
        std::string filepath = save_dir + "/" + filename;

        if (fs::exists(filepath)) {
            fs::remove(filepath);
            if (logger) {
                logger->logSystem("Save deleted: " + filename);
            }
            return true;
        } else {
            if (logger) {
                logger->logError("Save file not found: " + filename);
            }
            return false;
        }
    } catch (const std::exception& e) {
        if (logger) {
            logger->logError("Failed to delete save: " + std::string(e.what()));
        }
        return false;
    }
}

} // namespace ecs