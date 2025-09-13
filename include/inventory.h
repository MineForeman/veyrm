#pragma once

#include <vector>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include "item.h"
#include "serializable.h"

using json = nlohmann::json;

class Inventory : public ISerializable {
public:
    // Configuration
    static constexpr int DEFAULT_CAPACITY = 26;  // a-z slots

    // Constructor
    explicit Inventory(int capacity = DEFAULT_CAPACITY);

    // Item Management
    bool addItem(std::unique_ptr<Item> item);
    std::unique_ptr<Item> removeItem(int slot);
    std::unique_ptr<Item> removeItem(Item* item);

    // Queries
    Item* getItem(int slot) const;
    Item* findItem(const std::string& item_id) const;
    std::vector<Item*> findItems(Item::ItemType type) const;
    int findSlot(Item* item) const;

    // Stack Management
    bool canStackWith(Item* item) const;
    Item* findStackableItem(const std::string& item_id) const;

    // Capacity
    bool isFull() const;
    bool hasSpace(int count = 1) const;
    int getUsedSlots() const { return items.size(); }
    int getTotalSlots() const { return capacity; }

    // Weight (optional for MVP)
    int getTotalWeight() const;
    int getMaxWeight() const { return max_weight; }
    bool canCarry(const Item* item) const;

    // Utility
    void clear();
    void sort();  // Sort items by type/name
    std::vector<Item*> getAllItems() const;

    // Serialization
    json serialize() const override;
    bool deserialize(const json& data) override;

private:
    int capacity;
    int max_weight;  // Optional weight limit
    std::vector<std::unique_ptr<Item>> items;

    // Helper methods
    int findEmptySlot() const;
    bool tryStack(Item* item);
};