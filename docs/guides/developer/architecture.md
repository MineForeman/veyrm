# Veyrm Architecture Documentation

## Overview

Veyrm is built using a modern Entity Component System (ECS) architecture with PostgreSQL database integration. The game leverages C++23 features, smart pointer memory management, and the Repository pattern for clean separation of concerns between game logic, data persistence, and UI presentation.

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     Main Entry Point                     │
│                        main.cpp                          │
└─────────────────────────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────┐
│                      GameManager                         │
│         Central game state and system coordinator        │
│                   MVC Architecture                      │
└─────────────────────────────────────────────────────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│ Database     │    │ ECS World    │    │ UI Layer     │
│ PostgreSQL   │    │ Game Logic   │    │ FTXUI        │
└──────────────┘    └──────────────┘    └──────────────┘
        │                    │                    │
        ▼                    ▼                    ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│ Repositories │    │  Components  │    │ Controllers  │
│ Data Access  │    │ & Systems    │    │  & Views     │
└──────────────┘    └──────────────┘    └──────────────┘
        │                    │                    │
        ▼                    ▼                    ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│ Auth Service │    │Entity Factory│    │ Login Screen │
│Session Mgmt  │    │ Data Loader  │    │ Game Screen  │
└──────────────┘    └──────────────┘    └──────────────┘
```

## Core Components

### Database Layer (PostgreSQL Integration)

#### DatabaseManager (`db/database_manager.h/cpp`)
- **Purpose**: Connection pooling and database lifecycle management
- **Features**:
  - Connection pooling with configurable min/max connections
  - Automatic reconnection and error handling
  - Transaction support
  - Thread-safe singleton pattern

#### Repository Pattern
- **SaveGameRepository**: Cloud save operations
- **PlayerRepository**: User and session management
- **GameEntityRepository**: Game content data
- **Base Repository**: Common database operations

#### Authentication System (`auth/`)
- **AuthenticationService**: User registration, login, session management
- **ValidationService**: Input validation and security checks
- **Session Management**: Token-based authentication with refresh tokens

### ECS (Entity Component System) Architecture

#### GameWorld (`ecs/game_world.h/cpp`)
- **Purpose**: Central ECS world manager
- **Responsibilities**:
  - Entity lifecycle management
  - Component registration and retrieval
  - System coordination and updates
  - Entity queries and spatial indexing

#### Core Components (`ecs/components/`)
- **PositionComponent**: x, y coordinates
- **HealthComponent**: current/max HP
- **RenderableComponent**: glyph, color for display
- **CombatComponent**: attack, defense stats
- **AIComponent**: behavior state, target tracking
- **InventoryComponent**: item storage
- **StatsComponent**: level, experience, attributes
- **PlayerComponent**: player-specific data
- **ItemComponent**: item properties and type
- **SaveLoadComponent**: save/load state tracking

#### Core Systems (`ecs/systems/`)
- **MovementSystem**: Entity movement and collision
- **CombatSystem**: Bump-to-attack combat resolution
- **AISystem**: Monster behavior and pathfinding
- **RenderSystem**: Entity rendering to map
- **InputSystem**: Player input processing
- **LootSystem**: Item drops and pickups
- **ExperienceSystem**: XP and leveling
- **InventorySystem**: Item management and usage
- **EquipmentSystem**: Equipment handling
- **StatusEffectSystem**: Status effects and buffs
- **SaveLoadSystem**: Game state persistence

#### EntityFactory (`ecs/entity_factory.h/cpp`)
- **Purpose**: Centralized entity creation with proper component assignment
- **Factory Methods**:
  - `createPlayer()`: Player entity with all required components
  - `createMonster()`: Monsters with AI and combat components
  - `createItem()`: Items with properties and effects

### GameManager (`game_manager.h/cpp`)

- **Purpose**: Central orchestrator coordinating all systems
- **Responsibilities**:
  - Game state management (LOGIN, MENU, PLAYING, INVENTORY, etc.)
  - System initialization and coordination
  - Main game loop execution
  - Screen management and navigation
- **Key Members**:
  - `game_world`: ECS world instance
  - `database_manager`: Database connection
  - `auth_service`: User authentication
  - `cloud_save_service`: Save synchronization

### Map System (`map.h/cpp`)

- **Purpose**: World representation and tile management
- **Size**: 198x66 tiles (Angband standard)
- **Features**:
  - Tile-based world (walls, floors, doors, stairs)
  - Visibility tracking (explored/visible)
  - Memory system (remembers explored areas)
  - Lit room support
- **Key Classes**:
  - `Tile`: Individual map cells
  - `Map`: 2D grid of tiles
  - `MapGenerator`: Procedural generation
  - `MapValidator`: Connectivity validation

### Entity System

```
Entity (Base)
├── Player
│   ├── Stats (HP, ATK, DEF)
│   ├── Inventory (26 slots)
│   └── Position
├── Monster
│   ├── AI State
│   ├── Combat Stats
│   └── Behavior
└── Item
    ├── Type (potion, gold, etc.)
    ├── Properties
    └── Effects
