#include "entity.h"
#include "map.h"
// monster_ai.h removed - using ECS AISystem
#include <cmath>

Entity::Entity(int x, int y, const std::string& glyph, ftxui::Color color, const std::string& name)
    : x(x), y(y), prev_x(x), prev_y(y), glyph(glyph), color(color), name(name) {
}

void Entity::move(int dx, int dy) {
    savePreviousPosition();
    x += dx;
    y += dy;
}

void Entity::moveTo(int new_x, int new_y) {
    savePreviousPosition();
    x = new_x;
    y = new_y;
}

void Entity::update([[maybe_unused]] double delta_time) {
    // Base entities don't update by default
    // Override in derived classes for specific behavior
}

double Entity::distanceTo(const Entity& other) const {
    return distanceTo(other.x, other.y);
}

double Entity::distanceTo(int target_x, int target_y) const {
    int dx = target_x - x;
    int dy = target_y - y;
    return std::sqrt(dx * dx + dy * dy);
}

bool Entity::canMoveTo(const Map& map, int new_x, int new_y) const {
    // Check map bounds
    if (!map.inBounds(new_x, new_y)) {
        return false;
    }
    
    // Check if tile is walkable
    if (!map.isWalkable(new_x, new_y)) {
        return false;
    }
    
    return true;
}

void Entity::savePreviousPosition() {
    prev_x = x;
    prev_y = y;
}

// Combat interface default implementations
int Entity::getAttackBonus() const {
    return 0; // Base entities have no attack bonus
}

int Entity::getDefenseBonus() const {
    return 0; // Base entities have no defense bonus
}

int Entity::getBaseDamage() const {
    return 1; // Base entities do minimal damage
}

std::string Entity::getCombatName() const {
    return name; // Default to entity name
}

// Type-safe AI data accessors
// MonsterAI methods removed - using ECS AISystem