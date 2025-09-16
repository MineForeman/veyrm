/**
 * @file inventory_system.h
 * @brief System for managing entity inventories
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

// All includes at global scope
#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "system.h"
#include "entity.h"
#include "inventory_component.h"
#include "item_component.h"
#include "position_component.h"
#include "event.h"
#include "logger_interface.h"

// Forward declarations
class Map;

// Only now open the namespace
namespace ecs {

/**
 * @class InventorySystem
 * @brief Manages item pickup, drop, and use actions
 */
class InventorySystem : public System<InventorySystem> {
public:
    /**
     * @brief Construct inventory system
     * @param map Game map for item placement
     * @param logger Logger for notifications (can be nullptr)
     */
    InventorySystem(Map* map, ILogger* logger = nullptr);

    ~InventorySystem();

    /**
     * @brief Update inventory system
     * @param entities All entities
     * @param delta_time Time since last update
     */
    void update(const std::vector<std::unique_ptr<Entity>>& entities, double delta_time) override;

    /**
     * @brief Process item pickup
     * @param picker Entity picking up item
     * @param item Item to pick up
     * @return true if successful
     */
    bool pickupItem(Entity* picker, Entity* item);

    /**
     * @brief Process item drop
     * @param dropper Entity dropping item
     * @param item_id ID of item to drop
     * @return true if successful
     */
    bool dropItem(Entity* dropper, EntityID item_id);

    /**
     * @brief Use an item
     * @param user Entity using item
     * @param item_id ID of item to use
     * @param target Target entity (optional)
     * @return true if successful
     */
    bool useItem(Entity* user, EntityID item_id, Entity* target = nullptr);

    /**
     * @brief Get items at position
     * @param entities All entities
     * @param x X coordinate
     * @param y Y coordinate
     * @return Vector of item entities at position
     */
    std::vector<Entity*> getItemsAt(const std::vector<std::unique_ptr<Entity>>& entities,
                                    int x, int y);

    /**
     * @brief Check if entity can pick up item
     * @param entity Entity to check
     * @param item Item to check
     * @return true if pickup is possible
     */
    bool canPickup(Entity* entity, Entity* item);

    /**
     * @brief Get total weight of inventory
     * @param inventory Inventory component
     * @param entities All entities (to look up item weights)
     * @return Total weight
     */
    float getTotalWeight(const InventoryComponent& inventory,
                        const std::vector<std::unique_ptr<Entity>>& entities);

    /**
     * @brief Get system priority
     * @return Priority value
     */
    int getPriority() const override { return 40; }

    /**
     * @brief Check if system should process entity
     * @param entity Entity to check
     * @return true if entity has inventory component
     */
    bool shouldProcess(const Entity& entity) const override {
        return entity.hasComponent<InventoryComponent>();
    }

private:
    [[maybe_unused]] Map* map;          ///< Game map
    ILogger* logger;                    ///< Logger interface

    /**
     * @brief Handle pickup event
     * @param event Event data
     */
    void handlePickupEvent(const BaseEvent& event);

    /**
     * @brief Handle drop event
     * @param event Event data
     */
    void handleDropEvent(const BaseEvent& event);

    /**
     * @brief Handle use item event
     * @param event Event data
     */
    void handleUseItemEvent(const BaseEvent& event);

    /**
     * @brief Apply item effects
     * @param user User entity
     * @param item Item component
     * @param target Target entity (optional)
     */
    void applyItemEffects(Entity* user, const ItemComponent& item, Entity* target);

    /**
     * @brief Find entity by ID
     * @param entities All entities
     * @param id Entity ID
     * @return Entity pointer or nullptr
     */
    Entity* findEntity(const std::vector<std::unique_ptr<Entity>>& entities,
                      EntityID id);

    /**
     * @brief Get entity name for messages
     * @param entity Entity
     * @return Name string
     */
    std::string getEntityName(Entity* entity);
};

} // namespace ecs