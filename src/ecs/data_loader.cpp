/**
 * @file data_loader.cpp
 * @brief Implementation of JSON data loader
 */

#include "ecs/data_loader.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace ecs {

DataLoader& DataLoader::getInstance() {
    static DataLoader instance;
    return instance;
}

bool DataLoader::loadAllData(const std::string& data_dir) {
    std::filesystem::path dir(data_dir);

    bool monsters_loaded = loadMonsters((dir / "monsters.json").string());
    bool items_loaded = loadItems((dir / "items.json").string());

    data_loaded = monsters_loaded && items_loaded;
    return data_loaded;
}

bool DataLoader::loadMonsters(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open monsters file: " << filepath << std::endl;
            return false;
        }

        std::string json_content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
        file.close();

        boost::json::value data = boost::json::parse(json_content);
        monster_templates.clear();

        if (data.is_object() && data.as_object().contains("monsters") &&
            data.as_object().at("monsters").is_array()) {
            for (const auto& monster_json : data.as_object().at("monsters").as_array()) {
                MonsterTemplate monster = parseMonster(monster_json);
                monster_templates[monster.id] = monster;
            }
        }

        std::cout << "Loaded " << monster_templates.size() << " monster templates" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading monsters: " << e.what() << std::endl;
        return false;
    }
}

bool DataLoader::loadItems(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open items file: " << filepath << std::endl;
            return false;
        }

        std::string json_content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
        file.close();

        boost::json::value data = boost::json::parse(json_content);
        item_templates.clear();

        if (data.is_object() && data.as_object().contains("items") &&
            data.as_object().at("items").is_array()) {
            for (const auto& item_json : data.as_object().at("items").as_array()) {
                ItemTemplate item = parseItem(item_json);
                item_templates[item.id] = item;
            }
        }

        std::cout << "Loaded " << item_templates.size() << " item templates" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading items: " << e.what() << std::endl;
        return false;
    }
}

const MonsterTemplate* DataLoader::getMonsterTemplate(const std::string& id) const {
    auto it = monster_templates.find(id);
    return (it != monster_templates.end()) ? &it->second : nullptr;
}

const ItemTemplate* DataLoader::getItemTemplate(const std::string& id) const {
    auto it = item_templates.find(id);
    return (it != item_templates.end()) ? &it->second : nullptr;
}

void DataLoader::clearData() {
    monster_templates.clear();
    item_templates.clear();
    data_loaded = false;
}

ftxui::Color DataLoader::parseColor(const std::string& color_str) const {
    // Map color names to FTXUI colors
    static const std::unordered_map<std::string, ftxui::Color> color_map = {
        {"red", ftxui::Color::Red},
        {"green", ftxui::Color::Green},
        {"blue", ftxui::Color::Blue},
        {"yellow", ftxui::Color::Yellow},
        {"magenta", ftxui::Color::Magenta},
        {"cyan", ftxui::Color::Cyan},
        {"white", ftxui::Color::White},
        {"grey", ftxui::Color::GrayDark},
        {"gray", ftxui::Color::GrayDark},
        {"light_grey", ftxui::Color::GrayLight},
        {"light_gray", ftxui::Color::GrayLight},
        {"dark_grey", ftxui::Color::GrayDark},
        {"dark_gray", ftxui::Color::GrayDark},
        {"brown", ftxui::Color::RGB(139, 69, 19)},
        {"orange", ftxui::Color::RGB(255, 165, 0)},
        {"dark_green", ftxui::Color::RGB(0, 100, 0)},
        {"bright_red", ftxui::Color::RGB(255, 0, 0)}
    };

    auto it = color_map.find(color_str);
    return (it != color_map.end()) ? it->second : ftxui::Color::White;
}

