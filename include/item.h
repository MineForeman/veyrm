/**
 * @file item.h
 * @brief Item base class for all game items
 * @author Veyrm Team
 * @date 2025
 */

#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include "serializable.h"

using json = nlohmann::json;

/**
 * @class Item
 * @brief Base class for all items in the game
 *
 * The Item class represents all collectible objects in the game world,
 * including potions, weapons, armor, scrolls, food, and miscellaneous
 * items. It handles basic item properties, stacking mechanics, positioning,
 * and serialization for save/load functionality.
 *
 * Key features:
 * - Position tracking for world placement
 * - Stacking system for similar items
 * - Generic property system for flexible effects
 * - Type-safe item categorization
 * - JSON serialization support
 *
 * @see ItemFactory
 * @see ItemManager
 * @see Inventory
 */
class Item : public ISerializable {
public:
    /**
     * @enum ItemType
     * @brief Categories of items available in the game
     */
    enum ItemType {
        POTION,  ///< Healing and effect potions
        SCROLL,  ///< Magic scrolls with various effects
        WEAPON,  ///< Weapons for combat
        ARMOR,   ///< Protective equipment
        FOOD,    ///< Consumable food items
        GOLD,    ///< Currency and treasure
        MISC     ///< Miscellaneous items
    };

    // Position in world
    int x, y;              ///< World coordinates

    // Core item properties
    std::string id;        ///< Unique item identifier
    std::string name;      ///< Display name
    std::string description; ///< Detailed description
    char symbol;           ///< Display character
    std::string color;     ///< Display color name
    ItemType type;         ///< Item category

    // Economic and physical properties
    int value;             ///< Gold value for trading
    int weight;            ///< Weight for encumbrance (future)
    bool stackable;        ///< Can stack with identical items
    int stack_size;        ///< Current stack quantity
    int max_stack;         ///< Maximum items per stack

    // Generic property system for flexible effects
    std::map<std::string, int> properties; ///< Key-value effect properties

    // Constructors

    /** @brief Default constructor for empty item */
    Item();

    /**
     * @brief Construct item from template ID
     * @param item_id Item template identifier from items.json
     */
    Item(const std::string& item_id);

    // Position and basic properties

    /**
     * @brief Set item position in world
     * @param new_x X coordinate
     * @param new_y Y coordinate
     */
    void setPosition(int new_x, int new_y);

    /** @brief Check if item can be stacked @return true if stackable */
    bool isStackable() const { return stackable; }

    /**
     * @brief Check if this item can stack with another
     * @param other Item to check compatibility with
     * @return true if items can be stacked together
     */
    bool canStackWith(const Item& other) const;

    // Stack management

    /**
     * @brief Add items to stack
     * @param amount Number of items to add (default: 1)
     * @return true if items were successfully added
     */
    bool addToStack(int amount = 1);

    /**
     * @brief Remove items from stack
     * @param amount Number of items to remove (default: 1)
     * @return true if items were successfully removed
     */
    bool removeFromStack(int amount = 1);

    /** @brief Get current stack size @return Number of items in stack */
    int getStackSize() const { return stack_size; }

    // Type conversion utilities

    /**
     * @brief Convert string to ItemType enum
     * @param type_str String representation of item type
     * @return Corresponding ItemType value
     */
    static ItemType stringToType(const std::string& type_str);

    /**
     * @brief Convert ItemType enum to string
     * @param type ItemType to convert
     * @return String representation
     */
    static std::string typeToString(ItemType type);

    // Serialization interface

    /**
     * @brief Serialize item to JSON
     * @return JSON object containing all item data
     */
    json serialize() const override;

    /**
     * @brief Deserialize item from JSON
     * @param data JSON object to deserialize from
     * @return true on successful deserialization
     */
    bool deserialize(const json& data) override;
};

#endif // ITEM_H