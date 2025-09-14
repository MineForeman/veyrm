#include "monster_ai.h"
#include "monster.h"
#include "player.h"
#include "map.h"
#include "room.h"
#include "pathfinding.h"
#include "log.h"
#include <random>
#include <algorithm>

MonsterAI::MonsterAI() {
}

MonsterAI::~MonsterAI() = default;

void MonsterAI::updateMonsterAI(Monster& monster, const Player& player, const Map& map) {
    ensureAIData(monster);
    updateState(monster, player, map);
}

Point MonsterAI::getNextMove(Monster& monster, const Player& player, const Map& map) {
    ensureAIData(monster);
    AIData* data = getAIData(monster);

    Point next_pos = monster.getPosition();

    std::string state_name;
    switch (data->current_state) {
        case AIState::IDLE: state_name = "IDLE"; break;
        case AIState::ALERT: state_name = "ALERT"; break;
        case AIState::HOSTILE: state_name = "HOSTILE"; break;
        case AIState::FLEEING: state_name = "FLEEING"; break;
        case AIState::RETURNING: state_name = "RETURNING"; break;
    }

    LOG_AI("Monster " + monster.name + " in state " + state_name +
           " at (" + std::to_string(monster.x) + "," + std::to_string(monster.y) + ")" +
           " player at (" + std::to_string(player.x) + "," + std::to_string(player.y) + ")");

    switch (data->current_state) {
        case AIState::IDLE:
            next_pos = chooseIdleMove(monster, map);
            break;

        case AIState::ALERT:
            if (data->last_player_pos.x >= 0) {
                next_pos = chooseHostileMove(monster, player, map);
            } else {
                LOG_AI("Monster " + monster.name + " in ALERT but no last_player_pos");
            }
            break;

        case AIState::HOSTILE:
            next_pos = chooseHostileMove(monster, player, map);
            break;

        case AIState::FLEEING:
            next_pos = chooseFleeingMove(monster, player, map);
            break;

        case AIState::RETURNING:
            next_pos = chooseReturnMove(monster, map);
            break;
    }

    if (next_pos != monster.getPosition()) {
        LOG_AI("Monster " + monster.name + " choosing to move to (" +
               std::to_string(next_pos.x) + "," + std::to_string(next_pos.y) + ")");
    } else {
        LOG_AI("Monster " + monster.name + " choosing to stay at current position");
    }

    return next_pos;
}

void MonsterAI::assignRoomToMonster(Monster& monster, Room* room) {
    ensureAIData(monster);
    AIData* data = getAIData(monster);

    data->assigned_room = room;
    if (room) {
        data->home_room_center = Point(
            (room->x + room->x + room->width) / 2,
            (room->y + room->y + room->height) / 2
        );
    }
}

bool MonsterAI::canSeePlayer(const Monster& monster, const Player& player, const Map& map) {
    Point monster_pos = monster.getPosition();
    Point player_pos = player.getPosition();

    float distance = Pathfinding::getDistance(monster_pos, player_pos);
    if (distance > DEFAULT_VISION_RANGE) {
        return false;
    }

    return Pathfinding::hasLineOfSight(monster_pos, player_pos, map);
}

void MonsterAI::updateState(Monster& monster, const Player& player, const Map& map) {
    AIData* data = getAIData(monster);
    Point monster_pos = monster.getPosition();
    Point player_pos = player.getPosition();

    bool can_see = canSeePlayer(monster, player, map);
    float distance = Pathfinding::getDistance(monster_pos, player_pos);

    LOG_AI("Monster " + monster.name + " updateState: can_see=" + std::string(can_see ? "true" : "false") +
           ", distance=" + std::to_string(distance));

    float health_percent = (float)monster.hp / (float)monster.max_hp;
    bool should_flee = health_percent < 0.25f && monster.species != "orc";

    if (can_see) {
        data->last_player_pos = player_pos;
        data->turns_since_player_seen = 0;

        if (should_flee) {
            data->current_state = AIState::FLEEING;
        } else if (distance <= HOSTILE_RANGE) {
            data->current_state = AIState::HOSTILE;
        } else if (distance <= ALERT_RANGE) {
            data->current_state = AIState::ALERT;
        }
    } else {
        data->turns_since_player_seen++;

        if (data->current_state == AIState::FLEEING && data->turns_since_player_seen > 3) {
            data->current_state = AIState::IDLE;
        } else if (data->current_state == AIState::HOSTILE || data->current_state == AIState::ALERT) {
            if (data->turns_since_player_seen > MEMORY_TURNS) {
                if (data->assigned_room && !isInRoom(monster_pos, data->assigned_room)) {
                    data->current_state = AIState::RETURNING;
                } else {
                    data->current_state = AIState::IDLE;
                }
            }
        } else if (data->current_state == AIState::RETURNING) {
            if (data->assigned_room && isInRoom(monster_pos, data->assigned_room)) {
                data->current_state = AIState::IDLE;
            }
        }
    }
}

