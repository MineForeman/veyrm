# Veyrm

A modern roguelike game inspired by Angband, written in C++23 with a terminal-based UI. Descend through the Spiral Vaults beneath Veyrmspire to shatter the last shard of a dead god's crown.

![Version](https://img.shields.io/badge/version-1.0.0--MVP-blue)
![C++](https://img.shields.io/badge/C%2B%2B-23-brightgreen)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey)

## 🎮 Features

### Current (v1.0.0-MVP)
- **Item System** - Complete item entity system with collectibles and treasure
- **Item Pickup** - Press 'g' to get items, automatic gold collection
- **Combat System** - d20-based tactical bump-to-attack with critical hits
- **Monster AI** - State-based AI with pathfinding (IDLE, ALERT, HOSTILE, FLEEING)
- **Monster Spawning** - Intelligent spawn system with room preference (95% in rooms)
- **Dynamic Spawning** - Monsters spawn during gameplay based on configurable timers
- **Monster System** - Data-driven monsters with JSON configuration
- **Entity System** - Flexible component-based entity architecture
- **Player Entity** - Full player stats, leveling, movement, and gold tracking
- **Configuration** - YAML-based game configuration (config.yml)
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

### Recently Added
- **Inventory System** - Full 26-slot inventory with use, drop, and examine
- **Item Effects** - Healing potions restore HP
- **Save/Load** - 9 save slots with seed-based map regeneration

### Planned
- **Experience System** - Leveling and skill progression
- **Multiple Levels** - Stairs functionality to descend deeper
- **More Items** - Weapons, armor, scrolls, and magic items
- **Quests** - Story objectives and NPC interactions

## 🚀 Quick Start

### Prerequisites
- C++23 compatible compiler (GCC 12+, Clang 15+, MSVC 2022+)
- CMake 3.25 or higher
- Git

### Build & Run

```bash
# Clone the repository
git clone https://github.com/yourusername/veyrm.git
cd veyrm

# Quick build and run
./build.sh         # Interactive menu
./build.sh build   # Build in debug mode
./build.sh run     # Run the game

# Or manually with CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
./bin/veyrm
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

```
veyrm/
├── include/        # Header files
│   ├── game_state.h
│   ├── map.h
│   ├── tile.h
│   └── ...
├── src/           # Implementation files
│   ├── main.cpp
│   ├── game_manager.cpp
│   ├── map_generator.cpp
│   └── ...
├── docs/          # Documentation
│   ├── getting-started/  # Quick start guides
│   ├── guides/           # How-to guides
│   ├── reference/        # API & command reference
│   └── project/          # Project information
├── tests/         # Unit tests
├── data/          # Game data (monsters, items)
└── build.sh       # Build helper script
```

## 📚 Documentation

### Quick Links
- **[Getting Started](docs/getting-started/README.md)** - Installation and first game
- **[Player Guide](docs/guides/player/README.md)** - Gameplay mechanics and strategies
- **[Developer Guide](docs/guides/developer/README.md)** - Contributing to Veyrm
- **[Project Status](docs/project/status.md)** - Current development state
- **[API Reference](docs/reference/api/README.md)** - Code documentation
### Additional Resources
- **[Build Commands](docs/reference/commands/build-script.md)** - Build.sh command reference
- **[Architecture](docs/guides/developer/architecture.md)** - System architecture
- **[World Design](docs/design/world/README.md)** - Game lore and setting
- **[Changelog](docs/project/changelog.md)** - Version history

## 🛠️ Development

### Current Phase: 3.1 Complete - Entity Base ✅
Entity system implemented with Player class, EntityManager, and movement integration. All tests passing (57 test cases, 404 assertions).

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

### Testing

```bash
# Run all unit tests (57 test cases)
./build.sh test

# Run specific test
./build/bin/veyrm_tests "[entity]"

# Run with automated input for gameplay testing
./build.sh keys "qqqq\n"

# Dump mode for debugging frame-by-frame
./build.sh dump
```

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

- **v0.3.1** - Entity system with Player and EntityManager (current)
- **v0.2.3** - Map generation system with 5 test maps
- **v0.2.2** - Complete rendering system with viewport
- **v0.2.1** - Tile system implementation
- **v0.1.3** - Main game loop
- **v0.1.2** - Turn system
- **v0.0.3** - Basic FTXUI window
- **v0.0.2** - Dependencies setup
- **v0.0.1** - Project initialization

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