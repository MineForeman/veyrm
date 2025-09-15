# Legacy System Removal Plan

## Overview

This document outlines the systematic removal of legacy entity system components as we transition to a pure ECS (Entity Component System) architecture. The removal is prioritized from easiest to hardest based on dependencies and integration complexity.

## Current State

- **Branch**: `refactor-optimize`
- **Status**: ECS system functional, legacy system still partially integrated
- **Goal**: Complete removal of inheritance-based entity system

## Removal Priority Levels

- 🟢 **EASY**: Minimal dependencies, can be removed immediately
- 🟡 **MEDIUM**: Moderate refactoring required
- 🔴 **HARD**: Core system dependencies, careful migration needed
- 🔥 **VERY HARD**: Fundamental architecture changes required

---

## 🟢 EASY - Immediate Removal Candidates

### 1. Legacy Entity Test Files ✅ COMPLETED

**Files**:

- `tests/test_entity.cpp` ✅ Deleted
- `tests/test_player.cpp` ✅ Deleted
- `tests/test_monster.cpp` ✅ Deleted
- `tests/test_entity_manager.cpp` ✅ Deleted
- `tests/test_entity_memory_safety.cpp` ✅ Deleted
- `tests/test_spawn_manager.cpp` ✅ Deleted
- `tests/test_monster_ai.cpp` ✅ Deleted

**Status**: COMPLETED

- Deleted all legacy test files
- Updated `tests/CMakeLists.txt`
- All tests passing (1558 assertions in 128 test cases)

**Risk**: None - Successfully removed

---

## 🟡 MEDIUM - Moderate Refactoring Required

### 2. Entity Adapter ✅ COMPLETED

**Files**: `include/ecs/entity_adapter.h` ✅ Deleted

**Status**: COMPLETED

- Replaced all adapter calls with direct ECS entity creation in `game_world.cpp`
- Updated conversion code to create components directly
- Removed EntityAdapter tests from `test_ecs_factory.cpp`
- Deleted adapter header file
- All tests passing (1527 assertions in 125 test cases)

**Changes Made**:

- Direct player entity creation with all required components
- Direct monster entity creation with AI and loot components
- Direct item entity creation with item components
- Added necessary component includes to `game_world.cpp`

### 3. Monster Factory ✅ COMPLETED

**Files**:

- `include/monster_factory.h` ✅ Deleted
- `src/monster_factory.cpp` ✅ Deleted

**Status**: COMPLETED

- Migrated JSON loading to ECS DataLoader in `game_manager.cpp`
- Stubbed out `EntityManager::createMonster()` (EntityManager will be removed soon)
- Rewrote `test_monster_integration.cpp` to use ECS DataLoader and EntityFactory
- Removed MonsterFactory from CMakeLists.txt
- All builds successfully

**Changes Made**:

- `game_manager.cpp` now only calls `ecs::DataLoader::loadAllData()`
- Removed redundant monster data loading
- Updated tests to use ECS factories and templates
- Test count: 126 tests, 125 passing (1 unrelated ECS combat test failing)

### 4. Legacy Combat System ✅ COMPLETED

**Files**:

- `include/combat_system.h` ✅ Deleted
- `src/combat_system.cpp` ✅ Deleted
- `tests/test_combat_system.cpp` ✅ Already removed

**Status**: COMPLETED

- Removed legacy combat system files
- Fixed ECS CombatSystem to properly process and log combat results
- All references updated to use `ecs::CombatSystem`
- CMakeLists.txt updated
- All tests passing (1488 assertions in 120 test cases)

**Changes Made**:

- Modified `CombatSystem::update()` to use and log CombatResult
- Ensured combat messages are properly sent to MessageLog
- Verified ECS combat working in integration tests

### 5. Legacy Item System ✅ PARTIALLY COMPLETED

**Files**:

- `include/item.h` - ⚠️ Still in use (required by Player/Inventory)
- `include/item_factory.h` ✅ Deleted
- `include/item_manager.h` ✅ Deleted
- `src/item_factory.cpp` ✅ Deleted
- `src/item_manager.cpp` ✅ Deleted

**Status**: PARTIALLY COMPLETED

- Removed ItemFactory and ItemManager classes
- Item pickup/drop temporarily disabled in UI
- Item.h/cpp still required for Player inventory system
- All tests passing (1485 assertions in 120 test cases)

**Changes Made**:

- Deleted item_factory.h/cpp and item_manager.h/cpp
- Updated CMakeLists.txt to remove deleted files
- Commented out item pickup/drop code in game_screen.cpp
- Commented out item rendering in renderer.cpp
- Updated tests to not use ItemFactory
- Items are now loaded by ECS DataLoader

**Remaining Work**:

- Item.h/cpp cannot be removed until Player and Inventory classes are migrated to ECS
- Need to implement ECS-based item pickup/drop functionality
- Need to restore item rendering through ECS render system

**Original Removal Steps**:

1. Convert all items to use `ecs::ItemComponent`
2. Replace `ItemManager` with ECS item queries
3. Update inventory to work with ECS items
4. Migrate item data loading to ECS factory

**Risk**: Medium - Affects multiple gameplay systems

### 6. Monster Class ✅ COMPLETED

**Files**:

- `include/monster.h` ✅ Deleted
- `src/monster.cpp` ✅ Deleted

