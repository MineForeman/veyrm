/**
 * @file item_factory.h
 * @brief Factory for creating items from JSON templates
 * @author Veyrm Team
 * @date 2025
 */

#ifndef ITEM_FACTORY_H
#define ITEM_FACTORY_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <nlohmann/json_fwd.hpp>
#include "item.h"

/**
 * @class ItemFactory
 * @brief Singleton factory for creating items from JSON templates
 *
 * The ItemFactory loads item definitions from JSON configuration files
 * and creates Item instances based on those templates. It supports
 * depth-based item generation for procedural dungeon content and
 * manages all item creation throughout the game.
 *
 * Features:
 * - JSON-based item template loading
 * - Depth-based item filtering for progressive difficulty
 * - Random item selection for loot generation
 * - Template validation and error handling
 * - Singleton pattern for global access
 *
 * @see Item
 * @see ItemManager
 * @see SpawnManager
 */
class ItemFactory {
private:
    /**
     * @struct ItemTemplate
     * @brief Template data for item creation
     */
    struct ItemTemplate {
        std::string id;                       ///< Unique item identifier
        std::string name;                     ///< Display name
        std::string description;              ///< Item description
        char symbol;                          ///< Display character
        std::string color;                    ///< Display color
        Item::ItemType type;                  ///< Item category
        int value;                            ///< Gold value
        int weight;                           ///< Weight units
        bool stackable;                       ///< Can stack with others
        int max_stack;                        ///< Maximum stack size
        std::map<std::string, int> properties; ///< Effect properties
        int min_depth;                        ///< Minimum dungeon depth
        int max_depth;                        ///< Maximum dungeon depth
    };

    static ItemFactory* instance;             ///< Singleton instance
    std::map<std::string, ItemTemplate> templates; ///< Item template database

    /** @brief Private constructor for singleton */
    ItemFactory() = default;

public:
    /** @brief Get singleton instance @return Factory instance */
    static ItemFactory& getInstance();

    /** @brief Load item templates from JSON file @param filename Path to items.json */
    void loadFromJson(const std::string& filename);

    /** @brief Create item from template @param item_id Template ID @return New item instance */
    std::unique_ptr<Item> create(const std::string& item_id);

    /** @brief Check if template exists @param item_id Template ID @return true if exists */
    bool hasTemplate(const std::string& item_id) const;

    /** @brief Get items available at depth @param depth Dungeon depth @return Item ID list */
    std::vector<std::string> getItemsForDepth(int depth) const;

    /** @brief Get random item for depth @param depth Dungeon depth @return Random item ID */
    std::string getRandomItemForDepth(int depth) const;

    /** @brief Cleanup singleton instance */
    static void cleanup();

private:
    /** @brief Parse single item from JSON @param item_json JSON item data */
    void parseItemFromJson(const nlohmann::json& item_json);
};

#endif // ITEM_FACTORY_H