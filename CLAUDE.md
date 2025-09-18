# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Git Remote Configuration

**IMPORTANT:** This repository uses GitLab as the primary remote.

- **Default push destination:** `ssh://git@horse.local:23/nrf/veyrm.git` (GitLab)
- **GitHub mirror:** `origin` (github.com/MineForeman/veyrm) - Only push here when explicitly requested
- Always use `git push ssh://git@horse.local:23/nrf/veyrm.git <branch>` for normal pushes
- Only use `git push origin <branch>` when the user explicitly asks to push to GitHub

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

# Database commands (PostgreSQL integration)
./build.sh db create  # Create database tables
./build.sh db load    # Load initial data
./build.sh db status  # Check database connection
./build.sh db reset   # Clear and reload database

# Testing commands
./build.sh keys '\njjjq'       # Automated gameplay (Enter, down 2x, quit)
./build.sh testkeys            # Manual gameplay with test credentials
./build.sh autologin           # Auto-login + automated keys (default: Enter, down 2x, quit)
./build.sh autologin '\njjjq'  # Auto-login + custom key sequence
./build.sh cleardata           # Clear test data for NRF user
./build.sh dump               # Frame-by-frame dump mode
./build.sh dump '\njjjq'      # Dump with key sequence

# Testing with authentication (bypasses login screen)
./build/bin/veyrm --username NRF --password '@Cwtt4eva'
./build/bin/veyrm --username NRF --password '@Cwtt4eva' --keys '\n\r\r\r\rq'

# Advanced commands
./build.sh gource         # Create development visualization video
./build.sh diagram        # Generate class diagrams
./build.sh release patch  # Create release (patch/minor/major)

# Running specific tests
./build/bin/veyrm_tests "[ecs]"        # ECS tests only
./build/bin/veyrm_tests "[combat]"     # Combat tests only
./build/bin/veyrm_tests "[validator]"  # Map validation tests
./build/bin/veyrm_tests "[data]"       # Data loading tests
./build/bin/veyrm_tests "[json]"       # JSON parsing tests
```

## Architecture Overview

The game uses a modern **ECS (Entity Component System)** architecture. Legacy entity classes have been completely removed.

### Core ECS System (`include/ecs/`, `src/ecs/`)

```text
game_world.h/cpp        # Central ECS world manager
entity_factory.h        # Creates entities with components
entity.h               # ECS entity (just an ID)
component.h            # Base component class
system.h               # Base system class
```

### Key Components

- **PositionComponent**: x, y coordinates
- **HealthComponent**: current/max HP
- **RenderableComponent**: glyph, color for display
- **CombatComponent**: attack, defense stats
- **AIComponent**: behavior state, target tracking
- **InventoryComponent**: item storage
- **StatsComponent**: level, experience, attributes
- **PlayerComponent**: player-specific data
- **ItemComponent**: item properties and type
- **LootComponent**: loot drop configuration
- **ExperienceComponent**: XP tracking and leveling
- **EquipmentComponent**: equipped items
- **EffectsComponent**: status effects and buffs
- **SaveLoadComponent**: save/load state tracking

### Key Systems

- **MovementSystem**: Handles entity movement and collision
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

### Services (`include/services/`, `src/services/`)

- **CloudSaveService**: PostgreSQL cloud save integration
- **SyncStatus**: Save synchronization state tracking

### Game Flow Architecture

```text
GameManager (main loop)
  ├── LoginScreen (PostgreSQL auth)
  ├── MainMenuScreen
  ├── GameScreen (gameplay)
  │    ├── GameWorld (ECS)
  │    ├── Map (tiles)
  │    ├── InputHandler
  │    └── RendererTUI (FTXUI)
  └── SaveLoadScreen
