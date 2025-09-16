/**
 * @file equipment_system.cpp
 * @brief Implementation of equipment system
 */

#include <sstream>
#include "ecs/equipment_system.h"
#include "ecs/renderable_component.h"
#include "ecs/experience_component.h"
#include "ecs/system_manager.h"  // For World class

namespace ecs {

// Forward declaration for IntelliSense (already included above)
class ExperienceComponent;

EquipmentSystem::EquipmentSystem(ILogger* logger, World* world)
    : logger(logger), world(world) {
}

void EquipmentSystem::update(const std::vector<std::unique_ptr<Entity>>& entities, double) {
    for (const auto& entity : entities) {
        if (!shouldProcess(*entity)) continue;

        // Apply equipment bonuses each frame
        applyEquipmentBonuses(entity.get());
    }
}

std::unique_ptr<Entity> EquipmentSystem::equipItem(Entity* entity, Entity* item) {
    if (!entity || !item) return nullptr;

    auto* equipment = entity->getComponent<EquipmentComponent>();
    if (!equipment) {
        entity->addComponent<EquipmentComponent>();
        equipment = entity->getComponent<EquipmentComponent>();
    }

    auto* item_comp = item->getComponent<ItemComponent>();
    if (!item_comp || !item_comp->equippable) {
        if (logger) {
            logger->logSystem("Item cannot be equipped");
        }
        return nullptr;
    }

    // Determine slot
    EquipmentSlot slot = getSlotForItem(item);
    // Note: There's no NONE value in EquipmentSlot, check if we got a valid slot
    // by trying to equip and seeing if it works

    // Check requirements
    if (!canEquip(entity, item)) {
        if (logger) {
            std::stringstream msg;
            msg << "Cannot equip " << item_comp->name << " - requirements not met";
            logger->logSystem(msg.str());
        }
        return nullptr;
    }

    // Handle two-handed weapons
    std::unique_ptr<Entity> unequipped_offhand;
    if (item_comp->two_handed && slot == EquipmentSlot::MAIN_HAND) {
        // Unequip offhand if present
        [[maybe_unused]] EntityID offhand_id = equipment->getEquipped(EquipmentSlot::OFF_HAND);
        if (offhand_id != 0) {
            unequipped_offhand = unequipSlot(entity, EquipmentSlot::OFF_HAND);
            if (logger) {
                logger->logSystem("Unequipped offhand to equip two-handed weapon");
            }
        }
    }

    // Check if offhand is being equipped while two-handed weapon is equipped
    if (slot == EquipmentSlot::OFF_HAND) {
        // Would need to check if main hand has a two-handed weapon
        // For now, skip this check
    }

    // Equip the item using the map-based approach
    [[maybe_unused]] EntityID previous_id = equipment->equip(slot, item->getID());

    // Get the previous item if there was one
    std::unique_ptr<Entity> previous;
    // Note: We'd need to get the actual entity from previous_id
    // For now, just track that something was unequipped

    // Update stats
    applyEquipmentBonuses(entity);

    if (logger) {
        auto* renderable = entity->getComponent<RenderableComponent>();
        std::string entity_name = renderable ? renderable->name : "Entity";
        std::stringstream msg;
        msg << entity_name << " equipped " << item_comp->name;
        logger->logSystem(msg.str());
    }

    return previous;
}

std::unique_ptr<Entity> EquipmentSystem::unequipSlot(Entity* entity, EquipmentSlot slot) {
    if (!entity) return nullptr;

    auto* equipment = entity->getComponent<EquipmentComponent>();
    if (!equipment) return nullptr;

    // Unequip using the map-based approach
    [[maybe_unused]] EntityID unequipped_id = equipment->unequip(slot);

    std::unique_ptr<Entity> unequipped;
    // Note: We'd need to get the actual entity from unequipped_id
    // For now, return nullptr

    if (unequipped) {
        // Update stats
        applyEquipmentBonuses(entity);

        if (logger) {
            auto* item = unequipped->getComponent<ItemComponent>();
            auto* renderable = entity->getComponent<RenderableComponent>();
            std::string entity_name = renderable ? renderable->name : "Entity";
            std::stringstream msg;
            msg << entity_name << " unequipped " << (item ? item->name : "item");
            logger->logSystem(msg.str());
        }
    }

    return unequipped;
}

Entity* EquipmentSystem::getEquippedItem(const Entity* entity, EquipmentSlot slot) const {
    if (!entity) return nullptr;

    auto* equipment = entity->getComponent<EquipmentComponent>();
    if (!equipment) return nullptr;

    // Get equipped item ID from map
    EntityID item_id = equipment->getEquipped(slot);

    if (item_id == 0 || !world) return nullptr;

    return world->getEntity(item_id);
}

bool EquipmentSystem::canEquip(const Entity* entity, const Entity* item) const {
    if (!entity || !item) return false;

    auto* item_comp = item->getComponent<ItemComponent>();
    if (!item_comp || !item_comp->equippable) return false;

    auto* stats = entity->getComponent<StatsComponent>();
    if (!stats) return true;  // No stats means no requirements

    // Check stat requirements
    if (item_comp->required_strength > 0 && stats->strength < item_comp->required_strength) {
        return false;
    }
    if (item_comp->required_dexterity > 0 && stats->dexterity < item_comp->required_dexterity) {
        return false;
    }
    if (item_comp->required_intelligence > 0 && stats->intelligence < item_comp->required_intelligence) {
        return false;
    }

    // Check level requirement
    auto* exp = entity->getComponent<ExperienceComponent>();
    if (exp && item_comp->required_level > 0 && exp->level < item_comp->required_level) {
        return false;
    }

    return true;
}

EquipmentBonuses EquipmentSystem::calculateBonuses(const Entity* entity) const {
    EquipmentBonuses bonuses;

    if (!entity) return bonuses;

    auto* equipment = entity->getComponent<EquipmentComponent>();
    if (!equipment) return bonuses;

    // Add bonuses from each equipped item in the map
    for (const auto& [slot, item_id] : equipment->equipped_items) {
        if (item_id == 0 || !world) continue;

        Entity* item_entity = world->getEntity(item_id);
        if (!item_entity) continue;

        auto* item = item_entity->getComponent<ItemComponent>();
        if (item) {
            bonuses.addItemBonuses(*item);
        }
    }

    return bonuses;
}

void EquipmentSystem::applyEquipmentBonuses(Entity* entity) {
    if (!entity) return;

    auto bonuses = calculateBonuses(entity);

    // Apply to stats
    auto* stats = entity->getComponent<StatsComponent>();
    if (stats) {
        // Store base stats and apply equipment bonuses
        // This would need a way to track base vs modified stats
        stats->recalculateDerived();
    }

    // Apply to combat
    auto* combat = entity->getComponent<CombatComponent>();
    if (combat) {
        combat->attack_bonus = bonuses.attack_bonus;
        combat->damage_modifier = bonuses.damage_bonus;
        combat->defense_bonus = bonuses.defense_bonus;
    }
}

EquipmentSlot EquipmentSystem::getSlotForItem(const Entity* item) {
    if (!item) return EquipmentSlot::NONE;

    auto* item_comp = item->getComponent<ItemComponent>();
    if (!item_comp) return EquipmentSlot::NONE;

    // Map item type to equipment slot
    switch (item_comp->item_type) {
        case ItemType::WEAPON:
            return EquipmentSlot::MAIN_HAND;
        case ItemType::SHIELD:
            return EquipmentSlot::OFF_HAND;
        case ItemType::ARMOR:
            // Could check subtype for specific armor pieces
            return EquipmentSlot::BODY;
        case ItemType::HELMET:
            return EquipmentSlot::HEAD;
        case ItemType::GLOVES:
            return EquipmentSlot::HANDS;
        case ItemType::BOOTS:
            return EquipmentSlot::FEET;
        case ItemType::AMULET:
            return EquipmentSlot::NECK;
        case ItemType::RING:
            // Could alternate between left and right
            return EquipmentSlot::RING_LEFT;
        case ItemType::CLOAK:
            return EquipmentSlot::BACK;
        default:
            return EquipmentSlot::NONE;
    }
}

void EquipmentSystem::updateEquipmentStats(Entity* entity) {
    applyEquipmentBonuses(entity);
}

bool EquipmentSystem::isValidSlot(const Entity* item, EquipmentSlot slot) const {
    if (!item) return false;

    EquipmentSlot expected_slot = getSlotForItem(item);
    return expected_slot == slot ||
           (expected_slot == EquipmentSlot::RING_LEFT && slot == EquipmentSlot::RING_RIGHT);
}

Entity* EquipmentSystem::handleTwoHandedRestrictions(Entity* entity, const Entity* item) {
    if (!entity || !item) return nullptr;

    auto* item_comp = item->getComponent<ItemComponent>();
    auto* equipment = entity->getComponent<EquipmentComponent>();

    if (!item_comp || !equipment) return nullptr;

    if (item_comp->two_handed) {
        // Return offhand item that needs to be unequipped
        EntityID offhand_id = equipment->getEquipped(EquipmentSlot::OFF_HAND);
        if (offhand_id == 0 || !world) return nullptr;
        return world->getEntity(offhand_id);
    }

    return nullptr;
}

} // namespace ecs