/**
 * @file ai_system.cpp
 * @brief Native ECS AI system implementation
 */

#include "ecs/ai_system.h"
#include "ecs/movement_system.h"
#include "ecs/combat_system.h"
#include "ecs/health_component.h"
#include "ecs/renderable_component.h"
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <cmath>

namespace ecs {

AISystem::AISystem(Map* map,
                   MovementSystem* movement_system,
                   CombatSystem* combat_system,
                   MessageLog* message_log)
    : map(map)
    , movement_system(movement_system)
    , combat_system(combat_system)
    , message_log(message_log)
    , rng(std::random_device{}()) {
}

void AISystem::update(const std::vector<std::unique_ptr<Entity>>& entities, double) {
    // Process AI for each entity with an AI component
    for (const auto& entity : entities) {
        if (entity->hasComponent<AIComponent>() && entity->getID() != player_id) {
            // Create temporary shared_ptr for processing
            auto entity_ptr = std::shared_ptr<Entity>(entity.get(), [](Entity*){});
            processEntityAI(entity_ptr, entities);
        }
    }
}

void AISystem::processEntityAI(std::shared_ptr<Entity> entity,
                               const std::vector<std::unique_ptr<Entity>>& entities) {
    auto* ai = entity->getComponent<AIComponent>();
    if (!ai) return;

    // Find player entity
    auto player = findEntity(entities, player_id);
    if (!player) return;

    // Update AI state based on player visibility
    if (canSeeEntity(entity, player)) {
        ai->has_seen_player = true;
        ai->turns_since_player_seen = 0;
        auto* player_pos = player->getComponent<PositionComponent>();
        if (player_pos) {
            ai->last_player_position = Point{player_pos->position.x, player_pos->position.y};
        }
    } else {
        ai->turns_since_player_seen++;
    }

    // Execute behavior based on AI type
    switch (ai->behavior) {
        case AIBehavior::PASSIVE:
            handlePassiveBehavior(entity);
            break;
        case AIBehavior::WANDERING:
            handleWanderingBehavior(entity);
            break;
        case AIBehavior::AGGRESSIVE:
            handleAggressiveBehavior(entity, player);
            break;
        case AIBehavior::DEFENSIVE:
            handleDefensiveBehavior(entity, player);
            break;
        case AIBehavior::PATROL:
            // TODO: Implement patrol behavior
            handleWanderingBehavior(entity);
            break;
    }
}

void AISystem::handlePassiveBehavior(std::shared_ptr<Entity>) {
    // Passive entities don't move or attack
    // They just exist peacefully
}

void AISystem::handleWanderingBehavior(std::shared_ptr<Entity> entity) {
    auto* pos = entity->getComponent<PositionComponent>();
    if (!pos) return;

    // Random movement
    Point current{pos->position.x, pos->position.y};
    Point target = getRandomAdjacentPosition(current);

    // Try to move to random position
    if (map && map->isWalkable(target.x, target.y)) {
        int dx = target.x - pos->position.x;
        int dy = target.y - pos->position.y;
        movement_system->queueMove(entity->getID(), dx, dy);
    }
}

void AISystem::handleAggressiveBehavior(std::shared_ptr<Entity> entity,
                                        std::shared_ptr<Entity> player) {
    if (!player) return;

    auto* ai = entity->getComponent<AIComponent>();
    auto* pos = entity->getComponent<PositionComponent>();
    if (!ai || !pos) return;

    // Check if we can see the player
    if (canSeeEntity(entity, player)) {
        ai->target_id = player->getID();

        // Try to attack if adjacent
        if (tryAttack(entity, player)) {
            return;
        }

        // Otherwise, move towards player
        auto* player_pos = player->getComponent<PositionComponent>();
        if (player_pos) {
            Point target{player_pos->position.x, player_pos->position.y};
            moveTowards(entity, target);
        }
    } else if (ai->has_seen_player && ai->turns_since_player_seen < 5) {
        // Hunt last known position
        if (ai->last_player_position.x >= 0) {
            moveTowards(entity, ai->last_player_position);
        }
    } else {
        // Wander when no target
        handleWanderingBehavior(entity);
    }
}

void AISystem::handleDefensiveBehavior(std::shared_ptr<Entity> entity,
                                       std::shared_ptr<Entity> player) {
    if (!player) return;

    auto* ai = entity->getComponent<AIComponent>();
    auto* health = entity->getComponent<HealthComponent>();
    if (!ai || !health) return;

    // Check health status
    float health_percent = static_cast<float>(health->hp) / health->max_hp;

    // Flee if health is low
    if (health_percent < 0.3f) {
        auto* player_pos = player->getComponent<PositionComponent>();
        if (player_pos && canSeeEntity(entity, player)) {
            Point threat{player_pos->position.x, player_pos->position.y};
            moveAway(entity, threat);
            return;
        }
    }

    // Attack if player is close and we're healthy enough
    if (health_percent > 0.3f && getDistance(entity, player) <= ai->aggro_range) {
        // Try to attack if adjacent
        if (tryAttack(entity, player)) {
            return;
        }

        // Move towards player to attack
        auto* player_pos = player->getComponent<PositionComponent>();
        if (player_pos) {
            Point target{player_pos->position.x, player_pos->position.y};
            moveTowards(entity, target);
        }
    } else {
        // Default to wandering
        handleWanderingBehavior(entity);
    }
}

bool AISystem::canSeeEntity(std::shared_ptr<Entity> entity,
                            std::shared_ptr<Entity> target) const {
    auto* ai = entity->getComponent<AIComponent>();
    if (!ai) return false;

    int distance = getDistance(entity, target);
    if (distance > ai->vision_range) return false;

    // Check line of sight using FOV or simple line algorithm
    auto* entity_pos = entity->getComponent<PositionComponent>();
    auto* target_pos = target->getComponent<PositionComponent>();
    if (!entity_pos || !target_pos) return false;

    // Simple line of sight check - could be improved with proper FOV
    // For now, just check if within vision range
    return distance <= ai->vision_range;
}

int AISystem::getDistance(std::shared_ptr<Entity> e1,
                          std::shared_ptr<Entity> e2) const {
    auto* pos1 = e1->getComponent<PositionComponent>();
    auto* pos2 = e2->getComponent<PositionComponent>();

    if (!pos1 || !pos2) return INT_MAX;

    // Manhattan distance
    return std::abs(pos1->position.x - pos2->position.x) + std::abs(pos1->position.y - pos2->position.y);
}

std::deque<Point> AISystem::findPath(const Point& from, const Point& to) const {
    std::deque<Point> path;
    if (!map) return path;

    // Simple BFS pathfinding
    std::queue<Point> frontier;
    std::unordered_map<int, Point> came_from;
    std::unordered_set<int> visited;

    auto point_to_key = [this](const Point& p) {
        return p.y * map->getWidth() + p.x;
    };

    frontier.push(from);
    visited.insert(point_to_key(from));

    // BFS to find path
    bool found = false;
    while (!frontier.empty() && !found) {
        Point current = frontier.front();
        frontier.pop();

        // Check all 4 directions
        const int dx[] = {0, 1, 0, -1};
        const int dy[] = {-1, 0, 1, 0};

        for (int i = 0; i < 4; ++i) {
            Point next{current.x + dx[i], current.y + dy[i]};

            if (!map->isWalkable(next.x, next.y)) continue;

            int next_key = point_to_key(next);
            if (visited.find(next_key) != visited.end()) continue;

            visited.insert(next_key);
            came_from[next_key] = current;
            frontier.push(next);

            if (next.x == to.x && next.y == to.y) {
                found = true;
                break;
            }
        }
    }

    // Reconstruct path if found
    if (found) {
        Point current = to;
        while (!(current.x == from.x && current.y == from.y)) {
            path.push_front(current);
            int key = point_to_key(current);
            if (came_from.find(key) == came_from.end()) break;
            current = came_from[key];
        }
    }

    return path;
}

bool AISystem::moveTowards(std::shared_ptr<Entity> entity, const Point& target) {
    auto* pos = entity->getComponent<PositionComponent>();
    auto* ai = entity->getComponent<AIComponent>();
    if (!pos || !ai) return false;

    Point current{pos->position.x, pos->position.y};

    // Use cached path if available and still valid
    if (!ai->path.empty()) {
        Point next = ai->path.front();
        if (map && map->isWalkable(next.x, next.y)) {
            int dx = next.x - pos->position.x;
            int dy = next.y - pos->position.y;
            movement_system->queueMove(entity->getID(), dx, dy);
            ai->path.pop_front();
            return true;
        } else {
            // Path blocked, recalculate
            ai->path.clear();
        }
    }

    // Calculate new path if needed
    if (ai->path.empty()) {
        ai->path = findPath(current, target);
        if (!ai->path.empty()) {
            Point next = ai->path.front();
            int dx = next.x - pos->position.x;
            int dy = next.y - pos->position.y;
            movement_system->queueMove(entity->getID(), dx, dy);
            ai->path.pop_front();
            return true;
        }
    }

    // Fallback to simple movement towards target
    int dx = (target.x > pos->position.x) ? 1 : (target.x < pos->position.x) ? -1 : 0;
    int dy = (target.y > pos->position.y) ? 1 : (target.y < pos->position.y) ? -1 : 0;

    if (dx != 0 || dy != 0) {
        movement_system->queueMove(entity->getID(), dx, dy);
        return true;
    }

    return false;
}

bool AISystem::moveAway(std::shared_ptr<Entity> entity, const Point& threat) {
    auto* pos = entity->getComponent<PositionComponent>();
    if (!pos) return false;

    // Move in opposite direction from threat
    int dx = (pos->position.x > threat.x) ? 1 : (pos->position.x < threat.x) ? -1 : 0;
    int dy = (pos->position.y > threat.y) ? 1 : (pos->position.y < threat.y) ? -1 : 0;

    // Try to move away
    if (map && map->isWalkable(pos->position.x + dx, pos->position.y + dy)) {
        movement_system->queueMove(entity->getID(), dx, dy);
        return true;
    }

    // Try perpendicular directions if direct retreat is blocked
    if (dx != 0 && map->isWalkable(pos->position.x, pos->position.y + 1)) {
        movement_system->queueMove(entity->getID(), 0, 1);
        return true;
    }
    if (dx != 0 && map->isWalkable(pos->position.x, pos->position.y - 1)) {
        movement_system->queueMove(entity->getID(), 0, -1);
        return true;
    }
    if (dy != 0 && map->isWalkable(pos->position.x + 1, pos->position.y)) {
        movement_system->queueMove(entity->getID(), 1, 0);
        return true;
    }
    if (dy != 0 && map->isWalkable(pos->position.x - 1, pos->position.y)) {
        movement_system->queueMove(entity->getID(), -1, 0);
        return true;
    }

    return false;
}

bool AISystem::tryAttack(std::shared_ptr<Entity> entity,
                        std::shared_ptr<Entity> target) {
    if (!combat_system) return false;

    // Check if entities are adjacent
    if (combat_system->areAdjacent(entity, target)) {
        combat_system->queueAttack(entity->getID(), target->getID());
        return true;
    }

    return false;
}

Point AISystem::getRandomAdjacentPosition(const Point& pos) const {
    std::uniform_int_distribution<int> dist(0, 3);
    int dir = dist(const_cast<std::mt19937&>(rng));

    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};

    return Point{pos.x + dx[dir], pos.y + dy[dir]};
}

std::shared_ptr<Entity> AISystem::findEntity(
    const std::vector<std::unique_ptr<Entity>>& entities,
    EntityID id) const {

    auto it = std::find_if(entities.begin(), entities.end(),
        [id](const std::unique_ptr<Entity>& e) {
            return e->getID() == id;
        });

    // Create shared_ptr that doesn't own the entity
    return (it != entities.end()) ? std::shared_ptr<Entity>(it->get(), [](Entity*){}) : nullptr;
}

} // namespace ecs