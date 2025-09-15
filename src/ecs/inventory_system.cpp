/**
 * @file inventory_system.cpp
 * @brief Implementation of inventory management system
 */

#include <algorithm>
#include <sstream>
#include <deque>
#include <memory>
#include <exception>
#include <variant>
#include "ecs/inventory_system.h"
#include "ecs/renderable_component.h"
#include "ecs/health_component.h"

namespace ecs {

InventorySystem::InventorySystem(Map* map, MessageLog* message_log)
    : map(map)
    , message_log(message_log) {

    // Subscribe to inventory-related events
    auto& system = EventSystem::getInstance();

    system.subscribe(EventType::PICKUP,
        [this](const BaseEvent& e) { handlePickupEvent(e); });

    system.subscribe(EventType::DROP,
        [this](const BaseEvent& e) { handleDropEvent(e); });

    system.subscribe(EventType::USE_ITEM,
        [this](const BaseEvent& e) { handleUseItemEvent(e); });
}

InventorySystem::~InventorySystem() {
    // Event subscriptions are cleaned up automatically
}

void InventorySystem::update(const std::vector<std::unique_ptr<Entity>>& entities, double) {
    // Process any pending inventory actions
    // Most work is done through event handlers

    // Could add automatic pickup for player standing on items
    for (const auto& entity : entities) {
        if (!entity->hasComponent<InventoryComponent>()) continue;

        auto* inv = entity->getComponent<InventoryComponent>();
        auto* pos = entity->getComponent<PositionComponent>();
        if (!pos) continue;

        // Update inventory weight
        inv->current_weight = getTotalWeight(*inv, entities);
    }
}

bool InventorySystem::pickupItem(Entity* picker, Entity* item) {
    if (!picker || !item) return false;

    auto* inventory = picker->getComponent<InventoryComponent>();
    auto* item_comp = item->getComponent<ItemComponent>();

    if (!inventory || !item_comp) return false;

    // Check if inventory has space
    if (inventory->isFull()) {
        if (message_log) {
            message_log->addSystemMessage("Inventory is full!");
        }
        return false;
    }

    // Check weight limit
    float new_weight = inventory->current_weight + item_comp->weight;
    if (new_weight > inventory->max_weight) {
        if (message_log) {
            message_log->addSystemMessage("Too heavy to carry!");
        }
        return false;
    }

    // Add item to inventory
    if (inventory->addItem(item->getID())) {
        inventory->current_weight = new_weight;

        // Remove item's position component so it's not on the map
        item->removeComponent<PositionComponent>();

        if (message_log) {
            std::stringstream msg;
            msg << getEntityName(picker) << " picks up " << item_comp->name;
            message_log->addSystemMessage(msg.str());
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
        return false;
    }

    // Remove from inventory
    if (inventory->removeItem(item_id)) {
        // Fire drop event with position
        EventSystem::getInstance().emit(
            DropEvent(dropper->getID(), item_id,
                     dropper_pos->position.x, dropper_pos->position.y)
        );

        if (message_log) {
            message_log->addSystemMessage("Item dropped");
        }

        return true;
    }

    return false;
}

bool InventorySystem::useItem(Entity* user, EntityID item_id, Entity* target) {
    if (!user) return false;

    auto* inventory = user->getComponent<InventoryComponent>();
    if (!inventory || !inventory->hasItem(item_id)) {
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

void InventorySystem::handlePickupEvent(const BaseEvent& event) {
    // Already handled in pickupItem, this is for other systems to react
}

void InventorySystem::handleDropEvent(const BaseEvent& event) {
    // Add position component back to dropped item at event.value1, event.value2
    // This would need access to the entity system
    // For now, this is a placeholder
}

void InventorySystem::handleUseItemEvent(const BaseEvent& event) {
    // Apply item effects based on item type
    // event.value1 has the item ID
    // event.target_id has the target entity
    // This would need to look up the item and target entities
    // and apply appropriate effects
}

void InventorySystem::applyItemEffects(Entity* user, const ItemComponent& item, Entity* target) {
    if (!target) target = user;

    // Apply healing
    if (item.heal_amount > 0) {
        auto* health = target->getComponent<HealthComponent>();
        if (health) {
            health->hp = std::min(health->hp + item.heal_amount, health->max_hp);

            if (message_log) {
                std::stringstream msg;
                msg << getEntityName(target) << " healed for " << item.heal_amount << " HP";
                message_log->addSystemMessage(msg.str());
            }

            // Fire healing event
            EventSystem::getInstance().emit(
                DamageEvent(user->getID(), target->getID(), -item.heal_amount, "healing")
            );
        }
    }

    // Apply other effects as needed
}

Entity* InventorySystem::findEntity(const std::vector<std::unique_ptr<Entity>>& entities,
                                   EntityID id) {
    auto it = std::find_if(entities.begin(), entities.end(),
        [id](const std::unique_ptr<Entity>& e) {
            return e->getID() == id;
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
    if (item) {
        return item->name;
    }

    return "entity_" + std::to_string(entity->getID());
}

} // namespace ecs