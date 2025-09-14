/**
 * @file monster_ai.h
 * @brief AI behavior system for monsters
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include "point.h"
#include <vector>
#include <memory>

class Map;
class Player;
class Monster;
class Room;

/**
 * @class MonsterAI
 * @brief Manages AI behavior for all monsters in the game
 *
 * The MonsterAI class implements a state-based AI system that controls
 * monster behavior including idle wandering, player detection, combat
 * pursuit, fleeing, and returning to assigned areas. Each monster
 * maintains individual AI state and memory for intelligent behavior.
 *
 * AI States:
 * - IDLE: Random wandering within assigned room
 * - ALERT: Player detected, moving to investigate
 * - HOSTILE: Actively pursuing and attacking player
 * - FLEEING: Retreating from combat when injured
 * - RETURNING: Moving back to assigned room after losing player
 *
 * Features:
 * - Room-based territorial behavior
 * - Line-of-sight detection with memory
 * - Pathfinding integration for smart movement
 * - Configurable vision and aggression ranges
 *
 * @see Monster
 * @see Pathfinding
 * @see Config monster settings
 */
// AIData struct for monster AI state
enum class AIState {
    IDLE,
    ALERT,
    HOSTILE,
    FLEEING,
    RETURNING
};

struct MonsterAIData {
    AIState current_state = AIState::IDLE;
    Point home_room_center = Point(-1, -1);
    Room* assigned_room = nullptr;
    Point last_player_pos = Point(-1, -1);
    int turns_since_player_seen = 0;
    int idle_move_counter = 0;
    std::vector<Point> current_path;
    size_t path_index = 0;
};

class MonsterAI {
public:
    // Type aliases for compatibility
    using AIState = ::AIState;
    using AIData = MonsterAIData;

    MonsterAI();
    ~MonsterAI();

    void updateMonsterAI(Monster& monster, const Player& player, const Map& map);
    Point getNextMove(Monster& monster, const Player& player, const Map& map);

    void assignRoomToMonster(Monster& monster, Room* room);
    bool canSeePlayer(const Monster& monster, const Player& player, const Map& map);

    static const int DEFAULT_VISION_RANGE = 8;
    static const int ALERT_RANGE = 10;
    static const int HOSTILE_RANGE = 8;
    static const int MEMORY_TURNS = 5;
    static const int RETURN_THRESHOLD = 15;

private:
    void updateState(Monster& monster, const Player& player, const Map& map);
    Point chooseIdleMove(const Monster& monster, const Map& map);
    Point chooseHostileMove(Monster& monster, const Player& player, const Map& map);
    Point chooseFleeingMove(Monster& monster, const Player& player, const Map& map);
    Point chooseReturnMove(Monster& monster, const Map& map);

    bool isInRoom(const Point& pos, const Room* room);
    float getDistance(const Point& a, const Point& b);
    std::vector<Point> getValidMoves(const Point& pos, const Map& map);

    AIData* getAIData(Monster& monster);
    void ensureAIData(Monster& monster);

    // No longer need pool - AI data is owned by entities
};