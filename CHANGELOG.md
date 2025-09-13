# Changelog

All notable changes to Veyrm will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Angband-style lit rooms - 30% of rooms are permanently illuminated
  - Entire room becomes visible when player enters
  - Lit rooms remain bright in memory (not dimmed like normal tiles)
  - "The room is lit!" message when entering lit rooms
  - Strategic gameplay element with risk/reward tradeoffs

### Fixed
- FOV initialization bug - removed debug code that was setting entire map as visible at startup
- FOV movement update bug - added updateFOV() calls after player movement to properly recalculate visibility

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