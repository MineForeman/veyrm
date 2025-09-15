/**
 * @file inventory_system.cpp
 * @brief Implementation of inventory management system
 */

// All includes at global scope
#include <algorithm>
#include <sstream>
#include <deque>
#include <memory>
#include <exception>
#include <variant>

#include "ecs/inventory_system.h"
#include "ecs/renderable_component.h"
#include "ecs/health_component.h"
#include "ecs/combat_component.h"
#include "ecs/position_component.h"

// Forward declare to avoid includes
class Map;
class MessageLog;

// Only now open the namespace
namespace ecs {

InventorySystem::InventorySystem(Map* map, ILogger* logger)
    : map(map)
    , logger(logger) {

    // Subscribe to inventory-related events
    auto& event_system = EventSystem::getInstance();

    event_system.subscribe(EventType::PICKUP,
        [this](const BaseEvent& e) { handlePickupEvent(e); });

    event_system.subscribe(EventType::DROP,
        [this](const BaseEvent& e) { handleDropEvent(e); });

    event_system.subscribe(EventType::USE_ITEM,
        [this](const BaseEvent& e) { handleUseItemEvent(e); });
}

InventorySystem::~InventorySystem() = default;

void InventorySystem::update(const std::vector<std::unique_ptr<Entity>>& entities, double) {
    // Process any pending inventory actions
    // Check for auto-pickup for player on items
    for (const auto& entity : entities) {
        if (!entity->hasComponent<InventoryComponent>()) continue;

        auto* inv = entity->getComponent<InventoryComponent>();
        auto* pos = entity->getComponent<PositionComponent>();
        if (!pos) continue;

        // Update inventory weight
        inv->current_weight = getTotalWeight(*inv, entities);

        // Auto-pickup for player (if enabled) - commented out as features not yet implemented
        // if (inv->auto_pickup && entity->hasTag("player")) {
        //     auto items_here = getItemsAt(entities, pos->position.x, pos->position.y);
        //     for (Entity* item : items_here) {
        //         if (canPickup(entity.get(), item)) {
        //             pickupItem(entity.get(), item);
        //         }
        //     }
        // }
    }
}

bool InventorySystem::pickupItem(Entity* picker, Entity* item) {
    if (!picker || !item) return false;

    auto* inventory = picker->getComponent<InventoryComponent>();
    auto* item_comp = item->getComponent<ItemComponent>();
    auto* item_pos = item->getComponent<PositionComponent>();

    if (!inventory || !item_comp) return false;

    // Check if inventory has space
    if (inventory->isFull()) {
        if (logger) {
            logger->logSystem("Inventory is full!");
        }
        return false;
    }

    // Check weight limit
    float new_weight = inventory->current_weight + item_comp->weight;
    if (new_weight > inventory->max_weight) {
        if (logger) {
            logger->logSystem("Too heavy to carry!");
        }
        return false;
    }

    // Check if items are at same position
    auto* picker_pos = picker->getComponent<PositionComponent>();
    if (picker_pos && item_pos) {
        if (!picker_pos->isAt(item_pos->position.x, item_pos->position.y)) {
            if (logger) {
                logger->logSystem("Item is too far away!");
            }
            return false;
        }
    }

    // Add item to inventory
    if (inventory->addItem(item->getID())) {
        inventory->current_weight = new_weight;

        // Remove item from map
        item->removeComponent<PositionComponent>();

        if (logger) {
            std::stringstream msg;
            msg << getEntityName(picker) << " picks up " << item_comp->name;
            logger->logSystem(msg.str());
        }

        // Fire pickup event
        EventSystem::getInstance().emit(PickupEvent(item->getID(), picker->getID()));

        return true;
    }

    return false;
}

bool InventorySystem::dropItem(Entity* dropper, EntityID item_id) {
    if (!dropper) return false;

    auto* inventory = dropper->getComponent<InventoryComponent>();
    auto* dropper_pos = dropper->getComponent<PositionComponent>();

    if (!inventory || !dropper_pos) return false;

    // Check if entity has the item
    if (!inventory->hasItem(item_id)) {
        if (logger) {
            logger->logSystem("You don't have that item!");
        }
        return false;
    }

    // Remove from inventory
    if (inventory->removeItem(item_id)) {
        // Update weight
        inventory->current_weight = getTotalWeight(*inventory,
            std::vector<std::unique_ptr<Entity>>());

        // Fire drop event with position
        EventSystem::getInstance().emit(
            DropEvent(dropper->getID(), item_id,
                     dropper_pos->position.x, dropper_pos->position.y)
        );

        if (logger) {
            logger->logSystem("Item dropped");
        }

        return true;
    }

    return false;
}

bool InventorySystem::useItem(Entity* user, EntityID item_id, Entity* target) {
    if (!user) return false;

    auto* inventory = user->getComponent<InventoryComponent>();
    if (!inventory || !inventory->hasItem(item_id)) {
        if (logger) {
            logger->logSystem("You don't have that item!");
        }
        return false;
    }

    // Fire use item event
    EventSystem::getInstance().emit(
        UseItemEvent(user->getID(), item_id, target ? target->getID() : user->getID())
    );

    return true;
}

std::vector<Entity*> InventorySystem::getItemsAt(
    const std::vector<std::unique_ptr<Entity>>& entities,
    int x, int y) {

    std::vector<Entity*> items;

    for (const auto& entity : entities) {
        auto* item = entity->getComponent<ItemComponent>();
        auto* pos = entity->getComponent<PositionComponent>();

        if (item && pos && pos->position.x == x && pos->position.y == y) {
            items.push_back(entity.get());
        }
    }

    return items;
}

bool InventorySystem::canPickup(Entity* entity, Entity* item) {
    if (!entity || !item) return false;

    auto* inventory = entity->getComponent<InventoryComponent>();
    auto* item_comp = item->getComponent<ItemComponent>();

    if (!inventory || !item_comp) return false;

    // Check space and weight
    return !inventory->isFull() &&
           (inventory->current_weight + item_comp->weight <= inventory->max_weight);
}

float InventorySystem::getTotalWeight(const InventoryComponent& inventory,
                                      const std::vector<std::unique_ptr<Entity>>& entities) {
    float total = 0.0f;

    for (EntityID item_id : inventory.items) {
        Entity* item = findEntity(entities, item_id);
        if (item) {
            auto* item_comp = item->getComponent<ItemComponent>();
            if (item_comp) {
                total += item_comp->weight * item_comp->stack_size;
            }
        }
    }

    return total;
}

void InventorySystem::handlePickupEvent([[maybe_unused]] const BaseEvent& event) {
    // Event.source_id = picker, event.target_id = item
    // Already handled in pickupItem, this is for other systems to react
}

void InventorySystem::handleDropEvent([[maybe_unused]] const BaseEvent& event) {
    // Need to add position component back to dropped item
    // event.source_id = dropper, event.target_id = item
    // event.value1 = x, event.value2 = y

    // This would need access to the world's entity list
    // For now, handled externally
}

void InventorySystem::handleUseItemEvent([[maybe_unused]] const BaseEvent& event) {
    // Apply item effects based on item type
    // event.source_id = user
    // event.value1 = item_id
    // event.target_id = target entity

    // This needs access to entities - handled in update loop
}

void InventorySystem::applyItemEffects(Entity* user, const ItemComponent& item, Entity* target) {
    if (!target) target = user;

    // Apply healing
    if (item.heal_amount > 0) {
        auto* health = target->getComponent<HealthComponent>();
        if (health) {
            int old_hp = health->hp;
            health->hp = std::min(health->hp + item.heal_amount, health->max_hp);
            int healed = health->hp - old_hp;

            if (logger && healed > 0) {
                std::stringstream msg;
                msg << getEntityName(target) << " healed for " << healed << " HP";
                logger->logSystem(msg.str());
            }

            // Fire healing event
            EventSystem::getInstance().emit(
                DamageEvent(user->getID(), target->getID(), -healed, "healing")
            );
        }
    }

    // Apply damage (for offensive items)
    if (item.damage_amount > 0) {
        auto* health = target->getComponent<HealthComponent>();
        if (health) {
            health->hp -= item.damage_amount;

            if (logger) {
                std::stringstream msg;
                msg << getEntityName(target) << " takes " << item.damage_amount << " damage";
                logger->logSystem(msg.str());
            }

            // Fire damage event
            EventSystem::getInstance().emit(
                DamageEvent(user->getID(), target->getID(), item.damage_amount, "item")
            );
        }
    }

    // Apply stat buffs
    if (item.attack_bonus != 0 || item.defense_bonus != 0) {
        auto* combat = target->getComponent<CombatComponent>();
        if (combat) {
            combat->attack_modifier += item.attack_bonus;
            combat->defense_modifier += item.defense_bonus;

            if (logger) {
                std::stringstream msg;
                msg << getEntityName(target) << " gains ";
                if (item.attack_bonus > 0) msg << "+" << item.attack_bonus << " attack ";
                if (item.defense_bonus > 0) msg << "+" << item.defense_bonus << " defense";
                logger->logSystem(msg.str());
            }
        }
    }

    // Apply mana restoration
    if (item.mana_amount > 0) {
        // ManaComponent would need to be implemented
        if (logger) {
            std::stringstream msg;
            msg << getEntityName(target) << " restores " << item.mana_amount << " mana (not yet implemented)";
            logger->logSystem(msg.str());
        }
    }

    // Apply other effects based on item type
    switch (item.item_type) {
        case ItemType::FOOD:
            // Restore stamina or satisfy hunger
            if (logger) {
                logger->logSystem(getEntityName(target) + " eats " + item.name);
            }
            break;
        case ItemType::SCROLL:
            // Cast spell effect
            if (logger) {
                logger->logSystem(getEntityName(target) + " reads " + item.name);
            }
            break;
        case ItemType::WEAPON:
        case ItemType::ARMOR:
            // These should be equipped, not used
            if (logger) {
                logger->logSystem("This item should be equipped, not used");
            }
            return;  // Can't use equipment items this way
        default:
            break;
    }

    // Item effects have been applied
}

Entity* InventorySystem::findEntity(const std::vector<std::unique_ptr<Entity>>& entities,
                                   EntityID id) {
    auto it = std::find_if(entities.begin(), entities.end(),
        [id](const std::unique_ptr<Entity>& e) {
            return e && e->getID() == id;
        });

    return (it != entities.end()) ? it->get() : nullptr;
}

std::string InventorySystem::getEntityName(Entity* entity) {
    if (!entity) return "unknown";

    auto* renderable = entity->getComponent<RenderableComponent>();
    if (renderable && !renderable->name.empty()) {
        return renderable->name;
    }

    auto* item = entity->getComponent<ItemComponent>();
    if (item && !item->name.empty()) {
        return item->name;
    }

    return "entity_" + std::to_string(entity->getID());
}

} // namespace ecs