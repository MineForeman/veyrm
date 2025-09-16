/**
 * @file equipment_component.h
 * @brief Equipment slots and equipped items component
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "component.h"
#include "entity.h"
#include <unordered_map>
#include <string>

namespace ecs {

/**
 * @enum EquipmentSlot
 * @brief Equipment slot types
 */
enum class EquipmentSlot {
    NONE,           ///< No slot/invalid
    MAIN_HAND,      ///< Primary weapon
    OFF_HAND,       ///< Shield or secondary weapon
    HEAD,           ///< Helmet
    BODY,           ///< Armor
    HANDS,          ///< Gloves
    FEET,           ///< Boots
    NECK,           ///< Amulet
    RING_LEFT,      ///< Left ring
    RING_RIGHT,     ///< Right ring
    BACK,           ///< Cloak
    BELT,           ///< Belt
    RANGED          ///< Bow/crossbow
};

/**
 * @class EquipmentComponent
 * @brief Manages equipped items
 */
class EquipmentComponent : public Component<EquipmentComponent> {
public:
    // Map of equipment slots to item entity IDs
    std::unordered_map<EquipmentSlot, EntityID> equipped_items;

    // Cached total bonuses from all equipment
    int total_attack_bonus = 0;
    int total_defense_bonus = 0;
    int total_damage_bonus = 0;
    int total_armor_class = 0;
    int total_resistance = 0;

    EquipmentComponent() = default;

    /**
     * @brief Equip an item in a slot
     * @param slot Equipment slot
     * @param item_id Item entity ID
     * @return Previous item ID if any (0 if empty)
     */
    EntityID equip(EquipmentSlot slot, EntityID item_id) {
        EntityID previous = 0;

        auto it = equipped_items.find(slot);
        if (it != equipped_items.end()) {
            previous = it->second;
        }

        equipped_items[slot] = item_id;
        return previous;
    }

    /**
     * @brief Unequip item from slot
     * @param slot Equipment slot
     * @return Item ID if any (0 if empty)
     */
    EntityID unequip(EquipmentSlot slot) {
        auto it = equipped_items.find(slot);
        if (it != equipped_items.end()) {
            EntityID item = it->second;
            equipped_items.erase(it);
            return item;
        }
        return 0;
    }

    /**
     * @brief Check if slot has an item
     * @param slot Equipment slot
     * @return true if equipped
     */
    bool hasEquipped(EquipmentSlot slot) const {
        return equipped_items.find(slot) != equipped_items.end();
    }

    /**
     * @brief Get item in slot
     * @param slot Equipment slot
     * @return Item ID or 0
     */
    EntityID getEquipped(EquipmentSlot slot) const {
        auto it = equipped_items.find(slot);
        return (it != equipped_items.end()) ? it->second : 0;
    }

    /**
     * @brief Check if can equip item in slot
     * @param slot Equipment slot
     * @param two_handed If item requires both hands
     * @return true if can equip
     */
    bool canEquip(EquipmentSlot slot, bool two_handed = false) const {
        if (two_handed && slot == EquipmentSlot::MAIN_HAND) {
            // Two-handed weapons require off-hand to be free
            return !hasEquipped(EquipmentSlot::OFF_HAND);
        }
        return true;
    }

    /**
     * @brief Recalculate total bonuses
     * @note Should be called when equipment changes
     */
    void recalculateBonuses() {
        // This would need access to item components
        // Implemented by EquipmentSystem
        total_attack_bonus = 0;
        total_defense_bonus = 0;
        total_damage_bonus = 0;
        total_armor_class = 0;
        total_resistance = 0;
    }

    std::string getTypeName() const override { return "EquipmentComponent"; }
    ComponentType getType() const override { return ComponentType::CUSTOM; }
};

} // namespace ecs