```

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

- `config.json`: Game settings, map generation parameters (JSON format)
- `data/monsters.json`: Monster definitions with ECS components
- `data/items.json`: Item definitions with properties
- Save files: 9 slots with seed-based map regeneration

### Monster Definition Format

```json
{
  "id": "goblin",
  "glyph": "g",
  "color": "green",
  "components": {
    "health": { "max_hp": 20, "hp": 20 },
    "combat": { "min_damage": 1, "max_damage": 4 },
    "ai": { "behavior": "aggressive", "vision_range": 6 }
  }
}
```

## Testing Strategy

### Unit Tests

- Located in `tests/test_*.cpp` (45+ test files)
- Run with `./build.sh test` or specific test categories
- Must cover new components/systems
- Uses Catch2 framework

### Integration Tests

- `test_ecs_integration.cpp`: Full ECS workflow
- `test_monster_integration.cpp`: Monster behavior
- `test_input_handler.cpp`: Input processing
- `test_database_basic.cpp`: Database connectivity
- `test_cloud_save_basic.cpp`: Cloud save integration

### Test Categories (use with `./build/bin/veyrm_tests "[category]"`)

- `[ecs]`: Entity Component System tests
- `[combat]`: Combat system tests
- `[validator]`: Map validation tests
- `[data]`: Data loading tests
- `[json]`: JSON parsing tests
- `[database]`: Database integration tests
- `[cloud]`: Cloud save tests

### Gameplay Tests

```bash
# Test basic movement
./build.sh keys '\njjjq'

# Test combat in arena
./build.sh run arena

# Debug rendering issues
./build.sh dump '\n\u\u\r\r\q'
```

## Database Configuration

### PostgreSQL + Redis Docker Setup

The project includes both PostgreSQL database and Redis cache integration. The Docker setup is located at `/Users/nrf/repos/PostgreSQL` and provides a complete development stack.

**Docker Services:**

- **PostgreSQL 16 Alpine** on port 5432 - Main game database
- **Redis 7 Alpine** on port 6379 - Cache and session storage
- **PgAdmin** on port 5050 - PostgreSQL web UI
- **Redis Commander** on port 8081 - Redis web UI

**Containers:**
- `veyrm-postgres` - PostgreSQL database with game schema
- `veyrm-redis` - Redis cache with password auth and persistence
- `veyrm-pgadmin` - PostgreSQL management interface
- `veyrm-redis-commander` - Redis management interface

**Quick Commands (from `/Users/nrf/repos/PostgreSQL`):**

```bash
# Essential operations
./build.sh start         # Start all containers (PostgreSQL, Redis, UIs)
./build.sh stop          # Stop all containers
./build.sh status        # Show container and database status
./build.sh health        # Comprehensive health check (PostgreSQL & Redis)

# Database operations
./build.sh shell         # Access PostgreSQL CLI
./build.sh redis-cli     # Access Redis CLI
./build.sh backup        # Create timestamped backup
./build.sh restore       # Interactive restore from backup

# Direct Docker commands
docker compose up -d          # Start all services
docker compose logs postgres  # View PostgreSQL logs
docker compose down          # Stop all services
```

**Connection Details:**

```cpp
// PostgreSQL connection string for C++
"host=localhost port=5432 dbname=veyrm_db user=veyrm_admin password=changeme_to_secure_password"

// Redis connection (if needed for caching)
"redis://:changeme_to_secure_password@localhost:6379"
```

**Web Interfaces:**
- PgAdmin: http://localhost:5050 (Database management)
- Redis Commander: http://localhost:8081 (Cache management)

**Environment Setup:**
The Docker setup automatically creates `.env` from `.env.example` if missing. Key variables:
- `POSTGRES_PASSWORD` - Database password
- `REDIS_PASSWORD` - Redis password
- `PGADMIN_PASSWORD` - PgAdmin web UI password
- `REDIS_UI_PASSWORD` - Redis Commander password

See `docs/database/postgres-setup.md` for full setup details.

**Test Account Credentials:**

For testing authentication and movement functionality:
- Username: `NRF`
- Password: `@Cwtt4eva`

**Command Line Authentication:**

Skip the login screen for testing by providing credentials directly:
```bash
./build/bin/veyrm --username NRF --password '@Cwtt4eva'
```

This bypasses the PostgreSQL authentication screen and goes directly to the main menu. Perfect for automated testing and development workflows.

## CI/CD Configuration

The project uses GitLab CI with runners for both Linux and Windows builds:

- **Linux builds**: Use Docker with Ubuntu 24.04
- **Windows builds**: Use native Windows runners
- **Artifacts**: Build outputs are preserved for 1 week
- **Testing**: Automated test execution on all platforms

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

1. Add to `data/monsters.json` with proper ECS component structure
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
- `game_serializer.cpp`: Save/load serialization logic
- `db/database_manager.cpp`: PostgreSQL connection management
- `db/save_game_repository.cpp`: Cloud save operations
- `services/cloud_save_service.cpp`: Save sync orchestration
- `docs/project/notes.md`: User's notes (DO NOT EDIT)
