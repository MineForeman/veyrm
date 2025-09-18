# Veyrm Features Documentation

## Current Features (v0.8.1)

### Monster System

- **Data-Driven Design**: Monsters defined in JSON files for easy modification
- **MonsterFactory**: Singleton pattern for efficient monster template management
- **Combat Stats**: HP, attack, defense, speed, XP value
- **Behavior Flags**:
  - `aggressive`: Whether monster attacks on sight
  - `can_open_doors`: Can pursue through doors
  - `can_see_invisible`: Special vision capabilities
- **Threat Levels**: Letter-based ranking system (a-f)
- **Initial Monsters**:
  - Gutter Rat (threat: a)
  - Cave Spider (threat: b)
  - Kobold (threat: b)
  - Orc Rookling (threat: c)
  - Zombie (threat: d)

### Configuration System

- **YAML Configuration**: Comprehensive `config.yml` for game settings
- **Command-Line Overrides**: CLI arguments take precedence over config file
- **Configurable Elements**:
  - Map dimensions (default: 198x66 Angband standard)
  - Room generation parameters (min/max rooms and sizes)
  - Lit room probability (default: 95%)
  - Player starting stats (HP: 20, Attack: 5, Defense: 2)
  - FOV radius (default: 10)
  - Monster behavior settings
  - Keybindings (arrow keys only, no vi-keys by default)

### Lit Rooms (Angband-Style)

- **Permanent Illumination**: 95% of rooms are permanently lit (configurable)
- **Full Room Visibility**: Entire room revealed when player enters
- **Memory Persistence**: Lit rooms remain bright in memory
- **Strategic Element**: Risk/reward tradeoffs for exploration

### Build System Enhancements

- **Binary Statistics**: Display size, build time, and mode in menu
- **Streamlined UI**: Removed pause prompts for better flow
- **Project Root Execution**: All binaries run from project root
- **Flexible Data Paths**: `--data-dir` option for custom data directories

### Field of View System

- **Symmetric Shadowcasting**: Accurate 8-octant FOV calculation
- **Configurable Radius**: Default 10 tiles (adjustable in config)
- **Map Memory**: Explored areas remain visible but dimmed
- **Lit Room Integration**: Special handling for permanently lit areas

### UI Layout System

- **Fullscreen Mode**: Dynamic terminal sizing
- **Three-Panel Layout**:
  - Main map viewport
  - Status bar with HP, position, time, depth
  - Message log with 5-line scrollable history
- **Responsive Design**: Adapts to terminal size changes

### Map Generation

- **Procedural Dungeons**: Random room and corridor generation
- **Multiple Map Types**:
  - Procedural (default)
  - Test Room
  - Test Dungeon
  - Corridor Test
  - Combat Arena
  - Stress Test
- **Validation System**: Ensures connectivity and playability
- **Stair Placement**: Automatic placement at farthest point

### Entity System

- **Component Architecture**: Flexible entity-component design
- **Entity Types**: Player, Monster, Item (planned)
- **EntityManager**: Centralized entity lifecycle management
- **Collision Detection**: Blocking and non-blocking entities

### Player System

- **Configurable Stats**: Starting HP, attack, defense from config
- **Movement**: 8-directional with collision detection
- **Experience System**: Framework for leveling (not yet active)
- **Inventory**: Capacity system (implementation pending)

### Message System

- **Color-Coded Messages**: Different colors for message types
- **Message History**: 100-message buffer
- **Scrollable Display**: 5 visible lines with history access
- **Message Categories**: Combat, movement, system, debug

### Turn System

- **Action-Based**: Each action consumes time
- **Turn Counter**: Tracks total game turns
- **World Time**: In-game time display
- **Speed System**: Framework for variable action speeds

## Planned Features

### Monster AI (Phase 8.2)

- Basic pathfinding
- Aggression radius
- Door interaction
- Flee behavior

### Combat System (Phase 9)

- Bump-to-attack mechanics
- Damage calculations
- Death handling
- Combat messages

### Item System (Phase 10)

- Weapons and armor
- Consumables (potions, scrolls)
- Inventory management
- Item identification

### Save/Load System (Phase 11)

- JSON serialization
- Game state persistence
- Multiple save slots
- Death saves

### Advanced Features

- Magic system
- Character classes
- Skill trees
- Quest system
- Boss encounters
- Multiple dungeon levels

## Technical Features

### Testing

- Comprehensive test suite (110+ tests)
- Unit and integration tests
- 1800+ assertions
- Catch2 framework

### Dependencies

- **FTXUI**: Terminal UI framework
- **nlohmann/json**: JSON parsing
- **rapidyaml**: YAML configuration
- **Catch2**: Testing framework

### Build System

- CMake 3.25+ with FetchContent
- C++23 standard
- Cross-platform support
- Debug and Release configurations

### Code Architecture

- Modern C++23 features
- RAII and smart pointers
- Singleton patterns where appropriate
- Virtual polymorphism for entities
- Data-driven design patterns
