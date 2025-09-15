/**
 * @file input_system.cpp
 * @brief Implementation of ECS input system
 */

#include <algorithm>
#include <vector>
#include <memory>

#include "ecs/input_system.h"
#include "ecs/movement_system.h"
#include "ecs/combat_system.h"
#include "ecs/inventory_system.h"
#include "ecs/combat_component.h"
#include "ecs/inventory_component.h"
#include "ecs/item_component.h"
#include "ecs/event.h"

namespace ecs {

InputSystem::InputSystem(MovementSystem* movement,
                        CombatSystem* combat,
                        InventorySystem* inventory)
    : movement_system(movement)
    , combat_system(combat)
    , inventory_system(inventory) {
}

void InputSystem::update(const std::vector<std::unique_ptr<Entity>>& entities, double) {
    // Process input for all entities with InputComponent
    for (const auto& entity : entities) {
        if (!shouldProcess(*entity)) continue;

        auto* input = entity->getComponent<InputComponent>();
        if (!input) continue;

        // Check for input callback if waiting and is player
        if (input->is_player && input->waiting_for_input && input_callback) {
            InputCommand cmd = input_callback();
            if (cmd.action != InputAction::NONE) {
                input->queueCommand(cmd);
            }
        }

        // Process queued commands
        while (input->hasCommands()) {
            InputCommand cmd = input->getNextCommand();
            ActionSpeed speed = processCommand(entity.get(), cmd, entities);
            input->last_action_speed = speed;

            // For turn-based, process one command per update
            break;
        }
    }
}

void InputSystem::queuePlayerInput(EntityID, const InputCommand&) {
    // Find player entity and queue command
    // This would need access to world's entity list
    // For now, handled externally through InputComponent
}

InputCommand InputSystem::keyToCommand(int key, bool shift, [[maybe_unused]] bool ctrl) {
    InputCommand cmd;

    // Arrow keys and numpad
    switch (key) {
        // Movement
        case 72: case '8': // Up
            cmd.action = InputAction::MOVE;
            cmd.dy = -1;
            break;
        case 80: case '2': // Down
            cmd.action = InputAction::MOVE;
            cmd.dy = 1;
            break;
        case 75: case '4': // Left
            cmd.action = InputAction::MOVE;
            cmd.dx = -1;
            break;
        case 77: case '6': // Right
            cmd.action = InputAction::MOVE;
            cmd.dx = 1;
            break;
        case 71: case '7': // Up-Left
            cmd.action = InputAction::MOVE;
            cmd.dx = -1;
            cmd.dy = -1;
            break;
        case 73: case '9': // Up-Right
            cmd.action = InputAction::MOVE;
            cmd.dx = 1;
            cmd.dy = -1;
            break;
        case 79: case '1': // Down-Left
            cmd.action = InputAction::MOVE;
            cmd.dx = -1;
            cmd.dy = 1;
            break;
        case 81: case '3': // Down-Right
            cmd.action = InputAction::MOVE;
            cmd.dx = 1;
            cmd.dy = 1;
            break;

        // Actions
        case 'g': // Get/Pickup (lowercase g only)
            cmd.action = InputAction::PICKUP;
            break;
        case 'd': // Drop (shift+D for drop menu)
            if (shift) {
                cmd.action = InputAction::DROP;
            }
            break;
        case 'u': // Use item
            cmd.action = InputAction::USE_ITEM;
            break;
        case 'i': // Inventory
            cmd.action = InputAction::OPEN_INVENTORY;
            break;
        case '.': case '5': // Wait
            cmd.action = InputAction::WAIT;
            break;
        case 'q': // Quit (lowercase q only)
            cmd.action = InputAction::QUIT;
            break;

        default:
            cmd.action = InputAction::NONE;
            break;
    }

    return cmd;
}

bool InputSystem::isWaitingForInput([[maybe_unused]] EntityID player_id) const {
    // Would need entity access
    return true;
}

ActionSpeed InputSystem::processCommand(Entity* entity,
                                       const InputCommand& command,
                                       const std::vector<std::unique_ptr<Entity>>& entities) {
    switch (command.action) {
        case InputAction::MOVE:
            return processMove(entity, command.dx, command.dy, entities);

        case InputAction::PICKUP:
            return processPickup(entity, entities);

        case InputAction::DROP:
            return processDrop(entity, command.item_slot);

        case InputAction::USE_ITEM:
            return processUseItem(entity, command.item_slot, command.target_id);

        case InputAction::WAIT:
            return ActionSpeed::NORMAL;

        case InputAction::OPEN_INVENTORY:
            // UI action, no time cost
            return ActionSpeed::INSTANT;

        case InputAction::QUIT:
            // Game control action
            return ActionSpeed::INSTANT;

        default:
            return ActionSpeed::INSTANT;
    }
}

ActionSpeed InputSystem::processMove(Entity* entity, int dx, int dy,
                                    const std::vector<std::unique_ptr<Entity>>& entities) {
    if (!entity || !movement_system) return ActionSpeed::NORMAL;

    auto* pos = entity->getComponent<PositionComponent>();
    if (!pos) return ActionSpeed::NORMAL;

    int new_x = pos->position.x + dx;
    int new_y = pos->position.y + dy;

    // Check for combat (bump to attack)
    for (const auto& other : entities) {
        if (other->getID() == entity->getID()) continue;

        auto* other_pos = other->getComponent<PositionComponent>();
        auto* other_combat = other->getComponent<CombatComponent>();

        if (other_pos && other_combat && other_pos->isAt(new_x, new_y)) {
            // Attack the target
            if (combat_system) {
                combat_system->queueAttack(entity->getID(), other->getID());
            }
            return ActionSpeed::NORMAL;
        }
    }

    // Move if no combat
    if (movement_system->moveEntityTo(*entity, new_x, new_y)) {
        return ActionSpeed::NORMAL;
    }

    return ActionSpeed::INSTANT; // Failed move costs no time
}

ActionSpeed InputSystem::processPickup(Entity* entity,
                                      const std::vector<std::unique_ptr<Entity>>& entities) {
    if (!entity || !inventory_system) return ActionSpeed::FAST;

    auto* pos = entity->getComponent<PositionComponent>();
    if (!pos) return ActionSpeed::FAST;

    // Find items at position
    for (const auto& other : entities) {
        auto* item_pos = other->getComponent<PositionComponent>();
        auto* item_comp = other->getComponent<ItemComponent>();

        if (item_pos && item_comp && item_pos->isAt(pos->position.x, pos->position.y)) {
            if (inventory_system->pickupItem(entity, other.get())) {
                return ActionSpeed::FAST;
            }
        }
    }

    return ActionSpeed::INSTANT; // No item to pickup
}

ActionSpeed InputSystem::processDrop(Entity* entity, int item_slot) {
    if (!entity || !inventory_system) return ActionSpeed::FAST;

    auto* inventory = entity->getComponent<InventoryComponent>();
    if (!inventory || item_slot < 0 || item_slot >= (int)inventory->items.size()) {
        return ActionSpeed::INSTANT;
    }

    EntityID item_id = inventory->items[item_slot];
    if (inventory_system->dropItem(entity, item_id)) {
        return ActionSpeed::FAST;
    }

    return ActionSpeed::INSTANT;
}

ActionSpeed InputSystem::processUseItem(Entity* entity, int item_slot, EntityID target_id) {
    if (!entity || !inventory_system) return ActionSpeed::NORMAL;

    auto* inventory = entity->getComponent<InventoryComponent>();
    if (!inventory || item_slot < 0 || item_slot >= (int)inventory->items.size()) {
        return ActionSpeed::INSTANT;
    }

    EntityID item_id = inventory->items[item_slot];

    // Find target entity (default to self)
    Entity* target = (target_id != 0) ? findEntity(std::vector<std::unique_ptr<Entity>>(), target_id)
                                      : entity;

    if (inventory_system->useItem(entity, item_id, target)) {
        return ActionSpeed::NORMAL;
    }

    return ActionSpeed::INSTANT;
}

Entity* InputSystem::findEntity(const std::vector<std::unique_ptr<Entity>>& entities,
                               EntityID id) const {
    auto it = std::find_if(entities.begin(), entities.end(),
        [id](const std::unique_ptr<Entity>& e) {
            return e && e->getID() == id;
        });

    return (it != entities.end()) ? it->get() : nullptr;
}

} // namespace ecs