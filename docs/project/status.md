# Veyrm - Current State Documentation

## Last Updated: 2025-01-18

## Project Status

**Veyrm has completed PostgreSQL Integration (v0.0.4)!** The game now features a complete database layer with user authentication, cloud saves, and comprehensive testing. This represents a major architecture upgrade from local-only saves to a full cloud-enabled system with multi-user support.

### Current Version: v0.0.4 - PostgreSQL Integration Complete ✅

- **Database Integration**: Complete PostgreSQL layer with connection pooling
- **User Authentication**: Secure registration, login, and session management
- **Cloud Save System**: Multi-slot save games stored in PostgreSQL
- **Repository Pattern**: Clean data access layer separation
- **Comprehensive Testing**: 148 test cases covering all database features
- **Production Ready**: Security best practices and performance optimization

## Completed Phases

### Phase 1: Core Game Foundation ✅
- **ECS Architecture**: Complete Entity Component System implementation
- **Game Loop**: Turn-based game loop with FTXUI
- **Map System**: Procedural dungeon generation with validation
- **Player Systems**: Movement, FOV, collision detection
- **Basic UI**: Three-panel layout with message log and status bar

### Phase 2: Game Content ✅
- **Monster System**: Data-driven monsters with JSON configuration
- **AI System**: State-based AI (IDLE, ALERT, HOSTILE, FLEEING)
- **Combat System**: d20-based tactical bump-to-attack combat
- **Item System**: Complete item framework with 12+ item types
- **Inventory System**: 26-slot inventory with use/drop/examine

### Phase 3: PostgreSQL Integration ✅
- **Database Layer**: Complete PostgreSQL integration with connection pooling
- **Authentication System**: User registration, login, session management
- **Repository Pattern**: Clean data access layer (SaveGameRepository, PlayerRepository)
- **Cloud Save System**: Multi-slot cloud saves with conflict detection
- **Security**: SHA256 password hashing, session tokens, SQL injection prevention
- **Testing**: 148 comprehensive test cases covering all database operations

### Phase 4: Production Readiness ✅
- **Error Handling**: Graceful degradation when database unavailable
- **Performance**: Connection pooling, prepared statements, optimized indexes
- **Documentation**: Complete ERD, integration guide, and architecture docs
- **CI/CD Ready**: Docker setup, environment configuration, migration system

## Current Capabilities

### Database & Cloud Features
- **User Registration**: Secure account creation with email verification
- **User Authentication**: Login/logout with session management
- **Cloud Saves**: 9-slot save system synchronized to PostgreSQL
- **Save Synchronization**: Automatic cloud sync with conflict detection
- **Session Management**: Secure token-based authentication with refresh tokens
- **Multi-User Support**: Complete user isolation and data protection

### Gameplay Features
- **Complete ECS Gameplay**: Full Entity Component System implementation
- **Procedural Dungeons**: Dynamic map generation with room/corridor layouts
- **Advanced Combat**: d20-based tactical bump-to-attack with critical hits
- **Monster AI**: State-based AI (IDLE, ALERT, HOSTILE, FLEEING) with pathfinding
- **Item System**: 12+ item types with effects (healing potions, gold, etc.)
- **Inventory Management**: 26-slot inventory with use/drop/examine
- **Dynamic Spawning**: Monsters and items spawn during gameplay
- **FOV & Memory**: Field of view with explored area memory
- **Save/Load**: Local and cloud save options with 9 slots

### Technical Features
- **PostgreSQL Integration**: Complete database layer with connection pooling
- **Repository Pattern**: Clean data access layer with proper separation
- **Security**: SHA256 password hashing, SQL injection prevention
- **JSON Configuration**: Game settings with environment variable overrides
- **Comprehensive Testing**: 148 test cases covering all systems
- **Docker Support**: Complete PostgreSQL setup with Docker Compose
- **Error Handling**: Graceful degradation when database unavailable
- **Performance**: Optimized queries, indexes, and connection management
- **Cross-Platform**: Full support for macOS, Linux, Windows

## Active Systems

### Combat System

- **Mechanics**: d20 attack rolls vs defense values
- **Damage**: Base damage minus defense, minimum 1
- **Critical Hits**: Natural 20 deals double damage
- **Critical Misses**: Natural 1 always misses
- **Death**: Entities removed at 0 HP

### Monster AI States

1. **IDLE**: Default state, random movement
2. **ALERT**: Heard combat, moves toward sound
3. **HOSTILE**: Can see player, actively pursues
4. **FLEEING**: Low health (<25%), runs away
5. **RETURNING**: Lost player, returns to territory

### Item System

- **Item Types**: Potions, Scrolls, Weapons, Armor, Food, Gold, Misc
- **Current Items**: 12 types defined in items.json
- **Spawn Rate**: 5-10 items per level, 3-6 gold piles
- **Pickup**: Press 'g' to get items at current position
- **Gold**: Automatically added to player wealth
- **Rendering**: Items show appropriate symbols and colors

### Current Stats (Configurable)

- **Player HP**: 50
- **Player Attack**: 8
- **Player Defense**: 5
- **Player Gold**: 0 (starting)
- **FOV Radius**: 10 tiles
- **Monster Spawn Rate**: 100 turns

## File Structure

