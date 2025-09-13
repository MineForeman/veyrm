#ifndef ITEM_MANAGER_H
#define ITEM_MANAGER_H

#include <vector>
#include <memory>
#include "item.h"

class Map;
class FOV;

class ItemManager {
private:
    std::vector<std::unique_ptr<Item>> items;
    Map* map_ref;

public:
    explicit ItemManager(Map* map);

    // Item management
    void spawnItem(const std::string& item_id, int x, int y);
    void spawnItem(std::unique_ptr<Item> item, int x, int y);
    void spawnRandomItem(int x, int y, int depth);
    void removeItem(Item* item);
    void removeItemAt(int x, int y);

    // Item retrieval
    Item* getItemAt(int x, int y);
    const Item* getItemAt(int x, int y) const;
    std::vector<Item*> getItemsAt(int x, int y);
    std::vector<const Item*> getItemsAt(int x, int y) const;
    std::vector<Item*> getAllItems();
    std::vector<const Item*> getAllItems() const;

    // Utility
    void clear();
    size_t getItemCount() const { return items.size(); }

    // Spawning helpers
    void spawnGold(int x, int y, int amount);
    void spawnRandomItems(int count, int depth);
};

#endif // ITEM_MANAGER_H