# Veyrm

A modern roguelike game inspired by Angband, written in C++23 with a terminal-based UI. Descend through the Spiral Vaults beneath Veyrmspire to shatter the last shard of a dead god's crown.

![Version](https://img.shields.io/badge/version-0.0.4-blue)
![C++](https://img.shields.io/badge/C%2B%2B-23-brightgreen)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey)
![Database](https://img.shields.io/badge/PostgreSQL-16-blue)
![Tests](https://img.shields.io/badge/tests-148%20passing-brightgreen)

## 🎮 Features

### Current (v0.0.4) - PostgreSQL Integration Complete ✅

#### 🛢️ **Database & Cloud Features**
- **PostgreSQL Integration** - Complete database layer with connection pooling
- **User Authentication** - Secure registration, login, and session management
- **Cloud Save System** - Multi-slot save games stored in PostgreSQL
- **Save Synchronization** - Automatic cloud sync with conflict detection
- **User Profiles** - Persistent player profiles and statistics
- **Login Screen** - Full authentication UI with registration and login

#### 🎮 **Game Features**
- **ECS Architecture** - Complete Entity Component System implementation
- **Item System** - Complete item entity system with collectibles and treasure
- **Item Pickup** - Press 'g' to get items, automatic gold collection
- **Combat System** - d20-based tactical bump-to-attack with critical hits
- **Monster AI** - State-based AI with pathfinding (IDLE, ALERT, HOSTILE, FLEEING)
- **Monster Spawning** - Intelligent spawn system with room preference (95% in rooms)
- **Dynamic Spawning** - Monsters spawn during gameplay based on configurable timers
- **Monster System** - Data-driven monsters with JSON configuration
- **Player Entity** - Full player stats, leveling, movement, and gold tracking
- **Inventory System** - Full 26-slot inventory with use, drop, and examine
- **Save/Load** - 9 save slots with seed-based map regeneration

#### 🎨 **UI & Rendering**
- **Configuration** - JSON-based game configuration with environment overrides
- **Lit Rooms** - Angband-style permanently illuminated rooms
- **Fullscreen UI** - Dynamic fullscreen mode with responsive layout
- **Three-Panel Layout** - Map, status bar, and message log
- **Unicode Tiles** - Beautiful characters for walls (█), floors (·), items (!,$), etc.
- **FOV System** - Symmetric shadowcasting field of view
- **Map Memory** - Remember explored areas
- **Map System** - Procedural dungeon generation with validation
- **Movement** - 8-directional movement with collision detection
- **Rendering** - Layered rendering (terrain, items, monsters, player)
- **Turn System** - Action-based timing system
- **Message Log** - Scrollable message history with color coding
- **Status Bar** - HP with color coding, position, time, depth, and gold display

### Recently Added (PostgreSQL Integration Phase)

- **🛢️ PostgreSQL Database** - Complete integration with connection pooling
- **🔐 User Authentication** - Registration, login, session management
- **☁️ Cloud Save System** - PostgreSQL-backed save/load with 9 slots
- **📊 User Profiles** - Persistent player data and statistics
- **🔄 Save Synchronization** - Automatic cloud sync with conflict detection
- **🖥️ Login Screen** - Authentication UI integrated into game flow
- **⚡ Repository Pattern** - Clean separation of data access layer
- **🧪 Comprehensive Testing** - 148 test cases covering all database features

### Planned

- **Experience System** - Leveling and skill progression
- **Multiple Levels** - Stairs functionality to descend deeper
- **More Items** - Weapons, armor, scrolls, and magic items
- **Quests** - Story objectives and NPC interactions
- **🏆 Leaderboards** - Global high scores and achievements
- **📈 Analytics** - Gameplay telemetry and performance metrics

## 🚀 Quick Start

### Prerequisites

- C++23 compatible compiler (GCC 12+, Clang 15+, MSVC 2022+)
- CMake 3.25 or higher
- Git
- **Docker & Docker Compose** (for PostgreSQL database)
- **PostgreSQL 16** (if not using Docker)

### Build & Run

```bash
# Clone the repository
git clone https://github.com/yourusername/veyrm.git
cd veyrm

# Setup environment (copy and customize)
cp .env.example .env

# Start PostgreSQL database (Docker)
docker-compose up -d postgres

# Initialize database
./build.sh db create  # Create tables
./build.sh db load    # Load initial data

# Build and run
./build.sh         # Interactive menu
./build.sh build   # Build in debug mode
./build.sh run     # Run the game

# Or manually with CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
./bin/veyrm
```

### First Time Setup

1. **Database Setup:**
   ```bash
   # Copy environment template
   cp .env.example .env

   # Edit .env file - change passwords!
   # POSTGRES_PASSWORD=your_secure_password_here

   # Start database
   docker-compose up -d postgres

   # Initialize schema
   ./build.sh db create
   ./build.sh db load
   ```

2. **Verify Setup:**
   ```bash
   # Check database connection
   ./build.sh db status

   # Run all tests (should show 148 passing)
   ./build.sh test
   ```

3. **Run Game:**
   ```bash
   ./build.sh run
   ```

### Map Selection

Run with different map types:

```bash
# Command line (runs from project root)
./build/bin/veyrm --map procedural  # Procedural dungeon (default)
./build/bin/veyrm --map room        # Single room
./build/bin/veyrm --map dungeon     # Multi-room dungeon
./build/bin/veyrm --map corridor    # Corridor test
./build/bin/veyrm --map arena       # Combat arena
./build/bin/veyrm --map stress      # Stress test

# Via build script
./build.sh run                      # Interactive selection
./build.sh run arena                # Direct selection
```

### Configuration

Game settings are stored in `config.yml`. Command-line arguments override config values:

```yaml
game:
  default_map: procedural
  
map_generation:
  procedural:
    width: 198         # Angband standard
    height: 66         # Angband standard
    lit_room_chance: 0.95
    
player:
  starting_hp: 20
  starting_attack: 5
  starting_defense: 2
  fov_radius: 10
```

## 🎯 Controls

| Key | Action |
|-----|--------|
| **Arrow Keys** | Move in 4 directions |
| **5 (numpad)** | Wait one turn |
| **.** | Wait one turn |
| **i** | Open inventory |
| **g** | Get/pickup item |
| **u** | Use item |
| **D** | Drop item |
| **E** | Examine item |
| **S** | Save game |
| **L** | Load game |
| **o** | Open/close doors |
| **?** | Show help |
| **q** / **Q** | Quit to menu |
| **ESC** | Cancel/return to previous screen |
| **Enter** | Confirm action |

*Note: Vi-style movement (hjkl) and diagonal movement (yubn) are not currently implemented.*

## 🏗️ Architecture

### Modern C++23 Architecture with ECS and PostgreSQL

```
veyrm/
├── include/            # Header files
│   ├── ecs/           # Entity Component System
│   │   ├── game_world.h    # Central ECS world manager
│   │   ├── entity_factory.h # Entity creation
│   │   ├── components/     # All game components
│   │   └── systems/       # All game systems
│   ├── db/            # Database layer
│   │   ├── database_manager.h     # Connection pooling
│   │   ├── repository_base.h      # Repository pattern
│   │   ├── save_game_repository.h # Save/load operations
│   │   └── player_repository.h    # User management
│   ├── auth/          # Authentication system
│   │   ├── authentication_service.h
│   │   ├── login_models.h
│   │   └── validation_service.h
│   ├── services/      # Business logic services
│   │   └── cloud_save_service.h
│   ├── ui/            # User interface (FTXUI)
│   │   ├── screens/       # Game screens
│   │   ├── components/    # UI components
│   │   └── controllers/   # MVC controllers
│   └── game/          # Core game logic
├── src/               # Implementation files
│   ├── ecs/          # ECS implementation
│   ├── db/           # Database implementation
│   ├── auth/         # Authentication implementation
│   ├── services/     # Services implementation
│   ├── ui/           # UI implementation
│   └── main.cpp      # Entry point
├── tests/             # Comprehensive test suite (148 tests)
│   ├── test_database_*.cpp      # Database integration tests
│   ├── test_ecs_*.cpp          # ECS tests
│   ├── test_auth_*.cpp         # Authentication tests
│   └── test_cloud_*.cpp        # Cloud save tests
├── docs/              # Documentation
│   ├── database/          # Database documentation & ERD
│   ├── getting-started/   # Quick start guides
│   ├── guides/           # How-to guides
│   ├── reference/        # API & command reference
│   └── project/          # Project information
├── data/              # Game data (JSON)
│   ├── monsters.json     # Monster definitions
│   └── items.json       # Item definitions
├── init/              # Database initialization
│   └── *.sql            # Schema and data scripts
├── docker-compose.yml # PostgreSQL setup
├── .env.example      # Environment template
└── build.sh          # Build helper script
```

### Key Design Patterns

- **Entity Component System (ECS):** Modern game architecture
- **Repository Pattern:** Clean database access layer
- **Dependency Injection:** Loose coupling between components
- **MVC Pattern:** Clear separation of UI concerns
- **Connection Pooling:** Efficient database resource management
- **Strategy Pattern:** Configurable game systems

## 📚 Documentation

### Quick Links

- **[Getting Started](docs/getting-started/README.md)** - Installation and first game
- **[Database Setup](docs/database/postgres-setup.md)** - PostgreSQL configuration
- **[Database ERD](docs/database/database-erd.md)** - Schema documentation
- **[Player Guide](docs/guides/player/README.md)** - Gameplay mechanics and strategies
- **[Developer Guide](docs/guides/developer/README.md)** - Contributing to Veyrm
- **[Project Status](docs/project/status.md)** - Current development state
- **[API Reference](docs/reference/api/README.md)** - Code documentation

### Additional Resources

- **[Build Commands](docs/reference/commands/build-script.md)** - Build.sh command reference
- **[Architecture](docs/guides/developer/architecture.md)** - System architecture
- **[Database Integration](docs/database/postgres-integration-plan.md)** - Implementation details
- **[Authentication System](docs/database/phase2-authentication-final-report.md)** - Auth implementation
- **[World Design](docs/design/world/README.md)** - Game lore and setting
- **[Changelog](docs/project/changelog.md)** - Version history

## 🛠️ Development

### Current Phase: PostgreSQL Integration Complete ✅

Complete database integration with user authentication, cloud saves, and comprehensive testing. All 148 test cases passing with 1942+ assertions.

#### Recent Achievements
- ✅ PostgreSQL database layer with connection pooling
- ✅ User authentication and session management
- ✅ Cloud save system with 9-slot storage
- ✅ Repository pattern for clean data access
- ✅ Login/registration UI integrated
- ✅ Comprehensive test suite (database, auth, cloud saves)
- ✅ Error handling and graceful degradation

### Build Modes

```bash
# Debug build (with symbols, assertions)
./build.sh build debug

# Release build (optimized)
./build.sh build release

# Run tests
./build.sh test

# Clean build
./build.sh clean build
```

### 🛢️ Database Setup (PostgreSQL)

Veyrm includes a complete PostgreSQL integration for persistent game data including user accounts, cloud saves, and analytics.

#### ✅ What Database Provides

- **User Authentication:** Secure registration and login system
- **Cloud Saves:** Multi-slot save games synced to database
- **User Profiles:** Persistent player statistics and preferences
- **Session Management:** Secure token-based authentication
- **Analytics Ready:** Schema for leaderboards and telemetry

#### 🐳 Quick Setup (Docker - Recommended)

1. **Environment Setup:**
   ```bash
   cp .env.example .env
   # Edit .env - change POSTGRES_PASSWORD!
   ```

2. **Start Database:**
   ```bash
   docker-compose up -d postgres  # Start PostgreSQL
   ./build.sh db create           # Create tables
   ./build.sh db load            # Load initial data
   ```

3. **Verify Connection:**
   ```bash
   ./build.sh db status          # Check connection
   ./build.sh test               # Run all tests (148 should pass)
   ```

#### 📋 Database Management Commands

```bash
# Database operations
./build.sh db create     # Create all tables and indexes
./build.sh db load       # Load game data (monsters, items, etc.)
./build.sh db status     # Check connection and table counts
./build.sh db reset      # Drop and recreate everything

# Docker operations
docker-compose up -d postgres          # Start database
docker-compose logs -f postgres        # View logs
docker-compose down                    # Stop database
```

#### 🔧 Advanced Setup

- **Docker with PgAdmin:** `docker-compose --profile tools up -d`
- **Manual PostgreSQL:** See [Database Setup Guide](docs/database/postgres-setup.md)
- **Schema Documentation:** See [Database ERD](docs/database/database-erd.md)

#### 🚫 Without Database

The game gracefully degrades when PostgreSQL is unavailable:
- Authentication bypassed (direct to main menu)
- Saves stored locally in files
- All core gameplay features work normally

### Testing

```bash
# Run all tests (148 test cases, 1942+ assertions)
./build.sh test

# Run specific test categories
./build/bin/veyrm_tests "[database]"     # Database integration tests
./build/bin/veyrm_tests "[auth]"         # Authentication tests
./build/bin/veyrm_tests "[ecs]"          # Entity Component System tests
./build/bin/veyrm_tests "[entity]"       # Entity tests
./build/bin/veyrm_tests "[cloud]"        # Cloud save tests

# Run with automated input for gameplay testing
./build.sh keys "qqqq\n"

# Dump mode for debugging frame-by-frame
./build.sh dump

# Performance testing
./build/bin/veyrm_tests "[performance]"
```

#### Test Coverage

- **Database Integration:** User auth, save/load, repositories
- **ECS Architecture:** Components, systems, entity management
- **Game Logic:** Map generation, combat, inventory
- **UI Components:** Screens, forms, navigation
- **Performance:** Large save files, concurrent operations
- **Error Handling:** Network failures, invalid data

### Terminal Issues

If the terminal becomes unresponsive:

```bash
./build.sh reset
```

## 🎨 Features in Detail

### Map Generation

- **5 Test Map Types**: Room, Dungeon, Corridor, Arena, Stress Test
- **Validation System**: Ensures connectivity and playability
- **Proper Corridors**: Walls around corridors with automatic doorways

### Rendering System

- **Viewport-Based**: Handles large maps efficiently
- **Adaptive Colors**: Works on both dark and light terminals
- **Unicode Support**: Wall connections for better visuals (future)

### Turn System

- **Action-Based**: Different actions take different amounts of time
- **Speed System**: Fast (50%), Normal (100%), Slow (150%)
- **Turn Counter**: Tracks game progression

## 🤝 Contributing

This is currently a personal project, but feel free to:

- Report issues
- Suggest features
- Fork and experiment

## 📜 License

This project is currently under development. License TBD.

## 🏷️ Version History

- **v0.0.4** - PostgreSQL Integration Complete (current)
  - ✅ Complete database layer with connection pooling
  - ✅ User authentication and session management
  - ✅ Cloud save system with 9-slot storage
  - ✅ Repository pattern implementation
  - ✅ Login/registration UI
  - ✅ 148 test cases covering all features
- **v0.0.3** - Entity Component System
  - ✅ Complete ECS architecture
  - ✅ Item system and inventory
  - ✅ Combat and monster AI
  - ✅ Save/load functionality
- **v0.0.2** - Core Game Systems
  - ✅ Map generation and validation
  - ✅ Rendering and UI systems
  - ✅ Turn-based gameplay
- **v0.0.1** - Project Foundation
  - ✅ C++23 build system
  - ✅ FTXUI integration
  - ✅ Basic architecture

## 🔮 Roadmap

### Near Term (Phase 3-4)

- [x] Entity system (player, entities, manager)
- [ ] Monster entities with basic AI
- [ ] Basic combat system
- [ ] Item pickup and inventory

### Mid Term (Phase 5-7)

- [ ] Field of view system
- [ ] Save/load functionality
- [ ] Character stats and progression
- [ ] Status effects

### Long Term (Phase 8+)

- [ ] Procedural dungeon generation
- [ ] Multiple dungeon levels
- [ ] Boss encounters
- [ ] Victory conditions

## 🐛 Known Issues

- Terminal may need reset after crashes (use `./build.sh reset`)
- Some terminals may not display colors correctly (use `--no-color` flag when implemented)

## 💡 Tips

- Use `./build.sh` for the interactive menu - it's the easiest way to build and run
- Test different map types to see the variety of layouts
- The message log shows important information about the game state
- Check `DOC/PHASES/` for detailed information about each development phase

## 📧 Contact

Project Repository: [github.com/yourusername/veyrm](https://github.com/yourusername/veyrm)

---

*Veyrm - Descend. Survive. Shatter the Crown.*
