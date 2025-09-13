# Veyrm - Current State Documentation

## Last Updated: 2025-01-14

## Project Status

Veyrm is a modern roguelike game in active development, currently at Phase 9 (Combat System) completion with recent enhancements to the logging system and door mechanics. The game features procedural dungeon generation, monster AI, and a complete d20-based combat system.

## Completed Phases

### Phase 1-6: Foundation ✅
- Basic game loop and terminal UI
- Map generation with rooms and corridors
- Player movement and collision detection
- Field of view (FOV) with shadowcasting
- Save/load system with JSON serialization
- Menu system with navigation

### Phase 7: UI System ✅
- Message log for game events
- Status bar with player stats
- Three-panel responsive layout
- Fullscreen terminal support

### Phase 8: Monster System ✅
- **8.1 Monster Entity**: Data-driven monster design
- **8.2 Monster Spawning**: Dynamic population management
- **8.3 Basic AI**: State-based AI with pathfinding

### Phase 9: Combat System ✅
- **9.1 Combat Stats**: d20 mechanics with critical hits
- **9.2 Bump Combat**: Player melee attacks
- **9.3 Death Handling**: Complete death states

## Current Capabilities

### Gameplay Features
- Explore procedurally generated dungeons
- Fight monsters using tactical bump combat
- Gain experience from defeating enemies
- Navigate with 8-directional movement
- Save and load game progress
- View message log of events
- Track HP, position, and game stats

### Technical Features
- YAML configuration system
- Enhanced multi-category logging system with separate log files
- Data-driven content (monsters.json)
- Fullscreen terminal UI with FTXUI
- Cross-platform support (macOS, Linux, Windows)
- Extensive unit test coverage (126 tests, all passing)
- Interactive door system with open/close mechanics

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

### Current Stats (Configurable)
- **Player HP**: 50
- **Player Attack**: 8
- **Player Defense**: 5
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

## Recent Changes (v0.9.5 - Visualization Tools)

### Visualization Tools Added
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

## Next Steps (Phase 10: Item System)

### 10.1 Item Entity
- Create Item class
- Add item types (potions, scrolls)
- Load items from JSON

### 10.2 Inventory
- Add inventory to player
- Implement pickup/drop
- Create inventory UI

### 10.3 Item Usage
- Implement item effects
- Add use command
- Test item interactions

## Known Issues

1. **Performance**: No current performance issues
2. **Bugs**: No critical bugs remaining
3. **Limitations**:
   - No ranged combat
   - No item system yet
   - No magic/abilities
   - Single dungeon level only

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

Current Version: 0.9.5 (Visualization Tools)

- Gource video generation for repository history
- Class diagram generator with UML-style output
- Documentation of 12 additional visualization options
- All outputs to gitignored tmp/ directory

Previous: 0.9.4 (Post-Phase 9 Enhancements)

- Enhanced logging system with category separation
- Door system implementation
- Test suite fixes (all 126 tests passing)

Next Version: 0.10.1 (Phase 10.1: Item Entity)