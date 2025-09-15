/**
 * @file input_system.h
 * @brief ECS system for handling player input
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <queue>
#include <functional>
#include <memory>
#include <vector>

#include "system.h"
#include "entity.h"
#include "position_component.h"

// Include ActionSpeed definition
#include "../turn_manager.h"

namespace ecs {

/**
 * @enum InputAction
 * @brief Types of input actions
 */
enum class InputAction {
    MOVE,
    PICKUP,
    DROP,
    USE_ITEM,
    OPEN_INVENTORY,
    WAIT,
    QUIT,
    NONE
};

/**
 * @struct InputCommand
 * @brief Encapsulates an input command
 */
struct InputCommand {
    InputAction action = InputAction::NONE;
    int dx = 0;              ///< X direction for movement
    int dy = 0;              ///< Y direction for movement
    EntityID target_id = 0;  ///< Target entity (for use item, etc.)
    int item_slot = -1;      ///< Item slot for inventory actions
};

/**
 * @class InputComponent
 * @brief Component for entities that can receive input
 */
class InputComponent : public Component<InputComponent> {
public:
    bool is_player = false;                    ///< Is this the player entity?
    std::queue<InputCommand> command_queue;    ///< Queue of pending commands
    bool waiting_for_input = true;             ///< Is entity waiting for input?
    ActionSpeed last_action_speed = ActionSpeed::NORMAL; ///< Speed of last action

    InputComponent(bool player = false) : is_player(player) {}

    std::string getTypeName() const override { return "InputComponent"; }
    ComponentType getType() const override { return ComponentType::INPUT; }

    /**
     * @brief Add command to queue
     * @param cmd Command to queue
     */
    void queueCommand(const InputCommand& cmd) {
        command_queue.push(cmd);
        waiting_for_input = false;
    }

    /**
     * @brief Get next command
     * @return Next command or NONE if empty
     */
    InputCommand getNextCommand() {
        if (command_queue.empty()) {
            waiting_for_input = true;
            return InputCommand{};
        }

        InputCommand cmd = command_queue.front();
        command_queue.pop();
        return cmd;
    }

    /**
     * @brief Check if has pending commands
     * @return true if commands are queued
     */
    bool hasCommands() const {
        return !command_queue.empty();
    }
};

// Forward declarations
class MovementSystem;
class CombatSystem;
class InventorySystem;

/**
 * @class InputSystem
 * @brief Processes input commands for controllable entities
 */
class InputSystem : public System<InputSystem> {
public:
    /**
     * @brief Construct input system
     * @param movement Movement system for move commands
     * @param combat Combat system for attack commands
     * @param inventory Inventory system for item commands
     */
    InputSystem(MovementSystem* movement,
                CombatSystem* combat,
                InventorySystem* inventory);

    ~InputSystem() = default;

    /**
     * @brief Update input processing
     * @param entities All entities
     * @param delta_time Time since last update
     */
    void update(const std::vector<std::unique_ptr<Entity>>& entities, double delta_time) override;

    /**
     * @brief Get system priority (runs early)
     * @return Priority value
     */
    int getPriority() const override { return 5; }

    /**
     * @brief Check if system should process entity
     * @param entity Entity to check
     * @return true if entity has InputComponent
     */
    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<InputComponent>();
    }

    /**
     * @brief Queue input for player entity
     * @param player_id Player entity ID
     * @param command Command to queue
     */
    void queuePlayerInput(EntityID player_id, const InputCommand& command);

    /**
     * @brief Convert key input to command
     * @param key Key code
     * @param shift Shift key pressed
     * @param ctrl Control key pressed
     * @return Input command
     */
    static InputCommand keyToCommand(int key, bool shift = false, bool ctrl = false);

    /**
     * @brief Check if player needs input
     * @param player_id Player entity ID
     * @return true if waiting for input
     */
    bool isWaitingForInput(EntityID player_id) const;

    /**
     * @brief Set input callback for async input
     * @param callback Callback function
     */
    void setInputCallback(std::function<InputCommand()> callback) {
        input_callback = callback;
    }

private:
    MovementSystem* movement_system;    ///< Movement system
    CombatSystem* combat_system;        ///< Combat system
    InventorySystem* inventory_system;  ///< Inventory system

    std::function<InputCommand()> input_callback; ///< Optional input callback

    /**
     * @brief Process a single command
     * @param entity Entity executing command
     * @param command Command to process
     * @param entities All entities (for targeting)
     * @return Action speed
     */
    ActionSpeed processCommand(Entity* entity,
                               const InputCommand& command,
                               const std::vector<std::unique_ptr<Entity>>& entities);

    /**
     * @brief Process movement command
     * @param entity Moving entity
     * @param dx X direction
     * @param dy Y direction
     * @param entities All entities
     * @return Action speed
     */
    ActionSpeed processMove(Entity* entity, int dx, int dy,
                           const std::vector<std::unique_ptr<Entity>>& entities);

    /**
     * @brief Process pickup command
     * @param entity Entity picking up
     * @param entities All entities
     * @return Action speed
     */
    ActionSpeed processPickup(Entity* entity,
                             const std::vector<std::unique_ptr<Entity>>& entities);

    /**
     * @brief Process drop command
     * @param entity Entity dropping item
     * @param item_slot Item slot to drop
     * @return Action speed
     */
    ActionSpeed processDrop(Entity* entity, int item_slot);

    /**
     * @brief Process use item command
     * @param entity Entity using item
     * @param item_slot Item slot to use
     * @param target_id Target entity ID
     * @return Action speed
     */
    ActionSpeed processUseItem(Entity* entity, int item_slot, EntityID target_id);

    /**
     * @brief Find entity by ID
     * @param entities All entities
     * @param id Entity ID
     * @return Entity pointer or nullptr
     */
    Entity* findEntity(const std::vector<std::unique_ptr<Entity>>& entities,
                      EntityID id) const;
};

} // namespace ecs