```
veyrm/
├── src/              # Source files
├── include/          # Header files
├── tests/           # Unit tests
├── data/            # Game data (JSON)
├── DOC/             # Documentation
│   ├── PHASES/      # Phase documentation
│   ├── WORLD/       # World lore
│   └── MVP/         # MVP specs
├── logs/            # Debug logs
├── build.sh         # Build script
├── config.yml       # Game configuration
└── CMakeLists.txt   # Build configuration
```

## Recent Changes (v0.0.4 - PostgreSQL Integration)

### Database Integration Complete ✅

- **PostgreSQL Layer** - Complete database integration
  - Connection pooling with configurable min/max connections
  - Automatic reconnection and error handling
  - Transaction support and thread-safety
- **Authentication System** - Secure user management
  - User registration with email verification
  - SHA256 password hashing with unique salts
  - Session-based authentication with refresh tokens
  - Login history tracking and security features
- **Repository Pattern** - Clean data access
  - SaveGameRepository for cloud save operations
  - PlayerRepository for user and session management
  - Base repository for common database operations
- **Cloud Save System** - PostgreSQL-backed saves
  - 9-slot save system with metadata
  - Automatic save synchronization
  - Conflict detection and resolution
  - Save backups and version tracking
- **Comprehensive Testing** - 148 test cases
  - Database integration tests
  - Authentication and session tests
  - Cloud save functionality tests
  - Performance and error handling tests
- **Production Features** - Enterprise-ready
  - Docker Compose setup for development
  - Environment-based configuration
  - Security best practices
  - Performance optimization

### Previous Update (v0.9.5 - Visualization Tools)

- **Gource Video Generation** - Animated repository history visualization
  - Creates MP4 video of development timeline
  - Shows file changes and developer contributions
  - Configurable speed and duration settings
  - Output to tmp/veyrm-gource.mp4 (gitignored)
- **Class Diagram Generator** - Visual architecture documentation
  - UML-style class diagrams with Graphviz
  - Color-coded subsystems and relationships
  - Shows inheritance, composition, and usage patterns
  - Outputs SVG and PNG formats
- **Documentation** of 12 future visualization options
  - Comprehensive guide in DOC/VISUALIZATION_OPTIONS.md
  - Includes dependency graphs, git statistics, memory layout, and more

### Previous v0.9.4 Changes

#### Enhanced Logging System

- Separated logs into category-specific files
- Created logs/ directory for all log files
- Added 10 specialized log files (player, AI, combat, environment, etc.)
- Main debug log contains all events chronologically
- Fixed monster movements incorrectly appearing in player log

#### Door System Implementation

- Added interactive door tiles (open/closed states)
- 'o' key toggles adjacent doors
- Doors block movement and vision when closed
- Automatic door placement at room entrances
- Support for multiple door interactions per turn

#### Test Suite Improvements

- Fixed all failing tests (126 tests now passing)
- Updated tests to use Config values
- Fixed procedural dungeon stairs placement
- Resolved map validation issues with doors

## MVP Complete

The game has reached MVP status with all planned core features implemented:

- ✅ Map generation and rendering
- ✅ Player movement and FOV
- ✅ Monster AI and spawning
- ✅ Combat system
- ✅ Item system
- ✅ Inventory management
- ✅ Save/Load system
- ✅ Full UI with message log and status

## Future Development

### Immediate Priorities

- Bug fixes and optimization
- Code refactoring
- Documentation updates

### Future Features

- Multiple dungeon levels (stairs)
- Character progression
- More monsters and items
- Magic system
- Quests and NPCs

## Known Issues

1. **Performance**: No current performance issues
2. **Bugs**: No critical bugs remaining
3. **Limitations**:
   - No ranged combat
   - No character progression yet
   - No magic/abilities
   - Single dungeon level only (stairs not functional)

## Development Commands

```bash
# Build and run
./build.sh build
./build.sh run

# Testing
./build.sh test
./build.sh keys '\njjjhhq'  # Automated input
./build.sh dump              # Frame dump mode

# Check logs
tail -f logs/veyrm_debug.log

# Git workflow
git add -A && git commit -m "message"
git tag v0.9.3-death-handling
git push && git push --tags
```

## Configuration

Key settings in `config.yml`:

```yaml
player_starting_hp: 50
player_starting_attack: 8
player_starting_defense: 5
fov_radius: 10
monster_spawn_rate: 100
room_lit_chance: 95
```

## Dependencies

- **C++23**: Modern C++ features
- **CMake 3.25+**: Build system
- **FTXUI**: Terminal UI framework
- **nlohmann/json**: JSON parsing
- **rapidyaml**: YAML configuration
- **Catch2**: Unit testing

## Contact & Contributing

This is an active development project. All code changes should:

1. Include appropriate tests
2. Update relevant documentation
3. Follow existing code style
4. Pass all existing tests

## Version

**Current Version: v0.0.4 - PostgreSQL Integration Complete**

🎉 **DATABASE INTEGRATION COMPLETE!** 🎉

- Complete PostgreSQL database layer
- User authentication and cloud saves
- 148 tests with 100% pass rate
- Production-ready architecture
- Comprehensive documentation

### Key Database Features

- User registration and authentication
- Cloud save synchronization
- Session management with security
- Repository pattern for clean code
- Connection pooling for performance
- Docker setup for easy deployment
- Comprehensive test coverage
- Enterprise-level security
