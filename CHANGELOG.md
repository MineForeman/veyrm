# Changelog

All notable changes to Veyrm will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
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