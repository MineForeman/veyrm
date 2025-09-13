#include "monster.h"
#include <random>

Monster::Monster(int x, int y, const std::string& species)
    : Entity(x, y, "?", ftxui::Color::Red, species), 
      attack(1),
      defense(0),
      speed(100),
      xp_value(1),
      species(species),
      name(species),
      threat_level('a') {
    
    // Default monster appearance
    glyph = "?";
    color = ftxui::Color::Red;
    is_blocking = true;
    blocks_movement = true;
    
    // Set monster flag
    is_monster = true;
}

void Monster::setStats(int hp_val, int maxHp, int atk, int def, int spd, int xp) {
    this->hp = hp_val;
    this->max_hp = maxHp;
    this->attack = atk;
    this->defense = def;
    this->speed = spd;
    this->xp_value = xp;
}

void Monster::setMetadata(const std::string& name_val, const std::string& desc, 
                         const std::string& glyph_val, ftxui::Color color_val, char threat) {
    this->name = name_val;
    this->description = desc;
    this->glyph = glyph_val;
    this->color = color_val;
    this->threat_level = threat;
}

void Monster::setFlags(bool aggro, bool doors, bool seeInvis) {
    this->aggressive = aggro;
    this->can_open_doors = doors;
    this->can_see_invisible = seeInvis;
}

void Monster::update(double deltaTime) {
    // Basic update - more complex AI will be added in Phase 8.3
    // For now, just call base class update
    Entity::update(deltaTime);
}

int Monster::calculateDamage() const {
    // Simple damage calculation - can be made more complex later
    // Base damage is attack stat with some randomness
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> damage_roll(1, attack);
    return damage_roll(rng);
}

void Monster::takeDamage(int amount) {
    // Apply defense
    int actual_damage = std::max(1, amount - defense);
    hp = std::max(0, hp - actual_damage);
}

int Monster::getAttackRoll() const {
    // Roll for attack - used for hit determination
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> roll(1, 20);
    return roll(rng) + attack;
}

int Monster::getDefenseValue() const {
    // Base defense value for being hit
    return 10 + defense;
}