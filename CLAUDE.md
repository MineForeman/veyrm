# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Critical Requirements

**TESTING:** ALL tests must pass before marking any task complete. Run `./build.sh test` after every change.

**EXECUTION:** Always run binaries from project root directory, never from build/.

**USER NOTES:** NEVER modify `docs/project/notes.md` - it's the user's personal scratchpad.

**CONTROLS:** NEVER implement vi-style movement keys (hjkl/yubn). Use arrow keys and numpad only.

## Build Commands

```bash
# Primary commands (always prefer ./build.sh)
./build.sh            # Interactive menu with build status
./build.sh build      # Build debug mode
./build.sh run        # Run with map selection
./build.sh test       # Run ALL tests (must pass)
./build.sh clean      # Clean build directory
./build.sh reset      # Reset terminal after crashes

# Testing commands
./build.sh keys '\njjjq'   # Automated gameplay (Enter, down 2x, quit)
./build.sh dump            # Frame-by-frame dump mode
./build.sh dump '\njjjq'   # Dump with key sequence

# Advanced commands
./build.sh gource         # Create development visualization video
./build.sh diagram        # Generate class diagrams
./build.sh release patch  # Create release (patch/minor/major)

# Running specific tests
./build/bin/veyrm_tests "[ecs]"      # ECS tests only
./build/bin/veyrm_tests "[combat]"   # Combat tests only
```

## Architecture Overview

The game is transitioning from legacy entity classes to a modern **ECS (Entity Component System)** architecture.

### Current Branch: refactor-optimize
- Removing bridge classes (monster_ai.h, spawn_manager.h deleted)
- Consolidating ECS implementation
- All rendering now ECS-only

### Core ECS System (`include/ecs/`, `src/ecs/`)
```
game_world.h/cpp        # Central ECS world manager
entity_factory.h        # Creates entities with components
entity.h               # ECS entity (just an ID)
component.h            # Base component class
system.h               # Base system class
```

### Components
- **PositionComponent**: x, y coordinates
- **HealthComponent**: current/max HP
- **RenderableComponent**: glyph, color for display
- **CombatComponent**: attack, defense stats
- **AIComponent**: behavior state, target tracking
- **InventoryComponent**: item storage
- **StatsComponent**: level, experience, attributes

### Systems
- **MovementSystem**: Handles entity movement and collision
- **CombatSystem**: Bump-to-attack combat resolution
- **AISystem**: Monster behavior and pathfinding
- **RenderSystem**: Entity rendering to map
- **InputSystem**: Player input processing
- **LootSystem**: Item drops and pickups
- **ExperienceSystem**: XP and leveling

### Game Flow Architecture
```
GameManager (main loop)
  ├── MainMenuScreen
  ├── GameScreen (gameplay)
  │    ├── GameWorld (ECS)
  │    ├── Map (tiles)
  │    ├── InputHandler
  │    └── RendererTUI (FTXUI)
  └── SaveLoadScreen
```

### Legacy Systems (Being Phased Out)
- `include/entity.h`: Old base entity class
- `include/monster.h`, `include/player.h`: Being replaced by ECS components
- Bridge classes already removed

## Key Implementation Patterns

### Creating Entities
```cpp
// Always use EntityFactory
auto player = factory.createPlayer(x, y);
auto monster = factory.createMonster("goblin", x, y);
auto item = factory.createItem("potion_minor", x, y);
```

### System Processing
```cpp
// Systems process entities with matching components
movementSystem.update(deltaTime, world);
combatSystem.update(deltaTime, world);
```

### Component Access
```cpp
// Get component from entity
if (auto* pos = world.getComponent<PositionComponent>(entity)) {
    pos->x = newX;
    pos->y = newY;
}
```

## Data Files

### Configuration
- `config.yml`: Game settings, map generation parameters
- `data/monsters.json`: Monster definitions
- `data/items.json`: Item definitions
- Save files: 9 slots with seed-based map regen

### Monster Definition Example
```json
{
  "id": "goblin",
  "glyph": "g",
  "color": "green",
  "hp": 10,
  "atk": [2, 5],
  "def": 1,
  "speed": 100,
  "xp": 15
}
```

## Testing Strategy

### Unit Tests
- Located in `tests/test_*.cpp`
- Run with `./build.sh test`
- Must cover new components/systems

### Integration Tests
- `test_ecs_integration.cpp`: Full ECS workflow
- `test_monster_integration.cpp`: Monster behavior
- `test_input_handler.cpp`: Input processing

### Gameplay Tests
```bash
# Test basic movement
./build.sh keys '\njjjq'

# Test combat in arena
./build.sh run arena

# Debug rendering issues
./build.sh dump '\n\u\u\r\r\q'
```

## Common Tasks

### Adding a New Component
1. Create `include/ecs/<name>_component.h`
2. Inherit from `ecs::Component`
3. Add factory method in `entity_factory.cpp`
4. Write tests in `tests/test_ecs.cpp`

### Adding a New System
1. Create `include/ecs/<name>_system.h`
2. Inherit from `ecs::System`
3. Register in `game_world.cpp`
4. Write tests in `tests/test_ecs_systems.cpp`

### Adding a New Monster
1. Add to `data/monsters.json`
2. Test with `./build.sh run arena`
3. Verify spawning works

### Debugging ECS Issues
1. Check `GameWorld::getEntityAt()` for position queries
2. Verify component registration in factory
3. Ensure systems are added to world
4. Use `./build.sh dump` to see frame-by-frame

## Map Types for Testing

- **procedural**: Random dungeon generation (default)
- **arena**: Open combat testing area
- **dungeon**: Fixed 5-room layout
- **room**: Single 20x20 room
- **corridor**: Long corridor test
- **stress**: Large map performance test

## Important Files

- `game_screen.cpp`: Main gameplay logic and ECS integration point
- `input_handler.cpp`: Keyboard mapping and input dispatch
- `game_world.cpp`: ECS world implementation
- `entity_factory.h`: All entity creation logic
- `docs/project/notes.md`: User's notes (DO NOT EDIT)