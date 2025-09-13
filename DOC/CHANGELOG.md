# Changelog

All notable changes to the Veyrm project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
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
- Fixed incomplete type error in inventory.h

### Technical
- Added inventory.cpp to CMakeLists.txt
- Added test_inventory.cpp to test suite
- All tests passing (135 test cases, 1775 assertions)

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