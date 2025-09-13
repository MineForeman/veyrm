#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include "serializable.h"

using json = nlohmann::json;

class Item : public ISerializable {
public:
    enum ItemType {
        POTION,
        SCROLL,
        WEAPON,
        ARMOR,
        FOOD,
        GOLD,
        MISC
    };

    // Position
    int x, y;

    // Core properties
    std::string id;
    std::string name;
    std::string description;
    char symbol;
    std::string color;
    ItemType type;

    // Item properties
    int value;
    int weight;
    bool stackable;
    int stack_size;
    int max_stack;

    // Generic properties for effects
    std::map<std::string, int> properties;

    // Constructors
    Item();
    Item(const std::string& item_id);

    // Methods
    void setPosition(int new_x, int new_y);
    bool isStackable() const { return stackable; }
    bool canStackWith(const Item& other) const;

    // Stack management
    bool addToStack(int amount = 1);
    bool removeFromStack(int amount = 1);
    int getStackSize() const { return stack_size; }

    // Type conversion
    static ItemType stringToType(const std::string& type_str);
    static std::string typeToString(ItemType type);

    // Serialization
    json serialize() const override;
    bool deserialize(const json& data) override;
};

#endif // ITEM_H