MonsterTemplate DataLoader::parseMonster(const boost::json::value& json) const {
    MonsterTemplate monster;

    // Basic properties
    if (json.is_object()) {
        const auto& obj = json.as_object();
        if (obj.contains("id")) monster.id = boost::json::value_to<std::string>(obj.at("id"));
        if (obj.contains("name")) monster.name = boost::json::value_to<std::string>(obj.at("name"));
        if (obj.contains("description")) monster.description = boost::json::value_to<std::string>(obj.at("description"));
        else monster.description = "";

        // Visual
        if (obj.contains("glyph")) {
            std::string glyph_str = boost::json::value_to<std::string>(obj.at("glyph"));
            monster.glyph = glyph_str.empty() ? '?' : glyph_str[0];
        }
        if (obj.contains("color")) monster.color = parseColor(boost::json::value_to<std::string>(obj.at("color")));

        // Components
        if (obj.contains("components")) {
            const auto& components = obj.at("components");

            // Health component
            if (components.is_object() && components.as_object().contains("health")) {
                const auto& health = components.as_object().at("health");
                if (health.is_object() && health.as_object().contains("max_hp")) {
                    monster.hp = boost::json::value_to<int>(health.as_object().at("max_hp"));
                }
            }

            // Combat component
            if (components.is_object() && components.as_object().contains("combat")) {
                const auto& combat = components.as_object().at("combat");
                if (combat.is_object()) {
                    const auto& combat_obj = combat.as_object();
                    monster.attack = combat_obj.contains("attack_bonus") ? boost::json::value_to<int>(combat_obj.at("attack_bonus")) : 0;
                    monster.defense = combat_obj.contains("defense_bonus") ? boost::json::value_to<int>(combat_obj.at("defense_bonus")) : 0;
                }
            }

            // Stats component (for speed calculation)
            if (components.is_object() && components.as_object().contains("stats")) {
                const auto& stats = components.as_object().at("stats");
                if (stats.is_object()) {
                    int dexterity = stats.as_object().contains("dexterity") ? boost::json::value_to<int>(stats.as_object().at("dexterity")) : 10;
                    monster.speed = 100 + (dexterity - 10) * 5; // Speed based on dexterity
                }
            }

            // AI component
            if (components.is_object() && components.as_object().contains("ai")) {
                const auto& ai = components.as_object().at("ai");
                if (ai.is_object()) {
                    std::string behavior = ai.as_object().contains("behavior") ? boost::json::value_to<std::string>(ai.as_object().at("behavior")) : "aggressive";
                    monster.aggressive = (behavior == "aggressive");
                }
            }
        }

        // Spawn data
        if (obj.contains("spawn")) {
            const auto& spawn = obj.at("spawn");
            if (spawn.is_object()) {
                const auto& spawn_obj = spawn.as_object();
                monster.spawn_weight = spawn_obj.contains("weight") ? boost::json::value_to<float>(spawn_obj.at("weight")) : 1.0f;
                monster.min_depth = spawn_obj.contains("min_depth") ? boost::json::value_to<int>(spawn_obj.at("min_depth")) : 1;
                monster.max_depth = spawn_obj.contains("max_depth") ? boost::json::value_to<int>(spawn_obj.at("max_depth")) : 100;

                if (spawn_obj.contains("pack_size")) {
                    const auto& pack = spawn_obj.at("pack_size");
                    if (pack.is_array() && pack.as_array().size() >= 2) {
                        monster.min_pack_size = boost::json::value_to<int>(pack.as_array()[0]);
                        monster.max_pack_size = boost::json::value_to<int>(pack.as_array()[1]);
                    }
                }
            }
        }

        // XP value
        monster.xp_value = obj.contains("xp_value") ? boost::json::value_to<int>(obj.at("xp_value")) : 10;

        // Tags for special abilities
        if (obj.contains("tags")) {
            const auto& tags = obj.at("tags");
            if (tags.is_array()) {
                for (const auto& tag : tags.as_array()) {
                    std::string tag_str = boost::json::value_to<std::string>(tag);
                    if (tag_str == "intelligent") {
                        monster.can_open_doors = true;
                    }
                    else if (tag_str == "magical") {
                        monster.can_see_invisible = true;
                    }
                }
            }
        }
    }

    return monster;
}

ItemTemplate DataLoader::parseItem(const boost::json::value& json) const {
    ItemTemplate item;

    // Basic properties
    if (json.is_object()) {
        const auto& obj = json.as_object();
        if (obj.contains("id")) item.id = boost::json::value_to<std::string>(obj.at("id"));
        if (obj.contains("name")) item.name = boost::json::value_to<std::string>(obj.at("name"));
        if (obj.contains("description")) item.description = boost::json::value_to<std::string>(obj.at("description"));
        else item.description = "";
        if (obj.contains("type")) item.type = boost::json::value_to<std::string>(obj.at("type"));
        else item.type = "misc";

        // Visual
        if (obj.contains("glyph")) {
            std::string glyph_str = boost::json::value_to<std::string>(obj.at("glyph"));
            item.symbol = glyph_str.empty() ? '*' : glyph_str[0];
        }
        if (obj.contains("color")) item.color = parseColor(boost::json::value_to<std::string>(obj.at("color")));

        // Components
        if (obj.contains("components")) {
            const auto& components = obj.at("components");

            // Item component
            if (components.is_object() && components.as_object().contains("item")) {
                const auto& item_comp = components.as_object().at("item");
                if (item_comp.is_object()) {
                    const auto& item_obj = item_comp.as_object();
                    item.value = item_obj.contains("value") ? boost::json::value_to<int>(item_obj.at("value")) : 0;
                    item.weight = item_obj.contains("weight") ? boost::json::value_to<int>(item_obj.at("weight")) : 1;
                    item.stackable = item_obj.contains("stackable") ? boost::json::value_to<bool>(item_obj.at("stackable")) : false;
                    item.max_stack = item_obj.contains("max_stack") ? boost::json::value_to<int>(item_obj.at("max_stack")) : 1;

                    // Properties based on item type
                    item.heal_amount = item_obj.contains("heal_amount") ? boost::json::value_to<int>(item_obj.at("heal_amount")) : 0;
                    item.damage_amount = item_obj.contains("damage_amount") ? boost::json::value_to<int>(item_obj.at("damage_amount")) : 0;
                    item.attack_bonus = item_obj.contains("attack_bonus") ? boost::json::value_to<int>(item_obj.at("attack_bonus")) : 0;
                    item.defense_bonus = item_obj.contains("defense_bonus") ? boost::json::value_to<int>(item_obj.at("defense_bonus")) : 0;
                    item.min_damage = item_obj.contains("min_damage") ? boost::json::value_to<int>(item_obj.at("min_damage")) : 0;
                    item.max_damage = item_obj.contains("max_damage") ? boost::json::value_to<int>(item_obj.at("max_damage")) : 0;
                }
            }
        }

        // Spawn data
        if (obj.contains("spawn")) {
            const auto& spawn = obj.at("spawn");
            if (spawn.is_object()) {
                const auto& spawn_obj = spawn.as_object();
                item.min_depth = spawn_obj.contains("min_depth") ? boost::json::value_to<int>(spawn_obj.at("min_depth")) : 1;
                item.max_depth = spawn_obj.contains("max_depth") ? boost::json::value_to<int>(spawn_obj.at("max_depth")) : 100;
            }
        }
    }

    return item;
}

} // namespace ecs