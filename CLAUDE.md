# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Testing Requirements

**IMPORTANT:** Tests must ALWAYS be created and run when implementing new features or fixing bugs. Follow these guidelines:

1. **Test-First Development**: Write tests before or alongside implementation
2. **Run Tests**: Always run `./build.sh test` after changes to ensure nothing breaks
3. **Coverage**: Each new class or function should have corresponding unit tests
4. **Validation**: Never mark a task as complete without passing tests
5. **Test Location**: Tests go in `tests/` directory with naming pattern `test_<component>.cpp`
6. **All Tests Must Pass**: No new feature or bug fix is considered complete until ALL tests pass - both new tests and existing tests. A failing test suite means the work is incomplete.

### Gameplay Testing

**IMPORTANT:** Always test gameplay features using the build.sh dump and keys commands for automated verification:

- `./build.sh dump` - Runs the game with frame dump output for visual debugging. Use this to verify rendering, map layout, and UI elements are displaying correctly.
- `./build.sh run --keys "hjklq"` - Runs the game with automated key inputs. Use this to test movement, input handling, and game flow without manual interaction.
- Combine both: Test complex scenarios by providing key sequences and examining the frame dumps to ensure the game responds correctly.

Example testing workflow:
```bash
# Test movement and rendering
./build.sh run --keys "hjklhjkl" --dump  # Move around and check frame output

# Test specific scenarios
./build.sh run --keys "jjjlllkkkhhhq"    # Move to specific location and quit
```

## Project Overview

Veyrm is a modern C++ roguelike game inspired by Angband, using FTXUI for terminal UI. The game features a dark fantasy world where players descend through the Spiral Vaults beneath Veyrmspire to break the last shard of a dead god's crown.

## Tech Stack

- **Language:** C++23
- **Build System:** CMake ≥ 3.25
- **UI Framework:** FTXUI (terminal UI with Unicode, color, reactive rendering)
- **JSON:** nlohmann/json for save games and content tables
- **Testing:** Catch2
- **RNG:** std::mt19937_64
- **Target Platforms:** macOS, Linux, Windows (UTF-8 terminals)

## Build Commands

**IMPORTANT:** Always use `./build.sh` as the preferred way to build and run the game. It handles all build configurations, dependency management, and provides helpful utilities.

```bash
# Preferred method - using build.sh
./build.sh build    # Build the game
./build.sh run      # Run the game (with map selection menu)
./build.sh test     # Run all tests
./build.sh clean    # Clean build directory
./build.sh dump     # Run with frame dump for debugging

# Manual CMake commands (if needed)
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
./bin/veyrm

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j
```

## Architecture

The codebase follows a component-based architecture with these core systems:

- **Game** - World state management, message log, turn system
- **Map** - Tile grid with visibility and memory (# wall, · floor, > stairs)
- **DungeonGen** - Room and corridor generation with BFS-based connectivity
- **FOV** - Symmetric shadowcasting field-of-view (8 octants)
- **Entities** - Player, Monster, Item structs
- **Systems** - Input handling, movement, combat (bump-to-attack), inventory, monster AI
- **RendererTUI** - FTXUI-based layout for map, log, and status displays
- **Data** - JSON-based configuration (monsters.json, items.json, save.json)

## Key Algorithms

- **Map Generation:** Random room placement with overlap rejection, L-shaped corridors
- **FOV:** Symmetric shadowcasting with exploration memory
- **Pathfinding:** 4-directional BFS for monster movement
- **Combat:** Simple bump-to-attack with damage rolls (player: 1d6, monsters: [min,max])

## Game Data Format

Monsters and items are defined in JSON:

```json
// monsters.json
[
  { "id": "gutter_rat", "glyph": "r", "color": "grey", "hp": 3, "atk": [1,3], "def": 0, "speed": 100 }
]

// items.json
[
  { "id": "potion_minor", "glyph": "!", "color": "magenta", "heal": 6 }
]
```

## Controls

- **Movement:** Arrow keys or hjkl + diagonals (yubn)
- **Actions:** g (get), i (inventory), u (use), D (drop), . (wait), q (quit), N (new game)

## World Context

The game starts in Ring 1: Woundworks of the Spiral Vaults. The world documentation in DOC/WORLD/ provides narrative context and thematic guidance. The MVP documentation in DOC/MVP/ contains implementation specifications.

## Special Files

### VIBE.md - User Notes
**IMPORTANT:** `DOC/VIBE.md` contains the user's personal notes and should NOT be edited by Claude. This file should be checked into git when changes are made, but Claude should never modify its contents. It serves as the user's scratchpad and working notes for the project.
