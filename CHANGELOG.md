# Changelog

All notable changes to Veyrm will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- **Phase 10.1: Item Entity System** - Complete implementation of collectible items
  - Item class with full property system (type, value, weight, stackability)
  - ItemFactory singleton for data-driven item creation from JSON
  - ItemManager for world item lifecycle management
  - 12 initial item types (potions, scrolls, food, gold, misc items)
  - Item rendering in game world with appropriate colors and symbols
  - Item pickup system with 'g' key
  - Gold collection with automatic addition to player wealth
  - Items spawn randomly in rooms during map generation (5-10 items, 3-6 gold piles)
  - Comprehensive unit tests with 100% API coverage
  - Full integration with existing game systems

- Visualization Tools - comprehensive visualization capabilities
  - Gource video generation for git history visualization
    - Creates animated visualization of repository development
    - Saves to tmp/ directory (gitignored)
    - Configurable speed and duration settings
    - Option to clean old videos with --clean flag
  - Visual class diagram generator using Graphviz
    - Creates UML-style class diagrams with inheritance and relationships
    - Color-coded subsystems (Core, Entity, Map, Combat, AI, Utilities)
    - Shows inheritance, composition, and usage relationships
    - Outputs both SVG and PNG formats
  - Documentation of 12 additional visualization options for future implementation
    - Code metrics dashboard
    - Dependency graphs
    - Git statistics
    - Memory layout visualization
    - Performance flamegraphs
    - And more in DOC/VISUALIZATION_OPTIONS.md

### Changed

- Build script enhancements
  - Added 'gource' command for creating repository visualization videos
  - Added 'diagram' command for generating class diagrams
  - Gource videos now use fixed filename (veyrm-gource.mp4) with overwrite
  - All visualization outputs go to tmp/ directory which is gitignored

## Previous Release (v0.9.4)

- Enhanced Logging System - separate log files by category
  - Created separate log files for each category (player, AI, combat, environment, etc.)
  - All logs now stored in logs/ directory for better organization
  - Main debug log (veyrm_debug.log) contains all events chronologically
  - Category-specific logs for focused debugging:
    - veyrm_player.log - Player actions and movements
    - veyrm_ai.log - Monster AI decisions and movements
    - veyrm_combat.log - Combat events and damage
    - veyrm_env.log - Environment interactions (doors, terrain)
    - veyrm_map.log - Map generation events
    - veyrm_system.log - System messages
    - veyrm_inventory.log - Item interactions
    - veyrm_turn.log - Turn system events
    - veyrm_fov.log - Field of view updates
    - veyrm_spawn.log - Monster spawning
  - Automatic logs/ directory creation on startup
  - Fixed monster movements incorrectly appearing in player log

- Door System Implementation
  - Added door tiles (DOOR_OPEN and DOOR_CLOSED)
  - 'o' key binding for opening/closing adjacent doors
  - Doors block movement and visibility when closed
  - Automatic door placement at room entrances during map generation
  - Door interactions logged to environment log
  - Support for multiple door interactions in single turn

### Fixed

- Test Suite Failures
  - Fixed room generation tests to use Config values instead of deprecated static constants
  - Fixed procedural dungeon generation to always place stairs
  - Fixed TEST_ROOM validation by using open doors for connectivity
  - Added emergency fallback to ensure stairs placement
  - Fixed sign comparison warnings in tests
  - All 126 test cases now passing

### Changed

- Logging System Organization
  - Moved all log files into logs/ directory
  - Separated monster movement logs from player logs (now in AI log)
  - Updated build.sh clearlog command to handle new log structure

## Previous Releases

### Added
- Phase 9: Complete Combat System Implementation
  - Phase 9.1: Combat Stats - full d20 combat mechanics
    - CombatSystem class with d20 attack rolls and defense values
    - Critical hits (natural 20) and critical misses (natural 1)
    - Attack bonuses, defense bonuses, and damage calculations
    - Combat integration with Entity base class
    - Comprehensive combat logging system
  - Phase 9.2: Bump Combat - player melee combat
    - Player attacks monsters by moving into them (bump-to-attack)
    - Combat messages displayed in game message log
    - Experience rewards for defeating monsters
    - Automatic combat resolution with visual feedback
  - Phase 9.3: Death Handling - complete death mechanics
    - Player death triggers death screen with options to return to menu or quit
    - Monster death removes entity and awards experience
    - Death messages in combat log
    - HP display fixed (no longer shows 1/1 for all entities)
- Global Logging System - comprehensive debug infrastructure
  - Multi-level logging (ERROR, WARN, INFO, DEBUG, TRACE)
  - Category-specific logging (COMBAT, AI, MOVEMENT, etc.)
  - File-based log output to logs/veyrm_debug.log
  - Automatic log directory creation
  - Combat logging with detailed d20 roll information
- Player Stat Improvements
  - Increased starting HP from 20 to 50
  - Increased starting attack from 5 to 8
  - Increased starting defense from 2 to 5
  - All values configurable via config system
- Phase 8.3: Basic AI System - intelligent monster behavior
  - MonsterAI class with 5 behavioral states (IDLE, ALERT, HOSTILE, FLEEING, RETURNING)
  - A* pathfinding system with 8-directional movement support
  - Line-of-sight detection using Bresenham line algorithm
  - Room-bound territorial behavior for strategic gameplay
  - Chase mechanics with room-leaving exception for pursuit
  - Fleeing behavior for low-health monsters (< 25% health)
  - Turn-integrated AI processing for smooth gameplay
  - Comprehensive unit tests for all AI behaviors
