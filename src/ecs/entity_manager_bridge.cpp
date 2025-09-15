/**
 * @file entity_manager_bridge.cpp
 * @brief Implementation of EntityManager ECS bridge
 */

#include "../../include/ecs/entity_manager_bridge.h"
#include "../../include/player.h"
#include "../../include/monster.h"
#include "../../include/item.h"

namespace ecs {

EntityManagerBridge::EntityManagerBridge(EntityManager* manager)
    : legacy_manager(manager) {
}

void EntityManagerBridge::syncEntity(std::shared_ptr<::Entity> legacy_entity,
                                     std::shared_ptr<ecs::Entity> ecs_entity) {
    if (!legacy_entity || !ecs_entity) return;

    legacy_to_ecs[legacy_entity] = ecs_entity;
    ecs_to_legacy[ecs_entity] = legacy_entity;

    // Sync data from ECS to legacy
    EntityAdapter::syncToLegacy(*ecs_entity, *legacy_entity);
}

std::vector<std::shared_ptr<ecs::Entity>> EntityManagerBridge::getEntitiesAtPosition(
    int x, int y) const {

    std::vector<std::shared_ptr<ecs::Entity>> result;

    for (const auto& [legacy, ecs] : legacy_to_ecs) {
        if (!ecs) continue;

        auto* pos = ecs->getComponent<PositionComponent>();
        if (pos && pos->isAt(x, y)) {
            result.push_back(ecs);
        }
    }

    return result;
}

std::vector<std::shared_ptr<ecs::Entity>> EntityManagerBridge::getCombatEntities() const {
    std::vector<std::shared_ptr<ecs::Entity>> result;

    for (const auto& [legacy, ecs] : legacy_to_ecs) {
        if (!ecs) continue;

        if (ecs->hasComponent<CombatComponent>()) {
            result.push_back(ecs);
        }
    }

    return result;
}

std::vector<std::shared_ptr<ecs::Entity>> EntityManagerBridge::getRenderableEntities() const {
    std::vector<std::shared_ptr<ecs::Entity>> result;

    for (const auto& [legacy, ecs] : legacy_to_ecs) {
        if (!ecs) continue;

        if (ecs->hasComponent<RenderableComponent>()) {
            result.push_back(ecs);
        }
    }

    return result;
}

std::vector<std::shared_ptr<ecs::Entity>> EntityManagerBridge::getVisibleRenderableEntities(
    const std::vector<std::vector<bool>>& fov) const {

    std::vector<std::shared_ptr<ecs::Entity>> result;

    for (const auto& [legacy, ecs] : legacy_to_ecs) {
        if (!ecs) continue;

        auto* pos = ecs->getComponent<PositionComponent>();
        auto* render = ecs->getComponent<RenderableComponent>();

        if (!pos || !render) continue;

        // Check if position is in FOV
        int x = pos->position.x;
        int y = pos->position.y;

        if (y >= 0 && y < static_cast<int>(fov.size()) &&
            x >= 0 && x < static_cast<int>(fov[y].size()) &&
            fov[y][x]) {

            result.push_back(ecs);
        } else if (render->always_visible) {
            // Always visible entities bypass FOV
            result.push_back(ecs);
        }
    }

    return result;
}

bool EntityManagerBridge::isPositionBlockedByCombatEntity(int x, int y) const {
    for (const auto& [legacy, ecs] : legacy_to_ecs) {
        if (!ecs) continue;

        auto* pos = ecs->getComponent<PositionComponent>();
        auto* combat = ecs->getComponent<CombatComponent>();

        if (pos && combat && pos->isAt(x, y)) {
            return true;
        }
    }

    return false;
}

void EntityManagerBridge::updatePositionsFromComponents() {
    for (const auto& [legacy, ecs] : legacy_to_ecs) {
        if (!legacy || !ecs) continue;

        auto* pos = ecs->getComponent<PositionComponent>();
        if (pos) {
            legacy->moveTo(pos->position.x, pos->position.y);
        }
    }
}

void EntityManagerBridge::updateHealthFromComponents() {
    for (const auto& [legacy, ecs] : legacy_to_ecs) {
        if (!legacy || !ecs) continue;

        auto* health = ecs->getComponent<HealthComponent>();
        if (health) {
            legacy->hp = health->getHealth();
            legacy->max_hp = health->getMaxHealth();
        }
    }
}

std::shared_ptr<ecs::Entity> EntityManagerBridge::getECSEntity(
    std::shared_ptr<::Entity> legacy_entity) const {

    auto it = legacy_to_ecs.find(legacy_entity);
    return (it != legacy_to_ecs.end()) ? it->second : nullptr;
}

std::shared_ptr<::Entity> EntityManagerBridge::getLegacyEntity(
    std::shared_ptr<ecs::Entity> ecs_entity) const {

    auto it = ecs_to_legacy.find(ecs_entity);
    return (it != ecs_to_legacy.end()) ? it->second : nullptr;
}

void EntityManagerBridge::createComponentsForLegacyEntities() {
    auto all_entities = legacy_manager->getAllEntities();

    for (const auto& legacy : all_entities) {
        if (!legacy) continue;

        // Skip if already has ECS entity
        if (legacy_to_ecs.find(legacy) != legacy_to_ecs.end()) continue;

        // Create ECS entity based on type
        std::shared_ptr<ecs::Entity> ecs_entity;

        if (auto player = std::dynamic_pointer_cast<Player>(legacy)) {
            ecs_entity = EntityAdapter::fromPlayer(*player);
        } else if (auto monster = std::dynamic_pointer_cast<Monster>(legacy)) {
            ecs_entity = EntityAdapter::fromMonster(*monster);
        } else if (auto item = std::dynamic_pointer_cast<Item>(legacy)) {
            ecs_entity = EntityAdapter::fromItem(*item);
        } else {
            // Generic entity - create with basic components
            ecs_entity = std::make_unique<ecs::Entity>();
            ecs_entity->addComponent<PositionComponent>(legacy->getPosition().x, legacy->getPosition().y);
            ecs_entity->addComponent<RenderableComponent>(legacy->glyph, legacy->color);
        }

        if (ecs_entity) {
            syncEntity(legacy, ecs_entity);
        }
    }
}

void EntityManagerBridge::removeDeadEntitiesFromComponents() {
    std::vector<std::shared_ptr<::Entity>> to_remove;

    for (const auto& [legacy, ecs] : legacy_to_ecs) {
        if (!ecs) continue;

        auto* health = ecs->getComponent<HealthComponent>();
        if (health && health->isDead()) {
            // Don't remove the player
            if (legacy != legacy_manager->getPlayer()) {
                to_remove.push_back(legacy);
            }
        }
    }

    // Remove dead entities
    for (const auto& entity : to_remove) {
        auto ecs = legacy_to_ecs[entity];
        legacy_to_ecs.erase(entity);
        ecs_to_legacy.erase(ecs);
        legacy_manager->destroyEntity(entity);
    }
}

void EntityManagerBridge::removeEntity(EntityID id) {
    // Find entity with this ID and remove from mappings
    for (auto it = legacy_to_ecs.begin(); it != legacy_to_ecs.end(); ++it) {
        if (it->second && it->second->getID() == id) {
            auto legacy = it->first;
            auto ecs = it->second;

            legacy_to_ecs.erase(it);
            ecs_to_legacy.erase(ecs);

            if (legacy_manager) {
                legacy_manager->destroyEntity(legacy);
            }
            break;
        }
    }
}

void EntityManagerBridge::syncToLegacy(ecs::Entity* ecs_entity) {
    if (!ecs_entity) return;

    // Find corresponding legacy entity
    for (const auto& [ecs_ptr, legacy_ptr] : ecs_to_legacy) {
        if (ecs_ptr.get() == ecs_entity) {
            // Update legacy entity from ECS components
            EntityAdapter::syncToLegacy(*ecs_entity, *legacy_ptr);
            break;
        }
    }
}

void EntityManagerBridge::syncFromLegacy(std::shared_ptr<::Entity> legacy_entity) {
    if (!legacy_entity) return;

    auto it = legacy_to_ecs.find(legacy_entity);
    if (it != legacy_to_ecs.end() && it->second) {
        // Update ECS entity from legacy entity
        auto* pos = it->second->getComponent<PositionComponent>();
        if (pos) {
            pos->position.x = legacy_entity->x;
            pos->position.y = legacy_entity->y;
        }

        auto* health = it->second->getComponent<HealthComponent>();
        if (health) {
            health->hp = legacy_entity->hp;
            health->max_hp = legacy_entity->max_hp;
        }
    }
}

} // namespace ecs