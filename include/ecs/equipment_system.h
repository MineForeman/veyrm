/**
 * @file equipment_system.h
 * @brief System for managing equipment and gear
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "system.h"
#include "entity.h"
#include "equipment_component.h"
#include "item_component.h"
#include "stats_component.h"
#include "combat_component.h"
#include "inventory_component.h"
#include "logger_interface.h"
#include <memory>
#include <string>

namespace ecs {

/**
 * @struct EquipmentBonuses
 * @brief Aggregate bonuses from all equipped items
 */
struct EquipmentBonuses {
    int attack_bonus = 0;
    int damage_bonus = 0;
    int defense_bonus = 0;
    int armor_bonus = 0;
    int speed_bonus = 0;
    int strength_bonus = 0;
    int dexterity_bonus = 0;
    int intelligence_bonus = 0;
    int constitution_bonus = 0;
    int wisdom_bonus = 0;
    int charisma_bonus = 0;

    // Resistances
    int fire_resistance = 0;
    int cold_resistance = 0;
    int poison_resistance = 0;
    int magic_resistance = 0;

    // Special
    int critical_chance = 0;
    int critical_damage = 0;
    int life_steal = 0;
    int mana_steal = 0;

    void addItemBonuses(const ItemComponent& item) {
        attack_bonus += item.attack_bonus;
        damage_bonus += item.damage_bonus;
        defense_bonus += item.defense_bonus;
        // Add other bonuses as needed
    }
};

/**
 * @class EquipmentSystem
 * @brief Manages equipment slots and bonuses
 */
// Forward declaration
class World;

class EquipmentSystem : public System<EquipmentSystem> {
public:
    EquipmentSystem(ILogger* logger = nullptr, World* world = nullptr);
    ~EquipmentSystem() = default;

    void update(const std::vector<std::unique_ptr<Entity>>& entities, double delta_time) override;

    /**
     * @brief Equip an item to appropriate slot
     * @param entity Entity equipping the item
     * @param item Item to equip
     * @return Previous item in slot (nullptr if empty)
     */
    std::unique_ptr<Entity> equipItem(Entity* entity, Entity* item);

    /**
     * @brief Unequip item from specific slot
     * @param entity Entity unequipping
     * @param slot Equipment slot
     * @return Unequipped item (nullptr if slot was empty)
     */
    std::unique_ptr<Entity> unequipSlot(Entity* entity, EquipmentSlot slot);

    /**
     * @brief Get equipped item in slot
     * @param entity Entity to check
     * @param slot Equipment slot
     * @return Item entity or nullptr
     */
    Entity* getEquippedItem(const Entity* entity, EquipmentSlot slot) const;

    /**
     * @brief Check if item can be equipped
     * @param entity Entity trying to equip
     * @param item Item to check
     * @return true if item can be equipped
     */
    bool canEquip(const Entity* entity, const Entity* item) const;

    /**
     * @brief Calculate total equipment bonuses
     * @param entity Entity with equipment
     * @return Equipment stat bonuses
     */
    EquipmentBonuses calculateBonuses(const Entity* entity) const;

    /**
     * @brief Apply equipment bonuses to entity stats
     * @param entity Entity to update
     */
    void applyEquipmentBonuses(Entity* entity);

    /**
     * @brief Get equipment slot for item type
     * @param item Item to check
     * @return Equipment slot or NONE
     */
    static EquipmentSlot getSlotForItem(const Entity* item);

    int getPriority() const override { return 30; }

    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<EquipmentComponent>();
    }

    /**
     * @brief Set world reference for entity lookups
     * @param world World instance
     */
    void setWorld(World* world) { this->world = world; }

private:
    ILogger* logger;
    World* world = nullptr;  ///< World for entity lookups

    /**
     * @brief Update entity stats based on equipment
     * @param entity Entity to update
     */
    void updateEquipmentStats(Entity* entity);

    /**
     * @brief Check if slot is valid for item
     * @param item Item to check
     * @param slot Target slot
     * @return true if item can go in slot
     */
    bool isValidSlot(const Entity* item, EquipmentSlot slot) const;

    /**
     * @brief Handle two-handed weapon restrictions
     * @param entity Entity equipping
     * @param item Item being equipped
     * @return Item that needs to be unequipped or nullptr
     */
    Entity* handleTwoHandedRestrictions(Entity* entity, const Entity* item);
};

} // namespace ecs