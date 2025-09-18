/**
 * @file ai_system.cpp
 * @brief Native ECS AI system implementation
 */

#include <algorithm>
#include <queue>
#include <random>
#include <limits>
#include <climits>
#include <deque>
#include <unordered_set>
#include <cmath>
#include "ecs/ai_system.h"
#include "ecs/movement_system.h"
#include "ecs/combat_system.h"
#include "ecs/health_component.h"
#include "ecs/renderable_component.h"

namespace ecs {

AISystem::AISystem(Map* map,
                   MovementSystem* movement_system,
                   CombatSystem* combat_system,
                   ILogger* logger)
    : map(map)
    , movement_system(movement_system)
    , combat_system(combat_system)
    , logger(logger)
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
        if (!ai->has_seen_player && logger) {
            logger->logAI("Entity " + std::to_string(entity->getID()) + " spotted player");
        }
        ai->has_seen_player = true;
        ai->turns_since_player_seen = 0;
        auto* player_pos = player->getComponent<PositionComponent>();
        if (player_pos) {
            ai->last_player_position = Point{player_pos->position.x, player_pos->position.y};
        }
    } else {
        ai->turns_since_player_seen++;
        if (ai->turns_since_player_seen == 10 && logger) {
            logger->logAI("Entity " + std::to_string(entity->getID()) + " lost track of player");
        }
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
            if (logger) logger->logAI("Entity " + std::to_string(entity->getID()) + " acting aggressively");
            handleAggressiveBehavior(entity, player);
            break;
        case AIBehavior::DEFENSIVE:
            handleDefensiveBehavior(entity, player);
            break;
        case AIBehavior::PATROL:
            handlePatrolBehavior(entity);
            break;
        case AIBehavior::FLEEING:
            if (logger) logger->logAI("Entity " + std::to_string(entity->getID()) + " fleeing from threat");
            handleFleeingBehavior(entity, player);
            break;
        case AIBehavior::SUPPORT:
            handleSupportBehavior(entity, player);
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

void AISystem::handlePatrolBehavior(std::shared_ptr<Entity> entity) {
    auto* ai = entity->getComponent<AIComponent>();
    auto* pos = entity->getComponent<PositionComponent>();
    if (!ai || !pos) return;

    // Initialize patrol points if not set
    if (ai->patrol_points.empty()) {
        Point current{pos->position.x, pos->position.y};
        // Create a simple square patrol pattern
        ai->patrol_points.push_back({current.x + 3, current.y});
        ai->patrol_points.push_back({current.x + 3, current.y + 3});
        ai->patrol_points.push_back({current.x, current.y + 3});
        ai->patrol_points.push_back(current);
        ai->current_patrol_index = 0;
    }

    // Move to current patrol point
    if (ai->current_patrol_index < ai->patrol_points.size()) {
        Point target = ai->patrol_points[ai->current_patrol_index];
        Point current{pos->position.x, pos->position.y};

        // Check if we've reached the patrol point
        if (current.x == target.x && current.y == target.y) {
            // Move to next patrol point
            ai->current_patrol_index = (ai->current_patrol_index + 1) % ai->patrol_points.size();
            target = ai->patrol_points[ai->current_patrol_index];
        }

        // Move towards patrol point
        moveTowards(entity, target);
    }
}

void AISystem::handleFleeingBehavior(std::shared_ptr<Entity> entity,
                                     std::shared_ptr<Entity> threat) {
    if (!threat) return;

    auto* threat_pos = threat->getComponent<PositionComponent>();
    if (!threat_pos) return;

    Point threat_point{threat_pos->position.x, threat_pos->position.y};
    moveAway(entity, threat_point);

    // Check if we're far enough away to stop fleeing
    auto* ai = entity->getComponent<AIComponent>();
    if (ai && getDistance(entity, threat) > ai->vision_range * 2) {
        // Switch back to wandering or defensive
        ai->behavior = AIBehavior::DEFENSIVE;
    }
}

void AISystem::handleSupportBehavior(std::shared_ptr<Entity> entity,
                                     std::shared_ptr<Entity> player) {
    if (!player) return;

    auto* ai = entity->getComponent<AIComponent>();
    if (!ai) return;

    // Find nearby allies that need help
    auto* pos = entity->getComponent<PositionComponent>();
    if (!pos) return;

    Point current{pos->position.x, pos->position.y};

    // Note: Would need access to all entities to find allies
    // For now, just follow player
    /*for (const auto& other : entities) {
        if (other->getID() == entity->getID()) continue;

        auto* other_ai = other->getComponent<AIComponent>();
        if (!other_ai || other_ai->behavior == AIBehavior::AGGRESSIVE) continue;

        auto* other_health = other->getComponent<HealthComponent>();
        if (!other_health) continue;

        // Check if ally is injured
        if (other_health->hp < other_health->max_hp / 2) {
            // Move towards injured ally
            auto* other_pos = other->getComponent<PositionComponent>();
            if (other_pos && getDistance(entity, other) <= ai->vision_range) {
                Point ally_pos{other_pos->position.x, other_pos->position.y};
                moveTowards(entity, ally_pos);
                return;
            }
        }
    }*/

    // Follow the player at a distance
    if (getDistance(entity, player) > 3) {
        auto* player_pos = player->getComponent<PositionComponent>();
        if (player_pos) {
            Point player_point{player_pos->position.x, player_pos->position.y};
            moveTowards(entity, player_point);
        }
    }
}

} // namespace ecs