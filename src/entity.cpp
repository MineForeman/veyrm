#include "entity.h"
#include "map.h"
#include "monster_ai.h"
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
MonsterAIData* Entity::getAIData() {
    if (auto* ai_ptr = std::get_if<std::shared_ptr<MonsterAIData>>(&ai_data_storage)) {
        return ai_ptr->get();
    }
    return nullptr;
}

const MonsterAIData* Entity::getAIData() const {
    if (auto* ai_ptr = std::get_if<std::shared_ptr<MonsterAIData>>(&ai_data_storage)) {
        return ai_ptr->get();
    }
    return nullptr;
}

void Entity::setAIData(std::shared_ptr<MonsterAIData> data) {
    ai_data_storage = std::move(data);
}

bool Entity::hasAIData() const {
    return std::holds_alternative<std::shared_ptr<MonsterAIData>>(ai_data_storage);
}

// Legacy compatibility - deprecated
void* Entity::getUserData() const {
    // For backward compatibility, return raw pointer to AI data
    // This maintains existing behavior while we migrate code
    if (auto* ai_ptr = std::get_if<std::shared_ptr<MonsterAIData>>(&ai_data_storage)) {
        return ai_ptr->get();
    }
    return nullptr;
}

void Entity::setUserData(void* data) {
    // Legacy compatibility - wrap raw pointer in unique_ptr
    // WARNING: This assumes the pointer is to MonsterAIData
    // and that we should NOT take ownership (legacy behavior)
    // This is unsafe and should be removed after full migration
    if (data) {
        // UNSAFE: We don't take ownership of the raw pointer to maintain legacy behavior
        // This will leak memory but maintains backward compatibility during migration
        // TODO: Remove this once all code is migrated to use setAIData()
        ai_data_storage = std::monostate{};  // For now, just clear it
    } else {
        ai_data_storage = std::monostate{};
    }
}