**Status**: COMPLETED

- Removed all Monster class references from EntityManager
- Updated GameSerializer to remove Monster casting
- Fixed game_world.cpp to handle legacy monsters without Monster class
- Updated all includes to remove monster.h
- Updated CMakeLists.txt
- All tests passing (1454 assertions in 118 test cases)
- Game builds and runs successfully

**Changes Made**:

- EntityManager methods for Monster removed
- GameSerializer no longer casts to Monster type
- game_world.cpp uses is_monster flag instead of dynamic_cast
- All Monster includes commented out or removed

### 7. Player Class (Original #7)

**Files**:

- `include/player.h`
- `src/player.cpp`

**Dependencies**:

- Core to input handling
- Save/load system
- Status display
- Inventory management

**Removal Steps**:

1. Move all player state to ECS components
2. Update `InputHandler` to work with ECS player entity
3. Convert player methods to ECS systems
4. Update status bar to read from ECS components
5. Remove player class

**Risk**: Medium-High - Central to many systems

---

## 🔴 HARD - Core System Dependencies

### 8. Entity Base Class (Original #8)

**Files**:

- `include/entity.h`
- `src/entity.cpp`

**Dependencies**: 41 files reference this base class

**Prerequisites**: Must remove Monster and Player classes first

**Removal Steps**:

1. Ensure no classes inherit from Entity
2. Remove all Entity* pointers/references
3. Delete entity files
4. Update all includes

**Risk**: High - Fundamental base class

### 9. Entity Manager (Original #9)

**Files**:

- `include/entity_manager.h`
- `src/entity_manager.cpp`

**Dependencies**:

- `GameManager`
- `GameScreen`
- `Renderer`
- Save/load system

**Removal Steps**:

1. Replace all `EntityManager` usage with `ecs::GameWorld`
2. Convert entity queries to ECS queries
3. Update spawn/death logic to use ECS
4. Migrate collision detection to ECS
5. Remove manager files

**Risk**: High - Central entity coordination

### 10. Game Serializer Legacy Support (Original #10)

**Files**: `src/game_serializer.cpp`

**Dependencies**: Critical for save game functionality

**Removal Steps**:

1. Implement ECS serialization in `ecs::SaveLoadSystem`
2. Create migration path for old save files
3. Remove legacy entity serialization code
4. Test save/load compatibility

**Risk**: High - Breaking saves loses player progress

---

## 🔥 VERY HARD - Fundamental Architecture Changes

### 11. GameScreen Integration (Original #11)

**Files**: `src/game_screen.cpp`

**Dependencies**: Main gameplay loop

**Removal Steps**:

1. Convert all entity interactions to ECS
2. Replace direct entity access with ECS queries
3. Update rendering calls to use ECS
4. Refactor update loop for ECS systems
5. Remove all legacy entity imports

**Risk**: Very High - Core gameplay screen

### 12. Renderer Legacy Support (Original #12)

**Files**: `src/renderer.cpp`

**Dependencies**: Entire display pipeline

**Removal Steps**:

1. Convert to use only `ecs::RenderSystem`
2. Remove `EntityManager` references
3. Update viewport logic for ECS
4. Ensure all rendering uses ECS components

**Risk**: Very High - Breaking this breaks display

---

## Implementation Strategy

### Phase 1: Low-Hanging Fruit (1 day)

- [x] Remove test files ✅ COMPLETED
- [x] Remove EntityAdapter ✅ COMPLETED
- [x] Remove MonsterFactory ✅ COMPLETED

### Phase 2: Combat & Items (2-3 days)

- [ ] Migrate combat system
- [ ] Convert items to ECS
- [ ] Update related tests

### Phase 3: Entity Classes (3-4 days)

- [ ] Remove Monster class
- [ ] Remove Player class
- [ ] Remove Entity base class

### Phase 4: Core Systems (1 week)

- [ ] Replace EntityManager
- [ ] Update GameSerializer
- [ ] Refactor GameScreen
- [ ] Update Renderer

### Phase 5: Cleanup (2 days)

- [ ] Remove all legacy includes
- [ ] Update documentation
- [ ] Comprehensive testing
- [ ] Performance profiling

---

## Testing Checklist

After each removal:

- [ ] Run `./build.sh test`
- [ ] Test gameplay with `./build.sh run`
- [ ] Verify save/load functionality
- [ ] Check memory leaks with valgrind
- [ ] Test all map types
- [ ] Verify combat mechanics
- [ ] Test inventory operations

---

## Success Criteria

- Zero references to legacy `Entity`, `Player`, `Monster` classes
- All tests passing
- No performance regression
- Save game compatibility (or migration path)
- Clean compilation with no warnings
- Updated documentation reflecting pure ECS architecture

---

## Rollback Plan

If critical issues arise:

1. Git tag current state before starting: `git tag pre-legacy-removal`
2. Keep legacy removal in separate branch
3. Maintain compatibility layer if needed for gradual migration
4. Document any breaking changes for save files

---

## Notes

- The `refactor-optimize` branch has already removed `monster_ai.h` and `spawn_manager.h`
- ECS system is functional but coexisting with legacy
- Priority should be removing easy targets first to build momentum
- Each removal should be a separate commit for easy rollback
