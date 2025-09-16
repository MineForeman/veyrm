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

        nlohmann::json data;
        file >> data;

        monster_templates.clear();

        for (const auto& monster_json : data["monsters"]) {
            MonsterTemplate monster = parseMonster(monster_json);
            monster_templates[monster.id] = monster;
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

        nlohmann::json data;
        file >> data;

        item_templates.clear();

        for (const auto& item_json : data["items"]) {
            ItemTemplate item = parseItem(item_json);
            item_templates[item.id] = item;
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

MonsterTemplate DataLoader::parseMonster(const nlohmann::json& json) const {
    MonsterTemplate monster;

    // Basic properties
    monster.id = json["id"];
    monster.name = json["name"];
    monster.description = json.value("description", "");

    // Visual
    std::string glyph_str = json["glyph"];
    monster.glyph = glyph_str.empty() ? '?' : glyph_str[0];
    monster.color = parseColor(json["color"]);

    // Components
    const auto& components = json["components"];

    // Health component
    if (components.contains("health")) {
        const auto& health = components["health"];
        monster.hp = health["max_hp"];
    }

    // Combat component
    if (components.contains("combat")) {
        const auto& combat = components["combat"];
        monster.attack = combat.value("attack_bonus", 0);
        monster.defense = combat.value("defense_bonus", 0);
    }

    // Stats component (for speed calculation)
    if (components.contains("stats")) {
        const auto& stats = components["stats"];
        int dexterity = stats.value("dexterity", 10);
        monster.speed = 100 + (dexterity - 10) * 5; // Speed based on dexterity
    }

    // AI component
    if (components.contains("ai")) {
        const auto& ai = components["ai"];
        std::string behavior = ai.value("behavior", "aggressive");
        monster.aggressive = (behavior == "aggressive");
    }

    // Spawn data
    if (json.contains("spawn")) {
        const auto& spawn = json["spawn"];
        monster.spawn_weight = spawn.value("weight", 1.0f);
        monster.min_depth = spawn.value("min_depth", 1);
        monster.max_depth = spawn.value("max_depth", 100);

        if (spawn.contains("pack_size")) {
            const auto& pack = spawn["pack_size"];
            if (pack.is_array() && pack.size() >= 2) {
                monster.min_pack_size = pack[0];
                monster.max_pack_size = pack[1];
            }
        }
    }

    // XP value
    monster.xp_value = json.value("xp_value", 10);

    // Tags for special abilities
    if (json.contains("tags")) {
        for (const auto& tag : json["tags"]) {
            std::string tag_str = tag;
            if (tag_str == "intelligent") {
                monster.can_open_doors = true;
            }
            else if (tag_str == "magical") {
                monster.can_see_invisible = true;
            }
        }
    }

    return monster;
}

ItemTemplate DataLoader::parseItem(const nlohmann::json& json) const {
    ItemTemplate item;

    // Basic properties
    item.id = json["id"];
    item.name = json["name"];
    item.description = json.value("description", "");
    item.type = json.value("type", "misc");

    // Visual
    std::string glyph_str = json["glyph"];
    item.symbol = glyph_str.empty() ? '*' : glyph_str[0];
    item.color = parseColor(json["color"]);

    // Components
    const auto& components = json["components"];

    // Item component
    if (components.contains("item")) {
        const auto& item_comp = components["item"];
        item.value = item_comp.value("value", 0);
        item.weight = item_comp.value("weight", 1);
        item.stackable = item_comp.value("stackable", false);
        item.max_stack = item_comp.value("max_stack", 1);

        // Properties based on item type
        item.heal_amount = item_comp.value("heal_amount", 0);
        item.damage_amount = item_comp.value("damage_amount", 0);
        item.attack_bonus = item_comp.value("attack_bonus", 0);
        item.defense_bonus = item_comp.value("defense_bonus", 0);
        item.min_damage = item_comp.value("min_damage", 0);
        item.max_damage = item_comp.value("max_damage", 0);
    }

    // Spawn data
    if (json.contains("spawn")) {
        const auto& spawn = json["spawn"];
        item.min_depth = spawn.value("min_depth", 1);
        item.max_depth = spawn.value("max_depth", 100);
    }

    return item;
}

} // namespace ecs