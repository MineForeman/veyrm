/**
 * @file inventory_component.h
 * @brief Inventory component for entities that can carry items
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include <vector>
#include <algorithm>

namespace ecs {

/**
 * @class InventoryComponent
 * @brief Component for entities that can carry items
 */
class InventoryComponent : public Component<InventoryComponent> {
public:
    std::vector<EntityID> items;      ///< IDs of carried items
    size_t max_capacity = 10;         ///< Maximum number of items
    float max_weight = 50.0f;         ///< Maximum carry weight
    float current_weight = 0.0f;      ///< Current total weight
    bool auto_pickup = false;         ///< Auto-pickup items when walked over

    InventoryComponent() = default;
    InventoryComponent(size_t capacity, float weight_limit = 50.0f)
        : max_capacity(capacity), max_weight(weight_limit) {}

    /**
     * @brief Add an item to inventory
     * @param item_id Item entity ID
     * @return true if added successfully
     */
    bool addItem(EntityID item_id) {
        if (items.size() >= max_capacity) {
            return false;
        }
        items.push_back(item_id);
        return true;
    }

    /**
     * @brief Remove an item from inventory
     * @param item_id Item entity ID
     * @return true if removed successfully
     */
    bool removeItem(EntityID item_id) {
        auto it = std::find(items.begin(), items.end(), item_id);
        if (it != items.end()) {
            items.erase(it);
            return true;
        }
        return false;
    }

    /**
     * @brief Check if inventory contains an item
     * @param item_id Item entity ID
     * @return true if item is in inventory
     */
    bool hasItem(EntityID item_id) const {
        return std::find(items.begin(), items.end(), item_id) != items.end();
    }

    /**
     * @brief Check if inventory is full
     * @return true if at capacity
     */
    bool isFull() const {
        return items.size() >= max_capacity;
    }

    /**
     * @brief Get number of items
     * @return Item count
     */
    size_t getItemCount() const {
        return items.size();
    }

    /**
     * @brief Clear all items
     */
    void clear() {
        items.clear();
        current_weight = 0.0f;
    }

    std::string getTypeName() const override { return "InventoryComponent"; }
    ComponentType getType() const override { return ComponentType::INVENTORY; }
};

} // namespace ecs