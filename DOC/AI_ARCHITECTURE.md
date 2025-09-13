# AI Architecture Documentation

## Overview

Veyrm's AI system provides intelligent behavior for monsters through a state-based architecture with integrated pathfinding, territorial behavior, and tactical decision making. The system is designed for performance, extensibility, and maintainable code.

## Core Components

### MonsterAI Class (`include/monster_ai.h`, `src/monster_ai.cpp`)

The central AI controller that manages monster behavior through a state machine pattern.

#### Key Features
- **5 AI States**: IDLE, ALERT, HOSTILE, FLEEING, RETURNING
- **Room Assignment**: Territorial behavior with spawn room binding
- **Memory System**: Tracks last seen player position
- **State Transitions**: Context-aware behavior changes
- **Performance Optimized**: Pool-based memory management

#### Public Interface
```cpp
class MonsterAI {
public:
    void updateMonsterAI(Monster& monster, const Player& player, const Map& map);
    Point getNextMove(Monster& monster, const Player& player, const Map& map);
    void assignRoomToMonster(Monster& monster, Room* room);
    bool canSeePlayer(const Monster& monster, const Player& player, const Map& map);
};
```

#### AI State Machine
```
IDLE ←→ ALERT ←→ HOSTILE
  ↑                ↓
  └── RETURNING ←── FLEEING
```

### Pathfinding System (`include/pathfinding.h`, `src/pathfinding.cpp`)

A* pathfinding implementation optimized for 8-directional movement with obstacle avoidance.

#### Features
- **A* Algorithm**: Optimal path calculation
- **8-Way Movement**: Full directional support including diagonals
- **Line-of-Sight**: Bresenham line algorithm for vision
- **Performance**: Early termination and efficient data structures
- **Collision Avoidance**: Respects entity blocking

#### Public Interface
```cpp
class Pathfinding {
public:
    static std::vector<Point> findPath(const Point& start, const Point& goal,
                                       const Map& map, bool allow_diagonals = true);
    static bool hasLineOfSight(const Point& from, const Point& to, const Map& map);
    static float getDistance(const Point& a, const Point& b);
};
```

## Data Structures

### AIData Structure
Each monster maintains AI state through the AIData component:

```cpp
struct AIData {
    AIState current_state = AIState::IDLE;
    Point home_room_center = Point(-1, -1);
    Room* assigned_room = nullptr;
    Point last_player_pos = Point(-1, -1);
    int turns_since_player_seen = 0;
    int idle_move_counter = 0;
    std::vector<Point> current_path;
    size_t path_index = 0;
};
```

### Entity User Data
AI data is attached to monsters via the Entity base class user data system:

```cpp
// Entity base class
void* getUserData() const;
void setUserData(void* data);

// MonsterAI manages the AIData lifecycle
AIData* getAIData(Monster& monster);
void ensureAIData(Monster& monster);
```

## Integration Points

### Game Systems Integration

#### Turn Manager (`src/turn_manager.cpp`)
AI updates are integrated into the world turn phase:

```cpp
void TurnManager::processWorldTurn() {
    // ... existing code ...

    // Update monsters
    if (game_manager) {
        game_manager->updateMonsters();
    }

    endTurn();
}
```

#### Game Manager (`src/game_manager.cpp`)
Coordinates AI processing for all active monsters:

```cpp
void GameManager::updateMonsters() {
    auto monsters = entity_manager->getMonsters();
    for (auto& monster_ptr : monsters) {
        if (monster_ptr && monster_ptr->canAct()) {
            Monster* monster = dynamic_cast<Monster*>(monster_ptr.get());
            // Update AI state and get next move
            monster_ai->updateMonsterAI(*monster, *player, *map);
            Point next_pos = monster_ai->getNextMove(*monster, *player, *map);
            // Handle movement and combat
        }
    }
}
```

#### Spawn Manager (`src/spawn_manager.cpp`)
Automatically assigns rooms to monsters during creation:

```cpp
auto monster = entity_manager.createMonster(species, point.x, point.y);
if (monster && game_manager && game_manager->getMonsterAI()) {
    Room* room = map.getRoomAt(point.x, point.y);
    if (room) {
        Monster* monster_ptr = dynamic_cast<Monster*>(monster.get());
        game_manager->getMonsterAI()->assignRoomToMonster(*monster_ptr, room);
    }
}
```

## AI Behavior Details

### State Descriptions

#### IDLE State
- **Trigger**: Default state, no player detected
- **Behavior**: Random movement within room boundaries
- **Frequency**: Low movement rate (every 3+ turns)
- **Restrictions**: Room-bound monsters stay within assigned room