```

#### EntityManager (`entity_manager.h/cpp`)

- **Purpose**: Lifecycle management for all entities
- **Responsibilities**:
  - Entity creation/destruction
  - Position tracking
  - Entity queries (at position, in range)
  - Player reference management

#### Player (`player.h/cpp`)

- **Features**:
  - Stats: HP, Attack, Defense, Gold
  - 26-slot inventory system
  - Experience and level tracking
  - FOV calculation (radius 10)

#### Monster System

- **MonsterFactory**: JSON-based monster creation
- **MonsterAI**: State machine (IDLE, ALERT, HOSTILE, FLEEING)
- **SpawnManager**: Dynamic monster spawning
- **Pathfinding**: A* for movement

#### Item System

- **Item**: Base item class with properties
- **ItemFactory**: JSON-based item creation
- **ItemManager**: World item management
- **Inventory**: Player item storage

### Combat System (`combat_system.h/cpp`)

- **Mechanics**: d20-based combat
- **Formula**:
  - Attack Roll: 1d20 + attacker.attack
  - Damage: max(1, attacker.damage - defender.defense)
  - Critical Hit: Natural 20 = double damage
- **Death Handling**: Entity removal at 0 HP

### Turn Management (`turn_manager.h/cpp`)

- **Purpose**: Action-based turn system
- **Speed System**: 100 = normal speed
- **Turn Order**: Priority queue based on action costs

### UI System (FTXUI-based)

```
┌─────────────────────────────────────┐
│            Map Display              │
│         (198x66 tiles)              │
├─────────────────────────────────────┤
│           Status Bar                │
│    HP, Position, Time, Gold         │
├─────────────────────────────────────┤
│          Message Log                │
│     (Scrollable history)            │
└─────────────────────────────────────┘
```

#### Renderer (`game_screen.h/cpp`)

- **Framework**: FTXUI reactive components
- **Layout**: Three-panel responsive design
- **Features**:
  - Color-coded entities
  - Unicode rendering
  - Fullscreen support
  - Frame statistics

### Save/Load System (`game_serializer.h/cpp`)

- **Format**: JSON using nlohmann/json
- **Strategy**: Seed-based map regeneration
- **Features**:
  - 9 save slots
  - Compressed saves (7-8KB)
  - Entity preservation
  - Explored area tracking

## Data Flow

### Input Processing

```
User Input → InputHandler → GameManager → Active System → State Update → Render
```

### Turn Execution

```
1. Player Action → TurnManager
2. TurnManager → Process Player Turn
3. TurnManager → Process Monster Turns (by speed)
4. Combat Resolution (if needed)
5. State Updates
6. Render Frame
```

### Map Generation

```
1. MapGenerator → Create Rooms
2. Connect with Corridors
3. MapValidator → Verify Connectivity
4. Spawn Entities (Monsters, Items)
5. Place Player at Valid Position
```

## Memory Management

### Smart Pointers

- **std::shared_ptr**: Entities (shared ownership)
- **std::unique_ptr**: Systems (single ownership)
- **Raw pointers**: Never own memory

### Resource Management

- RAII throughout
- No manual new/delete
- Automatic cleanup via destructors

## Data Files

### Configuration

- `config.yml`: Game settings (YAML)
- `data/monsters.json`: Monster definitions
- `data/items.json`: Item definitions
- `saves/*.sav`: Save games (JSON)

### Logging

- `logs/veyrm_debug.log`: All events
- `logs/veyrm_player.log`: Player actions
- `logs/veyrm_ai.log`: AI decisions
- `logs/veyrm_combat.log`: Combat events

## Build System

### Dependencies

- **C++23**: Modern language features
- **CMake 3.25+**: Build configuration
- **FTXUI**: Terminal UI
- **nlohmann/json**: Serialization
- **rapidyaml**: Configuration
- **Catch2**: Testing

### Project Structure

```
veyrm/
├── include/        # Headers (.h)
├── src/           # Implementation (.cpp)
├── tests/         # Unit tests
├── data/          # Game data (JSON)
├── DOC/           # Documentation
├── logs/          # Runtime logs
├── saves/         # Save games
└── build/         # Build output
```

## Testing Architecture

### Test Coverage

- 135 unit tests
- 1,777 assertions
- 100% pass rate

### Test Categories

- Map generation and validation
- Entity management
- Combat calculations
- Save/Load integrity
- UI components
- Input handling

## Performance Characteristics

### Metrics

- **Binary Size**: ~2.5 MB
- **Memory Usage**: ~50 MB
- **Save File Size**: 7-8 KB
- **Frame Rate**: 60 FPS
- **Load Time**: <200ms

### Optimization Points

- Seed-based map regeneration
- Sparse coordinate storage for explored areas
- Entity pooling for monsters
- Lazy FOV calculation

## Extension Points

### Adding New Content

1. **Monsters**: Add to `data/monsters.json`
2. **Items**: Add to `data/items.json`
3. **Map Types**: Extend `MapGenerator`
4. **Commands**: Update `InputHandler`

### Future Systems

- **Networking**: Add NetworkManager for multiplayer
- **Database**: Replace JSON with MySQL backend
- **Graphics**: Add tile renderer alongside terminal
- **Audio**: Integrate sound system

## Design Patterns

### Factory Pattern

- MonsterFactory for monster creation
- ItemFactory for item creation

### Singleton Pattern

- Config for global settings
- MonsterFactory/ItemFactory instances

### Observer Pattern

- Message log for event notifications

### State Machine

- Game states (MENU, PLAYING, etc.)
- Monster AI states

### Component Pattern

- Entity system with shared components

## Thread Safety

Currently single-threaded. For future multiplayer:

- GameManager needs mutex protection
- Entity access requires synchronization
- Turn processing must be atomic

## Error Handling

### Strategy

- Exceptions for fatal errors
- Return codes for expected failures
- Logging for debugging
- Graceful degradation where possible

### Recovery

- Save game corruption: Fall back to seed
- Missing data files: Use defaults
- Invalid input: Ignore and log

## Conclusion

The architecture supports the current MVP feature set while providing clear extension points for future development. The component-based design allows for easy addition of new systems without disrupting existing functionality.
