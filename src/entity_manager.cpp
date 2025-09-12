#include "entity_manager.h"
#include <algorithm>

EntityManager::EntityManager() {
}

EntityManager::~EntityManager() {
    clear();
}

std::shared_ptr<Entity> EntityManager::createEntity(EntityType type, int x, int y) {
    std::shared_ptr<Entity> entity;
    
    switch (type) {
        case EntityType::PLAYER:
            entity = createPlayer(x, y);
            break;
            
        case EntityType::MONSTER:
            // Placeholder for monster creation
            entity = std::make_shared<Entity>(x, y, 'M', ftxui::Color::Red, "Monster");
            entity->is_monster = true;
            entity->blocks_movement = true;
            break;
            
        case EntityType::ITEM:
            // Placeholder for item creation
            entity = std::make_shared<Entity>(x, y, '!', ftxui::Color::Yellow, "Item");
            entity->is_item = true;
            entity->blocks_movement = false;
            break;
    }
    
    if (entity && entity->is_player == false) {
        addEntity(entity);
    }
    
    return entity;
}

std::shared_ptr<Player> EntityManager::createPlayer(int x, int y) {
    // Only one player allowed
    if (player) {
        player->moveTo(x, y);
        return player;
    }
    
    player = std::make_shared<Player>(x, y);
    addEntity(player);
    return player;
}

void EntityManager::destroyEntity(std::shared_ptr<Entity> entity) {
    if (!entity) return;
    
    // Don't destroy the player this way
    if (entity == player) {
        return;
    }
    
    // Remove from entities list
    entities.erase(
        std::remove(entities.begin(), entities.end(), entity),
        entities.end()
    );
}

void EntityManager::clear() {
    entities.clear();
    player.reset();
}

std::vector<std::shared_ptr<Entity>> EntityManager::getEntitiesAt(int x, int y) const {
    std::vector<std::shared_ptr<Entity>> result;
    
    for (const auto& entity : entities) {
        if (entity && entity->isAt(x, y)) {
            result.push_back(entity);
        }
    }
    
    return result;
}

std::shared_ptr<Entity> EntityManager::getBlockingEntityAt(int x, int y) const {
    for (const auto& entity : entities) {
        if (entity && entity->isAt(x, y) && entity->blocks_movement) {
            return entity;
        }
    }
    return nullptr;
}

std::shared_ptr<Entity> EntityManager::getItemAt(int x, int y) const {
    for (const auto& entity : entities) {
        if (entity && entity->isAt(x, y) && entity->is_item) {
            return entity;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getMonsters() const {
    std::vector<std::shared_ptr<Entity>> result;
    
    for (const auto& entity : entities) {
        if (entity && entity->is_monster) {
            result.push_back(entity);
        }
    }
    
    return result;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getItems() const {
    std::vector<std::shared_ptr<Entity>> result;
    
    for (const auto& entity : entities) {
        if (entity && entity->is_item) {
            result.push_back(entity);
        }
    }
    
    return result;
}

std::vector<std::shared_ptr<Entity>> EntityManager::getAllEntities() const {
    return entities;
}

void EntityManager::updateAll(double delta_time) {
    // Update all entities
    for (auto& entity : entities) {
        if (entity) {
            entity->update(delta_time);
        }
    }
}

size_t EntityManager::getMonsterCount() const {
    return std::count_if(entities.begin(), entities.end(),
        [](const auto& e) { return e && e->is_monster; });
}

size_t EntityManager::getItemCount() const {
    return std::count_if(entities.begin(), entities.end(),
        [](const auto& e) { return e && e->is_item; });
}

void EntityManager::removeDeadEntities() {
    // Remove dead monsters and consumed items
    // Don't remove the player even if dead (game should handle that)
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [this](const auto& entity) {
                if (!entity) return true;
                if (entity == player) return false;
                
                // In the future, check entity->isDead() for monsters
                return false;
            }),
        entities.end()
    );
}

bool EntityManager::isPositionBlocked(int x, int y) const {
    return getBlockingEntityAt(x, y) != nullptr;
}

void EntityManager::addEntity(std::shared_ptr<Entity> entity) {
    if (entity) {
        entities.push_back(entity);
    }
}