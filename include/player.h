#pragma once

#include "entity.h"
#include <vector>
#include <memory>

class Map;
class EntityManager;

class Player : public Entity {
public:
    // Constructor
    Player(int x, int y);
    
    // Stats
    int hp;
    int max_hp;
    int attack;
    int defense;
    int level;
    int experience;
    int gold;
    
    // Movement
    bool tryMove(Map& map, EntityManager* entity_manager, int dx, int dy);
    
    // Combat
    void takeDamage(int amount);
    void heal(int amount);
    bool isDead() const { return hp <= 0; }
    
    // Experience
    void gainExperience(int amount);
    void levelUp();
    
    // Inventory (future)
    std::vector<std::shared_ptr<Entity>> inventory;
    int inventory_capacity = 20;
    
    // Actions
    bool canPickUp() const;
    bool canAttack() const { return attack > 0; }
    
    // Override base class
    virtual void onDeath() override;
    virtual void update(double delta_time) override;
    
private:
    // Calculate level from experience
    int calculateLevel() const;
    
    // Stats per level
    static constexpr int BASE_HP = 10;
    static constexpr int HP_PER_LEVEL = 5;
    static constexpr int BASE_ATTACK = 1;
    static constexpr int BASE_DEFENSE = 0;
};