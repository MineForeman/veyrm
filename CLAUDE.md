# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

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

```bash
# Quick build
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
./ang

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