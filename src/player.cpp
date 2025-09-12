#include "player.h"
#include "map.h"
#include "entity_manager.h"
#include "color_scheme.h"

Player::Player(int x, int y) 
    : Entity(x, y, '@', ftxui::Color::White, "Player"),
      hp(BASE_HP),
      max_hp(BASE_HP),
      attack(BASE_ATTACK),
      defense(BASE_DEFENSE),
      level(1),
      experience(0),
      gold(0) {
    
    // Set entity flags
    is_player = true;
    blocks_movement = true;
    blocks_sight = false;
}

bool Player::tryMove(Map& map, EntityManager* entity_manager, int dx, int dy) {
    int new_x = x + dx;
    int new_y = y + dy;
    
    // Check if we can move to the new position
    if (!canMoveTo(map, new_x, new_y)) {
        return false;
    }
    
    // Check for blocking entities (future: combat)
    if (entity_manager) {
        auto blocking = entity_manager->getBlockingEntityAt(new_x, new_y);
        if (blocking && blocking.get() != this) {
            // In the future, this would trigger combat
            // For now, just block movement
            return false;
        }
    }
    
    // Perform the move
    move(dx, dy);
    return true;
}

void Player::takeDamage(int amount) {
    // Apply defense
    int actual_damage = std::max(1, amount - defense);
    hp -= actual_damage;
    
    if (hp <= 0) {
        hp = 0;
        onDeath();
    }
}

void Player::heal(int amount) {
    hp = std::min(max_hp, hp + amount);
}

void Player::gainExperience(int amount) {
    experience += amount;
    
    // Check for level up
    int new_level = calculateLevel();
    if (new_level > level) {
        levelUp();
    }
}

void Player::levelUp() {
    level++;
    
    // Increase stats
    int old_max_hp = max_hp;
    max_hp = BASE_HP + (level - 1) * HP_PER_LEVEL;
    
    // Heal the HP gained from leveling
    hp += (max_hp - old_max_hp);
    
    // Increase attack every 2 levels
    if (level % 2 == 0) {
        attack++;
    }
    
    // Increase defense every 3 levels
    if (level % 3 == 0) {
        defense++;
    }
}

bool Player::canPickUp() const {
    return inventory.size() < static_cast<size_t>(inventory_capacity);
}

void Player::onDeath() {
    // Handle player death
    // For now, just set a flag - game will handle the rest
    // In the future: death screen, save deletion for permadeath, etc.
}

void Player::update([[maybe_unused]] double delta_time) {
    // Update player-specific things
    // For now, nothing to update each frame
    // Future: regeneration, status effects, etc.
}

int Player::calculateLevel() const {
    // Simple level calculation
    // Level 1: 0 exp
    // Level 2: 100 exp
    // Level 3: 300 exp
    // Level 4: 600 exp
    // etc. (triangular number progression)
    
    int lvl = 1;
    int required = 0;
    
    while (experience >= required) {
        lvl++;
        required += lvl * 100;
    }
    
    return lvl - 1;
}