# Changelog

All notable changes to the Veyrm project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v0.11.3] - 2025-01-14

### Added

- Phase 11.3: Item Usage System
  - Implemented 'u' command to use items from inventory
  - Healing potions restore HP when used
  - Implemented 'D' command to drop items at player position
  - Dropped items appear on map and can be picked up again
  - Usage feedback in message log
  - Turn consumption: use = normal turn, drop = fast turn

- Phase 11.2: Complete Inventory UI System
  - Full-featured inventory screen with item display
  - Navigation with arrow keys and a-z direct selection
  - Item actions: Use (u), Drop (D), Examine (E)
  - Item details panel with properties
  - Visual selection highlighting
  - Action bar showing available commands
  - Support for scrolling large inventories
  - Color-coded items by type (weapons, armor, potions, scrolls)
  - Status bar with slots used, weight, and gold count

- Phase 11.1: Complete inventory storage system
  - 26-slot capacity inventory with a-z mapping potential
  - Smart item stacking for stackable items
  - Special gold handling (bypasses inventory)
  - Weight tracking system (optional)
  - Comprehensive query methods
  - Full test coverage (332 lines of tests)
  - Integration with pickup system


### Changed

- Updated Player class to include inventory
- Modified game_screen.cpp GET_ITEM handler to use inventory
- Fixed incomplete type error in inventory.h (forward declaration → full include)
- Changed drop/examine keys to uppercase D/E to avoid slot conflicts
- Fixed inventory event routing in main.cpp for both normal and dump modes
- Added inventory panel to GameScreen component system
- ItemManager now supports spawning existing items (for drops)


### Fixed

- Game freeze when opening inventory (component structure issue)
- Inventory input not being processed (event routing in INVENTORY state)
- Dropped items not appearing on map (added ItemManager::spawnItem overload)
- Key conflicts between actions and slot selection (d/e → D/E)
- Arrow key navigation not working in inventory (event forwarding)


### Technical

- Added inventory.cpp to CMakeLists.txt
- Added inventory_renderer.cpp to CMakeLists.txt
- Added test_inventory.cpp to test suite
- All tests passing (135 test cases, 1775 assertions)
- ~500 lines of new code across inventory implementation


## [v0.10.1] - 2025-09-14

### Added
- Phase 10.1: Item Entity System
  - Complete item framework with factory pattern
  - Item spawning in dungeons
  - Pickup system with 'g' key
  - Gold collection
  - Item rendering on map


## [v0.9.0] - Previous

### Added
- Phase 9: Complete Combat System
  - Bump-to-attack mechanics
  - Damage calculation with attack/defense
  - Death handling
  - Combat messages


## [v0.8.3] - Previous

### Added
- Phase 8.3: Basic AI System
  - Monster pathfinding
  - Chase behavior
  - Turn-based monster actions


## [v0.8.0] - Previous

### Added
- Phase 8: Monster System
  - Monster factory and spawning
  - Monster rendering
  - Basic monster entities


## Earlier Phases

See git history for detailed changes in phases 0-7, including:
- Project setup and dependencies
- Core game loop
- Map generation and rendering
- FOV system
- Entity management
- Turn system
- Input handling
