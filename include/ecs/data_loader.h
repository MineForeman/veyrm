/**
 * @file data_loader.h
 * @brief JSON data loader for monsters and items
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>
#include <ftxui/screen/color.hpp>

namespace ecs {

// Forward declarations
class Entity;

/**
 * @struct MonsterTemplate
 * @brief Template data for creating monsters from JSON
 */
struct MonsterTemplate {
    std::string id;
    std::string name;
    std::string description;
    char glyph;
    ftxui::Color color;
    int hp;
    int attack;
    int defense;
    int speed;
    int xp_value;
    int min_depth;
    int max_depth;
    float spawn_weight;

    // Flags
    bool aggressive = true;
    bool can_open_doors = false;
    bool can_see_invisible = false;

    // Pack info
    int min_pack_size = 1;
    int max_pack_size = 1;
};

/**
 * @struct ItemTemplate
 * @brief Template data for creating items from JSON
 */
struct ItemTemplate {
    std::string id;
    std::string name;
    std::string description;
    std::string type;
    char symbol;
    ftxui::Color color;
    int value;
    int weight;
    bool stackable;
    int max_stack;

    // Properties
    int heal_amount = 0;
    int damage_amount = 0;
    int attack_bonus = 0;
    int defense_bonus = 0;
    int min_damage = 0;
    int max_damage = 0;

    // Depth range
    int min_depth = 1;
    int max_depth = 100;
};

/**
 * @class DataLoader
 * @brief Loads and caches game data from JSON files
 */
class DataLoader {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to DataLoader instance
     */
    static DataLoader& getInstance();

    /**
     * @brief Load all data files
     * @param data_dir Directory containing data files
     * @return true if successful
     */
    bool loadAllData(const std::string& data_dir = "data");

    /**
     * @brief Load monsters from JSON
     * @param filepath Path to monsters.json
     * @return true if successful
     */
    bool loadMonsters(const std::string& filepath);

    /**
     * @brief Load items from JSON
     * @param filepath Path to items.json
     * @return true if successful
     */
    bool loadItems(const std::string& filepath);

    /**
     * @brief Get monster template by ID
     * @param id Monster ID
     * @return Pointer to template or nullptr
     */
    const MonsterTemplate* getMonsterTemplate(const std::string& id) const;

    /**
     * @brief Get item template by ID
     * @param id Item ID
     * @return Pointer to template or nullptr
     */
    const ItemTemplate* getItemTemplate(const std::string& id) const;

    /**
     * @brief Get all monster templates
     * @return Map of monster templates
     */
    const std::unordered_map<std::string, MonsterTemplate>& getMonsterTemplates() const {
        return monster_templates;
    }

    /**
     * @brief Get all item templates
     * @return Map of item templates
     */
    const std::unordered_map<std::string, ItemTemplate>& getItemTemplates() const {
        return item_templates;
    }

    /**
     * @brief Clear all loaded data
     */
    void clearData();

    /**
     * @brief Check if data is loaded
     * @return true if data has been loaded
     */
    bool isLoaded() const { return data_loaded; }

private:
    DataLoader() = default;
    ~DataLoader() = default;
    DataLoader(const DataLoader&) = delete;
    DataLoader& operator=(const DataLoader&) = delete;

    /**
     * @brief Parse color string to FTXUI color
     * @param color_str Color name string
     * @return FTXUI color
     */
    ftxui::Color parseColor(const std::string& color_str) const;

    /**
     * @brief Parse monster from JSON object
     * @param json JSON object
     * @return Monster template
     */
    MonsterTemplate parseMonster(const nlohmann::json& json) const;

    /**
     * @brief Parse item from JSON object
     * @param json JSON object
     * @return Item template
     */
    ItemTemplate parseItem(const nlohmann::json& json) const;

    std::unordered_map<std::string, MonsterTemplate> monster_templates;
    std::unordered_map<std::string, ItemTemplate> item_templates;
    bool data_loaded = false;
};

} // namespace ecs