#### ALERT State
- **Trigger**: Player detected at medium distance (8-10 tiles)
- **Behavior**: Increased attention, faces toward player
- **Movement**: May investigate last known position
- **Duration**: Until player comes closer or disappears

#### HOSTILE State
- **Trigger**: Player visible within combat range (≤8 tiles)
- **Behavior**: Active pursuit using pathfinding
- **Movement**: Direct approach via optimal path
- **Exception**: Can leave assigned room to chase player

#### FLEEING State
- **Trigger**: Monster health < 25% and player visible
- **Behavior**: Escape from player using reverse pathfinding
- **Movement**: Maximize distance from player
- **Duration**: Until player out of sight for 3+ turns

#### RETURNING State
- **Trigger**: Room-bound monster lost player outside assigned room
- **Behavior**: Navigate back to home room center
- **Movement**: Pathfind to room center point
- **Completion**: Reverts to IDLE once inside room

### Configuration Parameters

```cpp
static const int DEFAULT_VISION_RANGE = 8;    // Tiles for monster sight
static const int ALERT_RANGE = 10;            // Distance for alert state
static const int HOSTILE_RANGE = 8;           // Distance for hostile state
static const int MEMORY_TURNS = 5;            // Turns to remember player
static const int RETURN_THRESHOLD = 15;       // Distance before returning
```

## Performance Considerations

### Memory Management
- **Pool Allocation**: AIData stored in managed pool
- **Entity Components**: Minimal memory overhead per monster
- **Path Caching**: Reuse calculated paths when valid
- **Early Termination**: Stop processing distant monsters

### Computational Efficiency
- **A* Optimization**: Efficient priority queue and data structures
- **FOV Reuse**: Leverage existing field-of-view calculations
- **State Caching**: Avoid redundant state evaluations
- **Batch Processing**: Update all monsters in single pass

### Scalability Targets
- **30+ Monsters**: Smooth performance with full AI population
- **Large Maps**: Efficient pathfinding on 100x100+ grids
- **Real-time**: No perceptible lag during AI processing
- **Memory**: Minimal per-monster overhead (< 100 bytes)

## Testing Strategy

### Unit Tests (`tests/test_monster_ai.cpp`)

#### Core Functionality
- **State Transitions**: All state changes work correctly
- **Pathfinding**: Optimal path calculation verified
- **8-Way Movement**: All directions tested
- **Line-of-Sight**: Vision and obstacle detection

#### Behavioral Testing
- **Room Boundaries**: Territorial behavior validation
- **Chase Mechanics**: Pursuit and engagement
- **Fleeing Logic**: Escape behavior verification
- **Combat Approach**: Intelligent positioning

#### Integration Testing
- **Multi-Monster**: Collision avoidance and coordination
- **Performance**: Stress testing with many active monsters
- **Edge Cases**: Boundary conditions and error handling

### Test Coverage
- **120 Test Cases**: Comprehensive validation suite
- **1602 Assertions**: Detailed behavior verification
- **All Passing**: ✅ No failing tests
- **Continuous Integration**: Automated testing on changes

## Future Extensions

### Planned Enhancements
- **Group AI**: Pack behavior and monster coordination
- **Advanced Pathfinding**: Hierarchical and multi-threaded
- **Dynamic Difficulty**: AI adaptation based on player skill
- **Sound System**: Audio-based detection and alerts
- **Patrol Routes**: Predefined movement patterns

### Extensibility Points
- **State Addition**: Easy to add new AI states
- **Behavior Customization**: Per-species AI parameters
- **Plugin System**: External AI behavior modules
- **Configuration**: Runtime tunable AI parameters

## Debugging and Monitoring

### Debug Information
- **State Visualization**: Current AI state per monster
- **Path Display**: Visual pathfinding routes
- **Vision Cones**: Monster sight range indicators
- **Performance Metrics**: AI processing timing

### Logging Integration
- **State Changes**: Track AI transitions
- **Pathfinding**: Log path calculation results
- **Performance**: Monitor AI update timing
- **Error Handling**: Graceful failure recovery

## Code Organization

### File Structure
```
include/
  monster_ai.h      - AI system interface
  pathfinding.h     - Pathfinding system interface

src/
  monster_ai.cpp    - Core AI implementation (270+ lines)
  pathfinding.cpp   - A* pathfinding logic (140+ lines)

tests/
  test_monster_ai.cpp - Unit tests (150+ lines)
```

### Dependencies
- **Entity System**: Base monster and player classes
- **Map System**: Tile data and room information
- **Turn System**: Integration with game timing
- **FOV System**: Reused for monster vision

The AI architecture provides a robust foundation for intelligent monster behavior while maintaining performance and extensibility for future enhancements.