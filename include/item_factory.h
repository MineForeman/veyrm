#ifndef ITEM_FACTORY_H
#define ITEM_FACTORY_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <nlohmann/json_fwd.hpp>
#include "item.h"

class ItemFactory {
private:
    struct ItemTemplate {
        std::string id;
        std::string name;
        std::string description;
        char symbol;
        std::string color;
        Item::ItemType type;
        int value;
        int weight;
        bool stackable;
        int max_stack;
        std::map<std::string, int> properties;
        int min_depth;
        int max_depth;
    };

    static ItemFactory* instance;
    std::map<std::string, ItemTemplate> templates;

    ItemFactory() = default;

public:
    static ItemFactory& getInstance();

    void loadFromJson(const std::string& filename);
    std::unique_ptr<Item> create(const std::string& item_id);
    bool hasTemplate(const std::string& item_id) const;
    std::vector<std::string> getItemsForDepth(int depth) const;

    // Get random item ID for depth
    std::string getRandomItemForDepth(int depth) const;

    // Cleanup
    static void cleanup();

private:
    void parseItemFromJson(const nlohmann::json& item_json);
};

#endif // ITEM_FACTORY_H