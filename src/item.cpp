#include "item.h"
#include "log.h"

Item::Item()
    : x(0), y(0), id(""), name("Unknown"), description(""),
      symbol('?'), color("white"), type(MISC),
      value(0), weight(0), stackable(false),
      stack_size(1), max_stack(1) {
}

Item::Item(const std::string& item_id)
    : x(0), y(0), id(item_id), name("Unknown"), description(""),
      symbol('?'), color("white"), type(MISC),
      value(0), weight(0), stackable(false),
      stack_size(1), max_stack(1) {
    LOG_DEBUG("Created item: " + item_id);
}

void Item::setPosition(int new_x, int new_y) {
    x = new_x;
    y = new_y;
}

bool Item::canStackWith(const Item& other) const {
    if (!stackable || !other.stackable) {
        return false;
    }

    // Items must have the same ID to stack
    if (id != other.id) {
        return false;
    }

    // Check if there's room in the stack
    if (stack_size >= max_stack) {
        return false;
    }

    return true;
}

bool Item::addToStack(int amount) {
    if (!stackable) {
        return false;
    }

    int new_size = stack_size + amount;
    if (new_size > max_stack) {
        return false;
    }

    stack_size = new_size;
    return true;
}

bool Item::removeFromStack(int amount) {
    if (amount > stack_size) {
        return false;
    }

    stack_size -= amount;
    return true;
}

Item::ItemType Item::stringToType(const std::string& type_str) {
    if (type_str == "potion") return POTION;
    if (type_str == "scroll") return SCROLL;
    if (type_str == "weapon") return WEAPON;
    if (type_str == "armor") return ARMOR;
    if (type_str == "food") return FOOD;
    if (type_str == "gold" || type_str == "currency") return GOLD;
    return MISC;
}

std::string Item::typeToString(ItemType type) {
    switch (type) {
        case POTION: return "potion";
        case SCROLL: return "scroll";
        case WEAPON: return "weapon";
        case ARMOR: return "armor";
        case FOOD: return "food";
        case GOLD: return "gold";
        case MISC: return "misc";
        default: return "misc";
    }
}

json Item::serialize() const {
    json data;

    // Position
    data["x"] = x;
    data["y"] = y;

    // Core properties
    data["id"] = id;
    data["name"] = name;
    data["description"] = description;
    data["symbol"] = std::string(1, symbol);
    data["color"] = color;
    data["type"] = typeToString(type);

    // Item properties
    data["value"] = value;
    data["weight"] = weight;
    data["stackable"] = stackable;
    data["stack_size"] = stack_size;
    data["max_stack"] = max_stack;

    // Generic properties
    data["properties"] = properties;

    return data;
}

bool Item::deserialize(const json& data) {
    try {
        // Position
        if (data.contains("x") && data.contains("y")) {
            x = data["x"];
            y = data["y"];
        }

        // Core properties
        if (data.contains("id")) id = data["id"];
        if (data.contains("name")) name = data["name"];
        if (data.contains("description")) description = data["description"];
        if (data.contains("symbol")) {
            std::string sym = data["symbol"];
            if (!sym.empty()) symbol = sym[0];
        }
        if (data.contains("color")) color = data["color"];
        if (data.contains("type")) type = stringToType(data["type"]);

        // Item properties
        if (data.contains("value")) value = data["value"];
        if (data.contains("weight")) weight = data["weight"];
        if (data.contains("stackable")) stackable = data["stackable"];
        if (data.contains("stack_size")) stack_size = data["stack_size"];
        if (data.contains("max_stack")) max_stack = data["max_stack"];

        // Generic properties
        if (data.contains("properties")) {
            properties = data["properties"].get<std::map<std::string, int>>();
        }

        return true;
    } catch (const std::exception&) {
        return false;
    }
}