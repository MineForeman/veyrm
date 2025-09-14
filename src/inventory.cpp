#include "inventory.h"
#include "item.h"
#include "log.h"
#include <algorithm>

Inventory::Inventory(int capacity) 
    : capacity(capacity), max_weight(1000) {  // Default max weight
    items.reserve(capacity);
}

bool Inventory::addItem(std::unique_ptr<Item> item) {
    if (!item) return false;

    // Try to stack first if item is stackable
    if (item->stackable && tryStack(item.get())) {
        return true;  // Item was fully stacked
    }

    // Check if we have space
    if (items.size() >= static_cast<size_t>(capacity)) {
        LOG_DEBUG("Inventory full, cannot add item: " + item->name);
        return false;
    }

    // Add the item
    items.push_back(std::move(item));
    LOG_DEBUG("Added item to inventory: " + items.back()->name);
    return true;
}

std::unique_ptr<Item> Inventory::removeItem(int slot) {
    if (slot < 0 || slot >= static_cast<int>(items.size())) {
        return nullptr;
    }

    auto item = std::move(items[slot]);
    items.erase(items.begin() + slot);
    LOG_DEBUG("Removed item from slot " + std::to_string(slot) + ": " + item->name);
    return item;
}

std::unique_ptr<Item> Inventory::removeItem(Item* item) {
    if (!item) return nullptr;

    auto it = std::find_if(items.begin(), items.end(),
        [item](const std::unique_ptr<Item>& ptr) {
            return ptr.get() == item;
        });

    if (it != items.end()) {
        auto removed = std::move(*it);
        items.erase(it);
        LOG_DEBUG("Removed item from inventory: " + removed->name);
        return removed;
    }

    return nullptr;
}

Item* Inventory::getItem(int slot) const {
    if (slot < 0 || slot >= static_cast<int>(items.size())) {
        return nullptr;
    }
    return items[slot].get();
}

Item* Inventory::findItem(const std::string& item_id) const {
    for (const auto& item : items) {
        if (item && item->id == item_id) {
            return item.get();
        }
    }
    return nullptr;
}

std::vector<Item*> Inventory::findItems(Item::ItemType type) const {
    std::vector<Item*> result;
    for (const auto& item : items) {
        if (item && item->type == type) {
            result.push_back(item.get());
        }
    }
    return result;
}

int Inventory::findSlot(Item* item) const {
    if (!item) return -1;

    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].get() == item) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool Inventory::canStackWith(Item* item) const {
    if (!item || !item->stackable) return false;
    return findStackableItem(item->id) != nullptr;
}

Item* Inventory::findStackableItem(const std::string& item_id) const {
    for (const auto& existing : items) {
        if (existing && existing->id == item_id && 
            existing->stackable && 
            existing->stack_size < existing->max_stack) {
            return existing.get();
        }
    }
    return nullptr;
}

bool Inventory::isFull() const {
    return items.size() >= static_cast<size_t>(capacity);
}

bool Inventory::hasSpace(int count) const {
    return static_cast<int>(items.size()) + count <= capacity;
}

int Inventory::getTotalWeight() const {
    int total = 0;
    for (const auto& item : items) {
        if (item) {
            total += item->weight * (item->stackable ? item->stack_size : 1);
        }
    }
    return total;
}

bool Inventory::canCarry(const Item* item) const {
    if (!item) return false;
    int item_weight = item->weight * (item->stackable ? item->stack_size : 1);
    return getTotalWeight() + item_weight <= max_weight;
}

void Inventory::clear() {
    items.clear();
    LOG_DEBUG("Inventory cleared");
}

void Inventory::sort() {
    std::sort(items.begin(), items.end(),
        [](const std::unique_ptr<Item>& a, const std::unique_ptr<Item>& b) {
            if (!a) return false;
            if (!b) return true;
            // Sort by type first, then by name
            if (a->type != b->type) {
                return static_cast<int>(a->type) < static_cast<int>(b->type);
            }
            return a->name < b->name;
        });
}

std::vector<Item*> Inventory::getAllItems() const {
    std::vector<Item*> result;
    for (const auto& item : items) {
        if (item) {
            result.push_back(item.get());
        }
    }
    return result;
}

int Inventory::findEmptySlot() const {
    if (items.size() < static_cast<size_t>(capacity)) {
        return items.size();
    }
    return -1;
}

bool Inventory::tryStack(Item* item) {
    if (!item || !item->stackable) return false;

    for (auto& existing : items) {
        if (existing && existing->id == item->id &&
            existing->canStackWith(*item)) {

            int space = existing->max_stack - existing->stack_size;
            int to_add = std::min(space, item->stack_size);

            existing->stack_size += to_add;
            item->stack_size -= to_add;

            LOG_DEBUG("Stacked " + std::to_string(to_add) + " " + item->name + 
                      " (remaining: " + std::to_string(item->stack_size) + ")");

            return item->stack_size == 0;  // All stacked?
        }
    }

    return false;
}

json Inventory::serialize() const {
    json data;
    data["capacity"] = capacity;
    data["max_weight"] = max_weight;

    json items_array = json::array();
    for (const auto& item : items) {
        if (item) {
            items_array.push_back(item->serialize());
        }
    }
    data["items"] = items_array;

    return data;
}

bool Inventory::deserialize(const json& data) {
    try {
        // Clear existing items
        clear();

        // Restore capacity
        if (data.contains("capacity")) {
            capacity = data["capacity"];
        }

        if (data.contains("max_weight")) {
            max_weight = data["max_weight"];
        }

        // Restore items
        if (data.contains("items") && data["items"].is_array()) {
            for (const auto& item_data : data["items"]) {
                auto item = std::make_unique<Item>();
                if (item->deserialize(item_data)) {
                    items.push_back(std::move(item));
                }
            }
        }

        return true;
    } catch (const std::exception&) {
        return false;
    }
}