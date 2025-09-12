# Veyrm

A modern roguelike game inspired by Angband, written in C++23 with a terminal-based UI. Descend through the Spiral Vaults beneath Veyrmspire to shatter the last shard of a dead god's crown.

![Version](https://img.shields.io/badge/version-0.3.1-blue)
![C++](https://img.shields.io/badge/C%2B%2B-23-brightgreen)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey)

## 🎮 Features

### Current (v0.3.1)
- **Entity System** - Flexible component-based entity architecture
- **Player Entity** - Full player stats, leveling, and movement
- **Terminal UI** - Clean, responsive interface using FTXUI
- **Map System** - Multiple test maps with validation
- **Movement** - 8-directional movement with collision detection
- **Rendering** - Viewport-based rendering with adaptive colors
- **Turn System** - Action-based timing system
- **Message Log** - Scrollable message history

### Planned
- **Monsters** - Basic AI and enemy entities
- **Combat** - Tactical bump-to-attack system
- **Field of View** - Symmetric shadowcasting
- **Items** - Weapons, armor, and consumables
- **Procedural Generation** - Random dungeons every game
- **Save/Load** - Persistent game state

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

Run with different test maps:

```bash
# Command line
./build/bin/veyrm --map room      # Single room
./build/bin/veyrm --map dungeon   # Multi-room dungeon (default)
./build/bin/veyrm --map corridor  # Corridor test
./build/bin/veyrm --map arena     # Combat arena
./build/bin/veyrm --map stress    # Stress test

# Via build script
./build.sh run                    # Interactive selection
./build.sh run arena              # Direct selection
```

## 🎯 Controls

| Key | Action |
|-----|--------|
| **Arrow Keys** | Move in 4 directions |
| **Numpad** | Move in 8 directions (with diagonals) |
| **hjkl** | Vi-style movement |
| **yubn** | Diagonal movement |
| **.** | Wait one turn |
| **i** | Open inventory (coming soon) |
| **?** | Show help |
| **q** | Quit to menu |
| **ESC** | Return to previous screen |

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
├── DOC/           # Documentation
│   ├── PHASES/    # Development phases
│   ├── WORLD/     # Game world lore
│   └── MVP/       # MVP specifications
├── tests/         # Unit tests
├── data/          # Game data (future)
└── build.sh       # Build helper script
```

## 📚 Documentation

- **[Implementation Plan](DOC/IMPLEMENTATION_PLAN.md)** - Development roadmap
- **[Build Script Guide](DOC/BUILD_SCRIPT.md)** - Using the build.sh script
- **[Phase Documentation](DOC/PHASES/)** - Detailed phase specifications
- **[World Lore](DOC/WORLD/)** - Game world and setting
- **[Technical Spec](DOC/SPEC.md)** - Technical specifications

## 🛠️ Development

### Current Phase: 3.2 Player on Map
Entity system complete! Now improving player rendering and entity-based display.

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
# Run unit tests
./build.sh test

# Run with automated input
./build/bin/veyrm --keys '\n\u\u\r\d'

# Dump mode for debugging
./build/bin/veyrm --dump '\n' --map room
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