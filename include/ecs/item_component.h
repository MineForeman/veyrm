/**
 * @file item_component.h
 * @brief Component for item entities
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include <string>

namespace ecs {

/**
 * @enum ItemType
 * @brief Categories of items
 */
enum class ItemType {
    WEAPON,
    ARMOR,
    CONSUMABLE,
    QUEST,
    MISC,
    POTION,
    SCROLL,
    FOOD,
    AMMUNITION,
    SHIELD,
    ACCESSORY,
    HELMET,
    GLOVES,
    BOOTS,
    AMULET,
    RING,
    CLOAK
};

/**
 * @class ItemComponent
 * @brief Component for entities that are items
 */
class ItemComponent : public Component<ItemComponent> {
public:
    std::string name = "Unknown Item";    ///< Item name
    std::string description = "";         ///< Item description
    ItemType item_type = ItemType::MISC;  ///< Item category
    float weight = 1.0f;                  ///< Item weight
    int value = 0;                        ///< Item value in gold
    int stack_size = 1;                   ///< Current stack size
    int max_stack = 1;                    ///< Maximum stack size
    bool consumable = false;              ///< Can be consumed
    bool equippable = false;              ///< Can be equipped

    // Effects when used
    int heal_amount = 0;                  ///< HP restored when consumed
    int mana_amount = 0;                  ///< Mana restored when consumed
    int damage_amount = 0;                ///< Direct damage when used

    // Combat bonuses when equipped
    int attack_bonus = 0;                 ///< Attack/accuracy bonus
    int damage_bonus = 0;                 ///< Damage bonus when equipped
    int defense_bonus = 0;                ///< Defense bonus when equipped
    int armor_bonus = 0;                  ///< Armor class bonus

    // Weapon properties
    int min_damage = 1;                   ///< Minimum weapon damage
    int max_damage = 4;                   ///< Maximum weapon damage
    int range = 1;                        ///< Attack range (1 for melee)
    bool two_handed = false;              ///< Requires both hands

    // Equipment slot
    int equipment_slot = -1;              ///< Which slot it equips to

    // Requirements
    int required_level = 0;               ///< Minimum level to equip
    int required_strength = 0;            ///< Minimum strength to equip
    int required_dexterity = 0;           ///< Minimum dexterity to equip
    int required_intelligence = 0;        ///< Minimum intelligence to equip

    ItemComponent() = default;

    ItemComponent(const std::string& item_name, ItemType type, float item_weight = 1.0f)
        : name(item_name), item_type(type), weight(item_weight) {}

    /**
     * @brief Check if item can stack with another
     * @param other Other item component
     * @return true if stackable
     */
    bool canStackWith(const ItemComponent& other) const {
        return name == other.name &&
               item_type == other.item_type &&
               max_stack > 1;
    }

    /**
     * @brief Try to add to stack
     * @param amount Amount to add
     * @return Actual amount added
     */
    int addToStack(int amount) {
        int space = max_stack - stack_size;
        int added = std::min(amount, space);
        stack_size += added;
        return added;
    }

    /**
     * @brief Remove from stack
     * @param amount Amount to remove
     * @return Actual amount removed
     */
    int removeFromStack(int amount) {
        int removed = std::min(amount, stack_size);
        stack_size -= removed;
        return removed;
    }

    std::string getTypeName() const override { return "ItemComponent"; }
    ComponentType getType() const override { return ComponentType::ITEM_DATA; }
};

} // namespace ecs