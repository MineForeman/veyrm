#pragma once

#include "entity.h"
#include <string>
#include <ftxui/screen/color.hpp>

class Monster : public Entity {
public:
    // Combat stats
    int attack;
    int defense;
    int speed;
    int xp_value;
    
    // Monster metadata
    std::string species;
    std::string name;
    std::string description;
    char threat_level;  // 'a' to 'z' indicating difficulty
    
    // Behavior flags
    bool aggressive = true;
    bool can_open_doors = false;
    bool can_see_invisible = false;
    
    // Constructor
    Monster(int x, int y, const std::string& species);
    
    // Set monster stats from template
    void setStats(int hp, int maxHp, int atk, int def, int spd, int xp);
    void setMetadata(const std::string& name, const std::string& desc, 
                     const std::string& glyph, ftxui::Color color, char threat);
    void setFlags(bool aggro, bool doors, bool seeInvis);
    
    // Override Entity methods
    EntityType getType() const override { return EntityType::MONSTER; }
    bool isBlocking() const override { return true; }
    bool canAct() const override { return hp > 0; }
    void update(double deltaTime) override;
    
    // Monster-specific methods
    int calculateDamage() const;
    void takeDamage(int amount);
    bool isDead() const { return hp <= 0; }
    int getAttackRoll() const;
    int getDefenseValue() const;
    
    // AI state (for future use)
    enum class AIState {
        IDLE,
        WANDERING,
        CHASING,
        FLEEING,
        ATTACKING
    };
    
    AIState ai_state = AIState::IDLE;
    Point last_known_player_pos = Point(-1, -1);
};