- Phase 8.2: Monster Spawning System - dynamic creature population
  - SpawnManager with room-preference spawning (95% in rooms, 5% in corridors)
  - Depth-based monster selection with threat scaling
  - Dynamic spawning during gameplay based on monster count
  - FOV-aware spawning (outside player vision)
  - Configurable spawn rates and population limits
- Phase 8.1: Monster Entity System - complete monster implementation
  - Data-driven monster design with JSON configuration
  - Monster class with combat stats (HP, attack, defense, speed, XP)
  - MonsterFactory singleton for template-based creation
  - Behavior flags (aggressive, can_open_doors, can_see_invisible)
  - Threat level system (a-f ranking)
  - 5 initial monsters: Gutter Rat, Cave Spider, Kobold, Orc Rookling, Zombie
- Configuration System - YAML-based game settings
  - `config.yml` with comprehensive game configuration
  - Command-line arguments override config values
  - Integrated rapidyaml library for YAML parsing
  - Configurable map dimensions, room generation, player stats, FOV radius
- Build System Improvements
  - Binary size and build stats display in menu
  - Removed "Press Enter to continue" prompts
  - All binaries now run from project root directory
  - Added `--data-dir` command-line option
- Angband-style lit rooms - 95% of rooms are permanently illuminated (changed from 30%)
  - Entire room becomes visible when player enters
  - Lit rooms remain bright in memory (not dimmed like normal tiles)
  - "The room is lit!" message when entering lit rooms
  - Strategic gameplay element with risk/reward tradeoffs

### Changed
- Lit room probability increased from 30% to 95% (configurable)
- Map generation now uses config values for all parameters
- Player stats now use config values (HP: 20, Attack: 5, Defense: 2)
- FOV radius now uses config value (default: 10)
- Tests updated to use config values instead of hardcoded constants

### Fixed
- Critical HP display bug - Player and Entity classes had duplicate hp/max_hp members causing combat to use wrong values
- Player could not attack monsters - bump-to-attack was not implemented in input handling
- Combat messages not appearing in game log - messages were only going to debug log, not the UI
- Debug log output interfering with game display - reduced console output to ERROR level only
- Test failures after stat changes - updated test expectations to match new config values
- FOV initialization bug - removed debug code that was setting entire map as visible at startup
- FOV movement update bug - added updateFOV() calls after player movement to properly recalculate visibility
- Lit room probability now correctly uses config value instead of hardcoded 30%
- Test execution directory issues - all tests run from project root

## [0.7.3] - 2025-01-13

### Added
- Phase 7.3: Layout System - responsive three-panel UI layout
- Fullscreen terminal support with dynamic sizing
- Minimum terminal size validation (80x24)
- Proper panel boundaries to prevent map/UI overlap

### Changed
- Switched from fixed-size to fullscreen terminal display
- Map viewport now dynamically adjusts to terminal size

## [0.7.2] - 2025-01-13

### Added
- Phase 7.2: Status Bar - real-time game status display
- HP display with current/max values
- Player position tracking
- Turn counter and world time display
- Dungeon depth indicator
- Debug mode toggle (F5)

## [0.7.1] - 2025-01-13

### Added
- Phase 7.1: Message Log - game event messaging system
- 5-line scrolling message display
- Message history with 100-message buffer
- Color-coded message types (combat, movement, system)

## [0.6.3] - 2025-01-12

### Added
- Phase 6.3: Map Memory - exploration tracking
- Remembered tiles shown dimmed when out of sight
- Persistent exploration state between turns

## [0.6.2] - 2025-01-12

### Added
- Phase 6.2: Visibility System - fog of war rendering
- Entity visibility filtering based on FOV
- Different rendering styles for visible/remembered/unknown tiles

## [0.6.1] - 2025-01-12

### Added
- Phase 6.1: FOV Algorithm - field of view calculation
- Symmetric shadowcasting with 10-tile sight radius
- Line-of-sight blocking by walls
- Transparent vs opaque tile handling

## [0.5.3] - 2025-01-11

### Added
- Phase 5.3: Map Validation - connectivity verification
- BFS connectivity checking
- Automatic stair placement at farthest point
- Map statistics and warnings

## [0.5.2] - 2025-01-11

### Added
- Phase 5.2: Corridor Generation - room connection system
- L-shaped, straight, and S-shaped corridor styles
- MST, nearest neighbor, and sequential connection strategies
- Automatic door placement

## [0.5.1] - 2025-01-11

### Added
- Phase 5.1: Room Generation - procedural dungeon rooms
- Random room placement with overlap detection
- Configurable room size and count

## [0.4.0] - 2025-01-10

### Added
- Basic player entity system
- Movement with collision detection
- Turn-based action system
- Entity manager for game objects

## [0.3.0] - 2025-01-09

### Added
- Map rendering with Unicode tiles
- Viewport system for large maps
- Color scheme support with terminal auto-detection

## [0.2.0] - 2025-01-08

### Added
- FTXUI terminal UI framework integration
- Main menu system
- Game state management
- Basic input handling

## [0.1.0] - 2025-01-07

### Added
- Initial project structure
- CMake build system
- Basic game loop
- Map data structures