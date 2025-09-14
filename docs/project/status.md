# Veyrm - Current State Documentation

## Last Updated: 2025-09-14

## Project Status

**Veyrm has reached MVP status (v1.0.0-MVP)!** The game is now feature-complete with all core systems implemented, tested, and documented. It features procedural dungeon generation, monster AI, a complete d20-based combat system, full inventory management, item system with collectibles and treasure, and a comprehensive save/load system.

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

### Phase 10: Item System ✅
- **10.1 Item Entity**: Complete item foundation system
  - Data-driven item definitions (items.json)
  - Item spawning and management
  - Item pickup with 'g' key
  - Gold collection system
  - 12 initial item types

### Phase 11: Inventory System ✅
- **11.1 Storage System**: 26-slot inventory with stacking
- **11.2 Inventory UI**: Full UI with navigation and actions
- **11.3 Item Usage**: Use, drop, and examine functionality

### Phase 12: Save/Load System ✅
- **12.1 Persistence**: Complete save/load implementation
  - 9 save slots with visual management
  - Seed-based map regeneration (98.6% smaller files)
  - Separate S and L keybindings
  - JSON-based save format

## Current Capabilities

### Gameplay Features
- Explore procedurally generated dungeons
- Fight monsters using tactical bump combat
- Gain experience from defeating enemies
- Collect items and treasure
- Pick up items with 'g' key
- Manage 26-slot inventory
- Use items for effects (healing potions)
- Drop and examine items
- Accumulate gold currency
- Save/Load with 9 slots
- Navigate with 8-directional movement
- Save and load game progress
- View message log of events
- Track HP, position, and game stats

### Technical Features
- YAML configuration system
- Enhanced multi-category logging system with separate log files
- Data-driven content (monsters.json, **items.json**)
- Fullscreen terminal UI with FTXUI
- Cross-platform support (macOS, Linux, Windows)
- Extensive unit test coverage (**135 tests, all passing**)
- Interactive door system with open/close mechanics
- **Item factory pattern for data-driven items**
- **Item manager for world item lifecycle**

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

## Recent Changes (v1.0.0-MVP - Complete Game)

### MVP Completion
- **Save/Load System** - Complete persistence
  - 9 save slots with management UI
  - Seed-based map regeneration
  - 98.6% smaller save files (7-8KB)
  - Entity preservation during load
- **Inventory System** - Full inventory management
  - 26-slot storage with stacking
  - Complete UI with navigation
  - Use, drop, examine actions
  - Integration with items and combat
- **Item System** - Complete item framework
  - 12 item types with effects
  - Healing potions functional
  - Gold collection system

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

## MVP Complete!

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

**Current Version: 1.0.0-MVP (Minimum Viable Product)**

🎉 **MVP COMPLETE!** 🎉

- Complete roguelike gameplay loop
- All core systems implemented
- 135 tests with 100% pass rate
- Full documentation
- Ready for future expansion

### Key MVP Features:
- Procedural dungeon generation
- Turn-based gameplay
- Monster AI with multiple behaviors
- Combat system with d20 mechanics
- Item and inventory systems
- Save/Load with 9 slots
- Complete terminal UI