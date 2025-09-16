/**
 * @file game_world.cpp
 * @brief Implementation of ECS-based game world
 */

// All includes at global scope
#include <memory>
#include <vector>
#include <deque>
#include <algorithm>
#include <iostream>
#include <string>
#include <exception>
#include <variant>

#include "ecs/game_world.h"
#include "ecs/entity_factory.h"
#include "ecs/movement_system.h"
#include "ecs/render_system.h"
#include "ecs/combat_system.h"
#include "ecs/ai_system.h"
#include "ecs/inventory_system.h"
#include "ecs/equipment_system.h"
#include "ecs/event.h"
#include "ecs/experience_component.h"
#include "ecs/loot_component.h"
#include "turn_manager.h"
#include "message_log_adapter.h"

// Forward declare Map to avoid include issues
class Map;

// Only now open the namespace
namespace ecs {

GameWorld::GameWorld(MessageLog* log, ::Map* map)
    : message_log(log),
      game_map(map) {
    // Create ILogger adapter for MessageLog
    if (message_log) {
        logger = std::make_unique<MessageLogAdapter>(message_log);
    }
}

GameWorld::~GameWorld() = default;

void GameWorld::initialize(bool migrate_existing) {
    // Bridge objects removed - no longer needed in full ECS mode

    // Initialize all ECS systems
    initializeSystems();

    // Migrate existing entities if requested
    if (migrate_existing) {
        migrateExistingEntities();
    }
}

void GameWorld::initializeSystems() {
    // Register core systems with priorities
    auto& movement = world.registerSystem<MovementSystem>(game_map);
    world.registerSystem<RenderSystem>(game_map);

    // Register native ECS combat system
    native_combat_system = &world.registerSystem<CombatSystem>(logger.get());

    // Register AI system with dependencies
    native_ai_system = &world.registerSystem<AISystem>(game_map, &movement,
                                                        native_combat_system,
                                                        logger.get());

    // Register inventory system
    world.registerSystem<InventorySystem>(game_map, logger.get());

    // Register equipment system with world access
    world.registerSystem<EquipmentSystem>(logger.get(), &world);

    // Set player ID for AI targeting
    if (native_ai_system && player_id != 0) {
        native_ai_system->setPlayerId(player_id);
    }

    // Subscribe to drop events to restore item position when dropped
    EventSystem::getInstance().subscribe(EventType::DROP,
        [this](const BaseEvent& e) {
            if (logger) {
                logger->logSystem("Drop event received: item_id=" + std::to_string(e.target_id) +
                                " at (" + std::to_string(e.value1) + "," + std::to_string(e.value2) + ")");
            }

            Entity* item = getEntity(e.target_id);
            if (item) {
                // Restore position component to dropped item
                item->addComponent<PositionComponent>(e.value1, e.value2);

                // Also ensure the item has its RenderableComponent enabled
                auto* render = item->getComponent<RenderableComponent>();
                if (render) {
                    render->is_visible = true;
                }

                if (logger) {
                    logger->logSystem("Item position restored successfully");
                }
            } else {
                if (logger) {
                    logger->logSystem("ERROR: Could not find item entity " + std::to_string(e.target_id));
                }
            }
        });
}

void GameWorld::migrateExistingEntities() {
    return;  // Migration not needed

    /* Migration code removed:
    for (const auto& legacy_entity : all_entities) {
        if (!legacy_entity) continue;

        // Create ECS entity based on type
        std::unique_ptr<Entity> ecs_entity;

        // Check entity type and convert appropriately
        if (legacy_entity->is_player) {
            // Legacy player entity - create basic ECS entity
            ecs_entity = std::make_unique<Entity>();

            // Core components
            ecs_entity->addComponent<PositionComponent>(legacy_entity->x, legacy_entity->y);
            ecs_entity->addComponent<RenderableComponent>(legacy_entity->glyph, legacy_entity->color);
            ecs_entity->addComponent<HealthComponent>(100, 100);  // Default player health

            // Combat with default player stats
            auto& combat = ecs_entity->addComponent<CombatComponent>(6, 3, 2);
            combat.combat_name = "Player";
            combat.setDamageRange(1, 6);  // Player uses d6

            // Add new components for full ECS support
            ecs_entity->addComponent<InventoryComponent>();
            ecs_entity->addComponent<StatsComponent>();
            ecs_entity->addComponent<ExperienceComponent>();
            ecs_entity->addComponent<PlayerComponent>();

            // Tag as player
            ecs_entity->addTag("player");

            // Track player ID
            if (ecs_entity) {
                player_id = ecs_entity->getID();

                // Update AI system with player ID
                if (native_ai_system) {
                    native_ai_system->setPlayerId(player_id);
                }
            }
        } else if (legacy_entity->is_monster) {
            // Legacy monster entity - create basic ECS entity
            ecs_entity = std::make_unique<Entity>();

            // Core components
            ecs_entity->addComponent<PositionComponent>(legacy_entity->x, legacy_entity->y);
            ecs_entity->addComponent<RenderableComponent>(legacy_entity->glyph, legacy_entity->color);
            ecs_entity->addComponent<HealthComponent>(legacy_entity->max_hp, legacy_entity->hp);

            // Basic combat stats
            auto& combat = ecs_entity->addComponent<CombatComponent>(
                5,    // attack
                2,    // defense
                5     // damage
            );
            combat.combat_name = legacy_entity->name;
            combat.setDamageRange(3, 7);

            // AI component
            auto& ai = ecs_entity->addComponent<AIComponent>();
            ai.behavior = AIBehavior::AGGRESSIVE;

            // Loot component
            auto& loot = ecs_entity->addComponent<LootComponent>();
            loot.guaranteed_gold = 5;
            loot.random_gold_max = 20;

            // Tag as monster
            ecs_entity->addTag("monster");
        } else if (auto item = std::dynamic_pointer_cast<Item>(legacy_entity)) {
            // Create item entity directly
            ecs_entity = std::make_unique<Entity>();

            // Items typically just need position and rendering
            ecs_entity->addComponent<PositionComponent>(item->x, item->y);
            // Item has symbol and color string (need to convert)
            std::string glyph(1, item->symbol);  // Convert char to string
            ecs_entity->addComponent<RenderableComponent>(glyph, ftxui::Color::White);

            // Add ItemComponent with basic properties
            auto& item_comp = ecs_entity->addComponent<ItemComponent>();
            item_comp.name = item->name;
            item_comp.item_type = ItemType::MISC;  // Default type
            item_comp.value = 10;  // Default value

            // Tag as item
            ecs_entity->addTag("item");
        } else {
            // Generic entity migration
            ecs_entity = std::make_unique<Entity>();
            ecs_entity->addComponent<PositionComponent>(
                legacy_entity->getPosition().x,
                legacy_entity->getPosition().y
            );
            ecs_entity->addComponent<RenderableComponent>(
                legacy_entity->glyph,
                legacy_entity->color
            );
        }

        if (ecs_entity) {
            // Add to world
            world.addEntity(std::move(ecs_entity));
            // Legacy sync removed - no longer needed
        }
    }
    */
}

void GameWorld::update(double delta_time) {
    // Process all queued events from previous frame
    EventSystem::getInstance().update();

    // Update all ECS systems
    world.update(delta_time);

    // Process any events generated by systems
    EventSystem::getInstance().update();

    // Remove dead entities
    removeDeadEntities();

    // Sync changes back to legacy systems
    syncToLegacy();
}

void GameWorld::updateRenderSystem() {
    // Update only the render system to refresh visual display
    auto* render_system = getRenderSystem();
    if (render_system) {
        render_system->update(world.getEntities(), 0.0);
    }
}

EntityID GameWorld::createPlayer(int x, int y) {

    // Create player using factory
    auto player_entity = PlayerFactory().create(x, y);
    EntityID id = player_entity->getID();

    // Add to world
    world.addEntity(std::move(player_entity));

    // Legacy player creation removed - full ECS mode

    // Track player ID
    player_id = id;

    // Update AI system
    if (native_ai_system) {
        native_ai_system->setPlayerId(player_id);
    }

    return id;
}

EntityID GameWorld::createMonster(const std::string& type, int x, int y) {
    // Create monster using factory
    auto monster_entity = MonsterFactoryECS().create(type, x, y);
    EntityID id = monster_entity->getID();

    // Add to world
    world.addEntity(std::move(monster_entity));

    // Legacy monster creation removed - full ECS mode

    return id;
}

EntityID GameWorld::createItem(const std::string& type, int x, int y) {
    // Create item using factory
    auto item_entity = ItemFactoryECS().create(type, x, y);
    EntityID id = item_entity->getID();

    // Add to world
    world.addEntity(std::move(item_entity));

    // Legacy item creation removed

    return id;
}

Entity* GameWorld::getEntity(EntityID id) {
    return world.getEntity(id);
}

bool GameWorld::removeEntity(EntityID id) {
    // Don't remove player
    if (id == player_id) {
        return false;
    }

    // Bridge removal code removed - no longer needed

    return world.removeEntity(id);
}

std::vector<Entity*> GameWorld::getEntitiesAt(int x, int y) {
    std::vector<Entity*> result;

    for (const auto& entity : world.getEntities()) {
        auto* pos = entity->getComponent<PositionComponent>();
        if (pos && pos->isAt(x, y)) {
            result.push_back(entity.get());
        }
    }

    return result;
}

ActionSpeed GameWorld::processPlayerAction(int action, int dx, int dy) {

    // Get player entity
    Entity* player = getEntity(player_id);
    if (!player) {
        return ActionSpeed::NORMAL;
    }

    auto* pos = player->getComponent<PositionComponent>();
    if (!pos) {
        return ActionSpeed::NORMAL;
    }


    ActionSpeed speed = ActionSpeed::NORMAL;

    // Handle different actions
    switch (action) {
        case 0: // Movement
            if (dx != 0 || dy != 0) {
                int new_x = pos->position.x + dx;
                int new_y = pos->position.y + dy;

                // Check for combat (bump to attack)
                auto entities_at_target = getEntitiesAt(new_x, new_y);
                for (Entity* target : entities_at_target) {
                    if (target->hasComponent<CombatComponent>() &&
                        target->getID() != player_id) {
                        // Attack the target
                        if (native_combat_system) {
                            native_combat_system->queueAttack(player_id, target->getID());
                        }
                        return ActionSpeed::NORMAL;
                    }
                }

                // Move if no combat
                auto* movement = world.getSystem<MovementSystem>();
                if (movement) {
                    movement->queueMove(player_id, dx, dy);
                    // Process the queued movement immediately for player
                    movement->update(world.getEntities(), 0.0);
                    speed = ActionSpeed::NORMAL;
                }
            }
            break;

        case 1: // Pickup item
            {
                auto items_here = getEntitiesAt(pos->position.x, pos->position.y);
                auto* inv_system = world.getSystem<InventorySystem>();
                if (inv_system) {
                    for (Entity* item : items_here) {
                        if (item->hasComponent<ItemComponent>()) {
                            if (inv_system->pickupItem(player, item)) {
                                speed = ActionSpeed::FAST;
                                break;
                            }
                        }
                    }
                }
            }
            break;

        case 2: // Use item
            // Would need item selection UI
            speed = ActionSpeed::FAST;
            break;

        case 3: // Drop item
            // Would need item selection UI
            speed = ActionSpeed::FAST;
            break;

        case 4: // Wait
            speed = ActionSpeed::NORMAL;
            break;

        default:
            speed = ActionSpeed::NORMAL;
            break;
    }

    // Process any queued combat actions from player action
    if (native_combat_system) {
        native_combat_system->update(world.getEntities(), 0.0);
        // Remove dead entities immediately after combat
        removeDeadEntities();
    }

    return speed;
}

void GameWorld::processMonsterAI() {
    // Update AI system for one turn
    if (native_ai_system && player_id != 0) {
        native_ai_system->setPlayerId(player_id);
        // Run the AI system update manually for turn-based behavior
        native_ai_system->update(world.getEntities(), 0.0);

        // Process any queued movements from AI decisions
        auto* movement_system = getMovementSystem();
        if (movement_system) {
            movement_system->update(world.getEntities(), 0.0);
        }

        // Process any queued combat actions
        if (native_combat_system) {
            native_combat_system->update(world.getEntities(), 0.0);
        }

        // Remove dead entities immediately after combat to prevent ghost actions
        removeDeadEntities();
    }
}

void GameWorld::updateFOV(const std::vector<std::vector<bool>>& fov) {
    // Update visibility for all entities based on FOV
    for (const auto& entity : world.getEntities()) {
        auto* pos = entity->getComponent<PositionComponent>();
        auto* renderable = entity->getComponent<RenderableComponent>();

        if (pos && renderable) {
            if (pos->position.x >= 0 && pos->position.x < (int)fov.size() &&
                pos->position.y >= 0 && pos->position.y < (int)fov[0].size()) {
                renderable->is_visible = fov[pos->position.x][pos->position.y];
            }
        }
    }

    // Also update the render system's FOV for proper visibility checks
    auto* render_system = getRenderSystem();
    if (render_system) {
        render_system->setFOV(fov);
    }
}

void GameWorld::syncToLegacy() {
    // Legacy sync removed - no longer needed in full ECS mode
}

void GameWorld::syncFromLegacy() {
    // Legacy sync removed - no longer needed in full ECS mode
}

MovementSystem* GameWorld::getMovementSystem() {
    return world.getSystem<MovementSystem>();
}

RenderSystem* GameWorld::getRenderSystem() {
    return world.getSystem<RenderSystem>();
}

InventorySystem* GameWorld::getInventorySystem() {
    return world.getSystem<InventorySystem>();
}

bool GameWorld::isPositionBlocked(int x, int y) const {
    // Check map
    if (game_map && !game_map->isWalkable(x, y)) {
        return true;
    }

    // Check entities
    for (const auto& entity : world.getEntities()) {
        auto* pos = entity->getComponent<PositionComponent>();
        auto* combat = entity->getComponent<CombatComponent>();

        // Blocking entities have position and combat components
        if (pos && combat && pos->isAt(x, y)) {
            return true;
        }
    }

    return false;
}

void GameWorld::removeDeadEntities() {
    std::vector<EntityID> to_remove;

    for (const auto& entity : world.getEntities()) {
        auto* health = entity->getComponent<HealthComponent>();
        if (health && health->hp <= 0) {
            // Special handling for player death
            if (entity->getID() == player_id) {
                // Player died - log it
                if (logger) {
                    logger->logCombat("You die!");
                }
                // Set a flag that game manager can check
                player_died = true;
                continue;  // Don't remove the player entity
            }
            to_remove.push_back(entity->getID());

            // Drop items on death
            auto* inventory = entity->getComponent<InventoryComponent>();
            auto* pos = entity->getComponent<PositionComponent>();
            if (inventory && pos) {
                for (EntityID item_id : inventory->items) {
                    Entity* item = getEntity(item_id);
                    if (item) {
                        // Add position back to item
                        item->addComponent<PositionComponent>(
                            pos->position.x, pos->position.y
                        );
                    }
                }
            }

            // Log death message
            if (logger) {
                auto* renderable = entity->getComponent<RenderableComponent>();
                std::string name = renderable ? renderable->name : "Something";
                logger->logCombat(name + " dies!");
            }
        }
    }

    // Remove dead entities
    for (EntityID id : to_remove) {
        removeEntity(id);
    }
}

} // namespace ecs