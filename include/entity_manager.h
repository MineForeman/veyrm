#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include "entity.h"
#include "player.h"

// Forward declarations
class Map;
class MapRenderer;
class Monster;

class EntityManager {
public:
    EntityManager();
    ~EntityManager();
    
    // Entity lifecycle
    std::shared_ptr<Entity> createEntity(EntityType type, int x, int y);
    std::shared_ptr<Player> createPlayer(int x, int y);
    std::shared_ptr<Monster> createMonster(const std::string& species, int x, int y);
    void destroyEntity(std::shared_ptr<Entity> entity);
    void clear();
    
    // Access
    std::shared_ptr<Player> getPlayer() { return player; }
    const std::shared_ptr<Player> getPlayer() const { return player; }
    
    // Query entities at position
    std::vector<std::shared_ptr<Entity>> getEntitiesAt(int x, int y) const;
    std::shared_ptr<Entity> getBlockingEntityAt(int x, int y) const;
    std::shared_ptr<Entity> getItemAt(int x, int y) const;
    std::shared_ptr<Monster> getMonsterAt(int x, int y) const;
    
    // Get all entities of type
    std::vector<std::shared_ptr<Entity>> getMonsters() const;
    std::vector<std::shared_ptr<Entity>> getItems() const;
    std::vector<std::shared_ptr<Entity>> getAllEntities() const;
    
    // Visibility
    std::vector<std::shared_ptr<Entity>> getVisibleEntities() const;
    std::vector<std::shared_ptr<Entity>> getVisibleMonsters() const;
    std::vector<std::shared_ptr<Entity>> getVisibleItems() const;
    void updateEntityVisibility(const std::vector<std::vector<bool>>& fov);
    
    // Updates
    void updateAll(double delta_time);
    
    // Entity count
    size_t getEntityCount() const { return entities.size(); }
    size_t getMonsterCount() const;
    size_t getItemCount() const;
    
    // Utility
    void removeDeadEntities();
    bool isPositionBlocked(int x, int y) const;
    
private:
    std::shared_ptr<Player> player;
    std::vector<std::shared_ptr<Entity>> entities;
    
    // Helper to add entity to list
    void addEntity(std::shared_ptr<Entity> entity);
};