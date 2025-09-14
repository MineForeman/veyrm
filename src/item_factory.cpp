#include "item_factory.h"
#include "log.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <random>

ItemFactory* ItemFactory::instance = nullptr;

ItemFactory& ItemFactory::getInstance() {
    if (!instance) {
        instance = new ItemFactory();
    }
    return *instance;
}

void ItemFactory::cleanup() {
    delete instance;
    instance = nullptr;
}

void ItemFactory::loadFromJson(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open items file: " + filename);
            return;
        }

        nlohmann::json json_data;
        file >> json_data;

        templates.clear();

        if (json_data.contains("items")) {
            for (const auto& item_json : json_data["items"]) {
                parseItemFromJson(item_json);
            }
        }

        LOG_INFO("Loaded " + std::to_string(templates.size()) + " item templates");
    }
    catch (const std::exception& e) {
        LOG_ERROR("Error loading items: " + std::string(e.what()));
    }
}

void ItemFactory::parseItemFromJson(const nlohmann::json& item_json) {
    ItemTemplate tmpl;

    // Required fields
    tmpl.id = item_json["id"];
    tmpl.name = item_json["name"];
    tmpl.description = item_json["description"];
    tmpl.type = Item::stringToType(item_json["type"]);
    tmpl.symbol = item_json["symbol"].get<std::string>()[0];
    tmpl.color = item_json["color"];
    tmpl.value = item_json["value"];
    tmpl.weight = item_json["weight"];

    // Optional fields
    tmpl.stackable = item_json.value("stackable", false);
    tmpl.max_stack = item_json.value("max_stack", 1);

    // Depth range
    if (item_json.contains("depth_range") && item_json["depth_range"].is_array()) {
        tmpl.min_depth = item_json["depth_range"][0];
        tmpl.max_depth = item_json["depth_range"][1];
    } else {
        tmpl.min_depth = 1;
        tmpl.max_depth = 100;
    }

    // Properties
    if (item_json.contains("properties")) {
        for (const auto& [key, value] : item_json["properties"].items()) {
            tmpl.properties[key] = value;
        }
    }

    templates[tmpl.id] = tmpl;
    LOG_DEBUG("Loaded item template: " + tmpl.id);
}

std::unique_ptr<Item> ItemFactory::create(const std::string& item_id) {
    auto it = templates.find(item_id);
    if (it == templates.end()) {
        LOG_ERROR("Item template not found: " + item_id);
        return nullptr;
    }

    const ItemTemplate& tmpl = it->second;
    auto item = std::make_unique<Item>(item_id);

    // Copy template data to item
    item->name = tmpl.name;
    item->description = tmpl.description;
    item->symbol = tmpl.symbol;
    item->color = tmpl.color;
    item->type = tmpl.type;
    item->value = tmpl.value;
    item->weight = tmpl.weight;
    item->stackable = tmpl.stackable;
    item->max_stack = tmpl.max_stack;
    item->properties = tmpl.properties;

    // For gold, set initial stack size from amount property
    if (item->type == Item::GOLD && item->properties.count("amount") > 0) {
        item->stack_size = item->properties["amount"];
    }

    LOG_DEBUG("Created item: " + item->name);
    return item;
}

bool ItemFactory::hasTemplate(const std::string& item_id) const {
    return templates.find(item_id) != templates.end();
}

std::vector<std::string> ItemFactory::getItemsForDepth(int depth) const {
    std::vector<std::string> valid_items;

    for (const auto& [id, tmpl] : templates) {
        if (depth >= tmpl.min_depth && depth <= tmpl.max_depth) {
            valid_items.push_back(id);
        }
    }

    return valid_items;
}

std::string ItemFactory::getRandomItemForDepth(int depth) const {
    auto valid_items = getItemsForDepth(depth);

    if (valid_items.empty()) {
        LOG_WARN("No items available for depth " + std::to_string(depth));
        return "";
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, valid_items.size() - 1);

    return valid_items[dis(gen)];
}