Point MonsterAI::chooseIdleMove(const Monster& monster, const Map& map) {
    AIData* data = getAIData(const_cast<Monster&>(monster));
    Point current = monster.getPosition();

    data->idle_move_counter++;
    if (data->idle_move_counter < 3) {
        return current;
    }
    data->idle_move_counter = 0;

    std::vector<Point> valid_moves = getValidMoves(current, map);

    if (data->assigned_room) {
        valid_moves.erase(
            std::remove_if(valid_moves.begin(), valid_moves.end(),
                [this, data](const Point& p) {
                    return !isInRoom(p, data->assigned_room);
                }),
            valid_moves.end()
        );
    }

    if (valid_moves.empty()) {
        return current;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, valid_moves.size() - 1);

    return valid_moves[dis(gen)];
}

Point MonsterAI::chooseHostileMove(Monster& monster, const Player& player, const Map& map) {
    AIData* data = getAIData(monster);
    Point current = monster.getPosition();
    Point target = player.getPosition();

    if (data->last_player_pos.x >= 0 && data->turns_since_player_seen > 0) {
        target = data->last_player_pos;
    }

    if (data->current_path.empty() || data->path_index >= data->current_path.size()) {
        data->current_path = Pathfinding::findPath(current, target, map, true);
        data->path_index = 0;
    }

    if (!data->current_path.empty() && data->path_index < data->current_path.size()) {
        Point next = data->current_path[data->path_index];
        data->path_index++;
        return next;
    }

    std::vector<Point> valid_moves = getValidMoves(current, map);
    if (valid_moves.empty()) {
        return current;
    }

    Point best_move = current;
    float best_dist = Pathfinding::getDistance(current, target);

    for (const Point& move : valid_moves) {
        float dist = Pathfinding::getDistance(move, target);
        if (dist < best_dist) {
            best_dist = dist;
            best_move = move;
        }
    }

    return best_move;
}

Point MonsterAI::chooseFleeingMove(Monster& monster, const Player& player, const Map& map) {
    Point current = monster.getPosition();
    Point player_pos = player.getPosition();

    std::vector<Point> valid_moves = getValidMoves(current, map);
    if (valid_moves.empty()) {
        return current;
    }

    Point best_move = current;
    float best_dist = Pathfinding::getDistance(current, player_pos);

    for (const Point& move : valid_moves) {
        float dist = Pathfinding::getDistance(move, player_pos);
        if (dist > best_dist) {
            best_dist = dist;
            best_move = move;
        }
    }

    return best_move;
}

Point MonsterAI::chooseReturnMove(Monster& monster, const Map& map) {
    AIData* data = getAIData(monster);
    Point current = monster.getPosition();

    if (!data->assigned_room || data->home_room_center.x < 0) {
        return current;
    }

    if (data->current_path.empty() || data->path_index >= data->current_path.size()) {
        data->current_path = Pathfinding::findPath(current, data->home_room_center, map, true);
        data->path_index = 0;
    }

    if (!data->current_path.empty() && data->path_index < data->current_path.size()) {
        Point next = data->current_path[data->path_index];
        data->path_index++;
        return next;
    }

    return current;
}

bool MonsterAI::isInRoom(const Point& pos, const Room* room) {
    if (!room) return false;

    return pos.x >= room->x && pos.x < room->x + room->width &&
           pos.y >= room->y && pos.y < room->y + room->height;
}

float MonsterAI::getDistance(const Point& a, const Point& b) {
    return Pathfinding::getDistance(a, b);
}

std::vector<Point> MonsterAI::getValidMoves(const Point& pos, const Map& map) {
    std::vector<Point> moves;

    for (int i = 0; i < 8; ++i) {
        Point new_pos = pos + Pathfinding::DIRECTIONS_8[i];

        if (!map.inBounds(new_pos.x, new_pos.y)) {
            continue;
        }

        auto props = Map::getTileProperties(map.getTile(new_pos.x, new_pos.y));
        if (props.walkable) {
            moves.push_back(new_pos);
        }
    }

    return moves;
}

MonsterAI::AIData* MonsterAI::getAIData(Monster& monster) {
    // Use new type-safe interface
    AIData* data = monster.getAIData();
    if (!data) {
        ensureAIData(monster);
        data = monster.getAIData();
    }
    return data;
}

void MonsterAI::ensureAIData(Monster& monster) {
    if (!monster.hasAIData()) {
        // Create and transfer ownership to the entity
        auto ai_data = std::make_shared<AIData>();
        monster.setAIData(std::move(ai_data));
    }
}