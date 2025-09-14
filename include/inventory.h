/**
 * @file inventory.h
 * @brief Player inventory management system
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include "item.h"
#include "serializable.h"

using json = nlohmann::json;

/**
 * @class Inventory
 * @brief Manages player's item collection with slots and stacking
 *
 * The Inventory class handles the player's personal item storage,
 * providing slot-based organization with automatic stacking for
 * compatible items. It supports typical roguelike inventory
 * operations including pickup, drop, use, and management.
 *
 * Features:
 * - Slot-based storage (26 slots a-z by default)
 * - Automatic item stacking for compatible items
 * - Type-based item searching and filtering
 * - Weight system (optional for future expansion)
 * - JSON serialization for save/load
 * - Capacity management and overflow handling
 *
 * @see Item
 * @see InventoryRenderer
 * @see Config::getInventoryCapacity()
 */
class Inventory : public ISerializable {
public:
    /** @brief Default inventory capacity (26 slots for a-z) */
    static constexpr int DEFAULT_CAPACITY = 26;

    /**
     * @brief Construct inventory with specified capacity
     * @param capacity Maximum number of item slots (default: 26)
     */
    explicit Inventory(int capacity = DEFAULT_CAPACITY);

    // Item management methods

    /**
     * @brief Add item to inventory
     * @param item Item to add (ownership transferred)
     * @return true if item was successfully added
     * @note Attempts stacking with compatible items first
     */
    bool addItem(std::unique_ptr<Item> item);

    /**
     * @brief Remove item from specific slot
     * @param slot Slot index to remove from
     * @return Unique pointer to removed item (nullptr if slot empty)
     */
    std::unique_ptr<Item> removeItem(int slot);

    /**
     * @brief Remove specific item instance
     * @param item Pointer to item to remove
     * @return Unique pointer to removed item (nullptr if not found)
     */
    std::unique_ptr<Item> removeItem(Item* item);

    // Query methods

    /**
     * @brief Get item in specific slot
     * @param slot Slot index to query
     * @return Pointer to item (nullptr if slot empty)
     */
    Item* getItem(int slot) const;

    /**
     * @brief Find item by ID
     * @param item_id Item identifier to search for
     * @return Pointer to first matching item (nullptr if not found)
     */
    Item* findItem(const std::string& item_id) const;

    /**
     * @brief Find all items of specific type
     * @param type Item type to search for
     * @return Vector of pointers to matching items
     */
    std::vector<Item*> findItems(Item::ItemType type) const;

    /**
     * @brief Find slot containing specific item
     * @param item Item to locate
     * @return Slot index (-1 if not found)
     */
    int findSlot(Item* item) const;

    // Stack management

    /**
     * @brief Check if item can stack with existing items
     * @param item Item to check for stacking compatibility
     * @return true if item can be stacked
     */
    bool canStackWith(Item* item) const;

    /**
     * @brief Find existing stackable item by ID
     * @param item_id Item ID to find stackable version of
     * @return Pointer to stackable item (nullptr if none found)
     */
    Item* findStackableItem(const std::string& item_id) const;

    // Capacity management

    /** @brief Check if inventory is at capacity @return true if no empty slots */
    bool isFull() const;

    /**
     * @brief Check if inventory has space for items
     * @param count Number of items to check space for (default: 1)
     * @return true if enough empty slots available
     */
    bool hasSpace(int count = 1) const;

    /** @brief Get number of occupied slots @return Used slot count */
    int getUsedSlots() const { return items.size(); }

    /** @brief Get total slot capacity @return Maximum slots */
    int getTotalSlots() const { return capacity; }

    // Weight system (for future expansion)

    /** @brief Get total weight of all items @return Total weight */
    int getTotalWeight() const;

    /** @brief Get maximum weight capacity @return Weight limit */
    int getMaxWeight() const { return max_weight; }

    /**
     * @brief Check if item can be carried without exceeding weight limit
     * @param item Item to check
     * @return true if weight allows carrying item
     */
    bool canCarry(const Item* item) const;

    // Utility methods

    /** @brief Remove all items from inventory */
    void clear();

    /** @brief Sort items by type and name for organized display */
    void sort();

    /**
     * @brief Get all items as vector of pointers
     * @return Vector containing pointers to all items
     */
    std::vector<Item*> getAllItems() const;

    // Serialization interface

    /**
     * @brief Serialize inventory to JSON
     * @return JSON object containing all inventory data
     */
    json serialize() const override;

    /**
     * @brief Deserialize inventory from JSON
     * @param data JSON object to deserialize from
     * @return true on successful deserialization
     */
    bool deserialize(const json& data) override;

private:
    int capacity;                             ///< Maximum number of item slots
    int max_weight;                          ///< Maximum weight capacity (future)
    std::vector<std::unique_ptr<Item>> items; ///< Item storage vector

    // Helper methods

    /**
     * @brief Find first empty slot
     * @return Slot index (-1 if no empty slots)
     */
    int findEmptySlot() const;

    /**
     * @brief Attempt to stack item with existing compatible item
     * @param item Item to stack
     * @return true if item was successfully stacked
     */
    bool tryStack(Item* item);
};