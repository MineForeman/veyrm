#include "item_manager.h"
#include "item_factory.h"
#include "map.h"
#include "log.h"
#include <algorithm>
#include <random>

ItemManager::ItemManager(Map* map) : map_ref(map) {
    LOG_DEBUG("ItemManager initialized");
}

void ItemManager::spawnItem(const std::string& item_id, int x, int y) {
    // Check if position is valid
    if (!map_ref || !map_ref->inBounds(x, y)) {
        LOG_ERROR("Invalid spawn position for item: " + std::to_string(x) + "," + std::to_string(y));
        return;
    }

    // Check if position is walkable
    if (!map_ref->isWalkable(x, y)) {
        LOG_WARN("Attempting to spawn item on non-walkable tile");
        return;
    }

    // Create the item
    auto item = ItemFactory::getInstance().create(item_id);
    if (!item) {
        LOG_ERROR("Failed to create item: " + item_id);
        return;
    }

    // Set position and add to manager
    item->setPosition(x, y);
    LOG_INFO("Spawned item '" + item->name + "' at (" + std::to_string(x) + "," + std::to_string(y) + ")");
    items.push_back(std::move(item));
}

void ItemManager::spawnItem(std::unique_ptr<Item> item, int x, int y) {
    if (!item) return;

    // Check if position is valid
    if (!map_ref || !map_ref->inBounds(x, y)) {
        LOG_ERROR("Invalid spawn position for item: " + std::to_string(x) + "," + std::to_string(y));
        return;
    }

    // Check if position is walkable
    if (!map_ref->isWalkable(x, y)) {
        LOG_WARN("Attempting to spawn item on non-walkable tile");
        return;
    }

    // Set position and add to manager
    std::string item_name = item->name;
    item->setPosition(x, y);
    LOG_INFO("Dropped item '" + item_name + "' at (" + std::to_string(x) + "," + std::to_string(y) + ")");
    items.push_back(std::move(item));
}

void ItemManager::spawnRandomItem(int x, int y, int depth) {
    std::string item_id = ItemFactory::getInstance().getRandomItemForDepth(depth);
    if (!item_id.empty()) {
        spawnItem(item_id, x, y);
    }
}

void ItemManager::removeItem(Item* item) {
    if (!item) return;

    auto it = std::find_if(items.begin(), items.end(),
        [item](const std::unique_ptr<Item>& ptr) {
            return ptr.get() == item;
        });

    if (it != items.end()) {
        LOG_DEBUG("Removed item: " + (*it)->name);
        items.erase(it);
    }
}

void ItemManager::removeItemAt(int x, int y) {
    Item* item = getItemAt(x, y);
    if (item) {
        removeItem(item);
    }
}

Item* ItemManager::getItemAt(int x, int y) {
    for (const auto& item : items) {
        if (item->x == x && item->y == y) {
            return item.get();
        }
    }
    return nullptr;
}

std::vector<Item*> ItemManager::getItemsAt(int x, int y) {
    std::vector<Item*> result;
    for (const auto& item : items) {
        if (item->x == x && item->y == y) {
            result.push_back(item.get());
        }
    }
    return result;
}

std::vector<Item*> ItemManager::getAllItems() {
    std::vector<Item*> result;
    for (const auto& item : items) {
        result.push_back(item.get());
    }
    return result;
}

const Item* ItemManager::getItemAt(int x, int y) const {
    for (const auto& item : items) {
        if (item->x == x && item->y == y) {
            return item.get();
        }
    }
    return nullptr;
}

std::vector<const Item*> ItemManager::getItemsAt(int x, int y) const {
    std::vector<const Item*> result;
    for (const auto& item : items) {
        if (item->x == x && item->y == y) {
            result.push_back(item.get());
        }
    }
    return result;
}

std::vector<const Item*> ItemManager::getAllItems() const {
    std::vector<const Item*> result;
    for (const auto& item : items) {
        result.push_back(item.get());
    }
    return result;
}

void ItemManager::clear() {
    items.clear();
    LOG_DEBUG("ItemManager cleared");
}

void ItemManager::spawnGold(int x, int y, int amount) {
    // Spawn gold coins with specific amount
    auto item = ItemFactory::getInstance().create("gold");
    if (item) {
        item->setPosition(x, y);
        item->stack_size = amount;
        item->properties["amount"] = amount;
        LOG_INFO("Spawned " + std::to_string(amount) + " gold at (" +
                 std::to_string(x) + "," + std::to_string(y) + ")");
        items.push_back(std::move(item));
    }
}

void ItemManager::spawnRandomItems(int count, int depth) {
    if (!map_ref) return;

    static std::random_device rd;
    static std::mt19937 gen(rd());

    int spawned = 0;
    int attempts = 0;
    const int max_attempts = count * 10;

    while (spawned < count && attempts < max_attempts) {
        attempts++;

        // Find random walkable position
        std::uniform_int_distribution<> x_dis(1, map_ref->getWidth() - 2);
        std::uniform_int_distribution<> y_dis(1, map_ref->getHeight() - 2);

        int x = x_dis(gen);
        int y = y_dis(gen);

        if (map_ref->isWalkable(x, y) && !getItemAt(x, y)) {
            spawnRandomItem(x, y, depth);
            spawned++;
        }
    }

    LOG_INFO("Spawned " + std::to_string(spawned) + " random items");
}