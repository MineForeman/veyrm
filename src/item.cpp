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
    if (type_str == "gold") return GOLD;
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