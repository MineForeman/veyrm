# Veyrm MVP Implementation Plan

A detailed, step-by-step implementation plan for building the Veyrm roguelike MVP. Each phase is designed to be small, testable, and tagged in git for version control.

## Git Strategy

- Create feature branches for each phase: `feature/phase-X-name`
- Tag completion of each phase: `v0.0.X-phase-name`
- Commit after each successful step within a phase
- Run tests before merging to main
- Document changes in commit messages

---

## Phase 0: Project Setup (Day 1)

### 0.1 Initialize Repository ✅
- [x] Create CMakeLists.txt with C++23 configuration
- [x] Add .gitignore for C++ projects
- [x] Create directory structure: `src/`, `include/`, `data/`, `tests/`
- [x] **Test:** CMake configures successfully
- [x] **Git tag:** `v0.0.1-project-init`

### 0.2 Dependencies Setup ✅
- [x] Configure FetchContent for FTXUI
- [x] Configure FetchContent for nlohmann/json
- [x] Configure FetchContent for Catch2
- [x] Create simple main.cpp with "Hello Veyrm"
- [x] **Test:** Build and run hello world
- [x] **Git tag:** `v0.0.2-dependencies`
- **Notes:** Fixed AddressSanitizer conflicts, added terminal cleanup, enhanced build.sh

### 0.3 Basic FTXUI Window ✅
- [x] Create minimal FTXUI app with quit on 'q'
- [x] Set up screen with proper Unicode support
- [x] Add window title and basic layout
- [x] Implement three-panel game screen (map, status, log)
- [x] Add state management (MENU, PLAYING, QUIT)
- [x] **Test:** App launches, displays, and quits properly
- [x] **Git tag:** `v0.0.3-ftxui-window`
- **Notes:** Fixed segfault in menu renderer, resolved terminal mouse tracking issues

### 0.4 Input and Turn Foundation ✅
- [x] Create `InputHandler` class for keyboard processing
- [x] Add ~~vi-keys (hjkl)~~ and arrow key movement handling (vi-keys excluded per request)
- [x] Implement basic turn counter in GameManager
- [x] Add player position tracking (x, y coordinates)
- [x] Update game screen to show player position
- [x] **Test:** Input handling works, player moves, turns increment
- [x] **Git tag:** `v0.0.4-input-turns`
- **Notes:** Completed as part of Phase 1.1 and 1.2

---

## Phase 1: Core Game Loop (Day 2)

### 1.1 Game State Structure ✅
- [x] Create `GameManager` class with state enum (extended with PAUSED, INVENTORY, HELP)
- [x] Create `InputHandler` class for keyboard processing
- [x] Implement state transitions with previous state tracking
- [x] **Test:** State transitions work correctly
- [x] **Git tag:** `v0.1.1-game-state`
- **Notes:** Complete state management with InputHandler, player movement, and turn counting

### 1.2 Turn System ✅
- [x] Implement `TurnManager` class
- [x] Add player turn vs world turn distinction
- [x] Create turn counter
- [x] **Test:** Turns increment properly
- [x] **Git tag:** `v0.1.2-turn-system`
- **Notes:** Implemented action speed system, message log, world time tracking

### 1.3 Main Game Loop ✅
- [x] Connect input → update → render cycle
- [x] Add frame rate limiting
- [x] Implement clean shutdown
- [x] **Test:** Game loop runs at stable FPS
- [x] **Documentation:** Add game loop diagram to docs
- [x] **Git tag:** `v0.1.3-game-loop`
- **Notes:** GameLoop class with 60 FPS target, FrameStats monitoring, FTXUI integration via refresh thread

---

## Phase 2: Map Foundation (Day 3)

### 2.1 Tile System ✅
- [x] Create `Tile` enum (WALL, FLOOR, STAIRS)
- [x] Create `Map` class with 2D tile array
- [x] Implement coordinate system (Point struct)
- [x] **Test:** Map stores and retrieves tiles correctly
- [x] **Git tag:** `v0.2.1-tile-system`
- **Notes:** Full tile system with properties, collision detection, visibility tracking

### 2.2 Map Rendering ✅
- [x] Create `MapRenderer` class with FTXUI integration
- [x] Render map with ASCII characters (#, ., >)
- [x] Add terminal-adaptive color support for tiles
- [x] Implement viewport system for scrolling
- [x] Fix layer compositing bug (single-pass rendering)
- [x] Add ColorScheme for terminal compatibility
- [x] **Test:** Map displays correctly at 80x24
- [x] **Git tag:** `v0.2.2-map-render`
- **Notes:** Complete rendering system with viewport, adaptive colors, and wall visibility fix

### 2.3 Simple Test Map

- [x] Create `MapGenerator` class for map generation
- [x] Extract existing 5-room dungeon from GameManager
- [x] Implement `generateTestRoom()` - single 20x20 room
- [x] Implement `generateTestDungeon()` - current multi-room layout
- [x] Create `MapValidator` class for map validation
- [x] Ensure connectivity and spawn point validation
- [x] Add map selection system to GameManager
- [x] Add command-line map selection (`--map <type>`)
- [x] Fix corridor wall generation and room connections
- [x] **Test:** All test maps load and are playable
- [x] **Git tag:** `v0.2.3-test-map`
- **Notes:** Complete map system with 5 types (room, dungeon, corridor, arena, stress), validation, proper walls

---

## Phase 3: Entity System (Day 4)

### 3.1 Entity Base ✅
- [x] Create `Entity` base class with position
- [x] Create `Player` class extending Entity
- [x] Add glyph and color properties
- [x] Create `EntityManager` class for entity lifecycle
- [x] Integrate entities with GameManager
- [x] Update movement to use entity system
- [x] Comprehensive test suite (57 test cases, 404 assertions)
- [x] Fix all test failures and ensure 100% pass rate
- [x] **Test:** Entities can be created and positioned ✅
- [x] **Git tag:** `v0.3.1-entity-base` ✅
- **Notes:** Complete entity system with Player class, EntityManager, movement integration, and full test coverage

### 3.2 Player on Map (Completed as part of 3.1) ✅
- [x] Add player to game state
- [x] Render player on map  
- [x] Ensure player renders above floor
- [x] **Test:** Player appears at spawn position ✅
- **Notes:** Completed as part of Phase 3.1 integration

### 3.3 Entity Manager (Completed as part of 3.1) ✅
- [x] Create `EntityManager` class
- [x] Track all entities in single container
- [x] Add entity lookup by position
- [x] **Test:** Can add, remove, find entities ✅
- [x] **Documentation:** Entity architecture documented in TESTS.md
- **Notes:** Completed as part of Phase 3.1 comprehensive implementation

---

## Phase 3.5: Test Suite Completion ✅

### 3.5.1 Comprehensive Test Coverage
- [x] Document all 57 test cases in TESTS.md
- [x] Fix MapGenerator stairs placement (out of bounds issue)
- [x] Fix MapGenerator spawn point validation for all map types
- [x] Fix MapValidator CORRIDOR_TEST connectivity issues
- [x] Fix MessageLog constructor behavior and message ordering
- [x] Achieve 100% test pass rate (404 assertions)
- [x] **Test:** All unit tests pass consistently ✅
- **Notes:** Complete test suite with comprehensive coverage of entity system, map generation, input handling, and message logging

---

## Phase 4: Movement System (Day 5)

### 4.1 Basic Movement ✅
- [x] Implement 4-directional movement (arrow keys)
- [x] Add collision detection with walls
- [x] Update player position on valid moves
- [x] **Test:** Player moves in empty spaces, stops at walls ✅
- **Notes:** Completed as part of entity system integration

### 4.2 8-Directional Movement (Partially Complete)
- [x] Add wait command (. and numpad 5)
- [ ] Add diagonal movement (yubn keys) - *Deferred per design decision*
- [ ] Implement hjkl vim-style movement - *Deferred per design decision*
- [x] **Test:** Wait command works ✅
- **Notes:** Arrow keys only for movement, vi-keys explicitly excluded from current design

### 4.3 Movement Validation ✅
- [x] Movement validation integrated into Player class
- [x] Add bounds checking
- [x] Add entity collision checking (blocking vs non-blocking entities)
- [x] **Test:** Movement validation prevents invalid moves ✅
- **Notes:** Validation handled by Player::tryMove() method with comprehensive test coverage

---

## Phase 5: Map Generation (Day 6-7)

### 5.1 Room Generation ✅ Complete

- [x] Create `Room` class with bounds
- [x] Implement random room placement
- [x] Add overlap detection
- [x] **Test:** Generates non-overlapping rooms
- [ ] **Git tag:** `v0.5.1-room-generation`

### 5.2 Corridor Generation ✅ Complete

- [x] Implement L-shaped corridor algorithm
- [x] Add straight and S-shaped corridor styles
- [x] Connect room centers
- [x] Carve corridors in map with configurable width
- [x] Implement MST, nearest neighbor, and sequential connection strategies
- [x] Add door placement system
- [x] **Test:** All rooms connected
- [ ] **Git tag:** `v0.5.2-corridors`

### 5.3 Map Validation ✅ Complete
- [x] Implement BFS connectivity check
- [x] Ensure all floor tiles reachable
- [x] Place stairs at farthest point
- [x] **Test:** Map always fully connected
- [x] **Documentation:** Document generation algorithm
- [x] **Git tag:** `v0.5.3-map-validation`

---

## Phase 6: Field of View (Day 8)

### 6.1 FOV Algorithm ✅ Complete
- [x] Implement symmetric shadowcasting
- [x] Create `FOV` class with calculate method
- [x] Handle 8 octants properly
- [x] **Test:** FOV calculates correctly
- [x] **Git tag:** `v0.6.1-fov-algorithm`

### 6.2 Visibility System ✅ Complete
- [x] Add visible/revealed states to tiles
- [x] Update visibility on player move
- [x] Implement fog of war rendering
- [x] **Test:** Only visible area shown
- [x] **Git tag:** `v0.6.2-visibility`

### 6.3 Map Memory ✅ Complete
- [x] Track explored tiles
- [x] Render explored but not visible tiles dimmed
- [x] Persist exploration between turns
- [x] **Test:** Map memory persists correctly
- [x] **Git tag:** `v0.6.3-map-memory`

---

## Phase 7: UI Framework (Day 9)

### 7.1 Message Log ✅ Complete
- [x] Create `MessageLog` class
- [x] Implement 5-line display
- [x] Add message history storage
- [x] **Test:** Messages display and scroll
- [x] **Git tag:** `v0.7.1-message-log`

### 7.2 Status Bar ✅
- [x] Create status bar component (StatusBar class)
- [x] Display HP, position, turn count
- [x] Add color coding for HP levels
- [x] **Test:** Status updates in real-time
- [x] **Git tag:** `v0.7.2-status-bar`
- **Notes:** Implemented StatusBar class with HP color coding, position display, time formatting

### 7.3 Layout System
- [x] Implement three-panel layout (map, log, status)
- [x] Handle window resizing
- [x] Ensure minimum 80x24 size
- [x] **Test:** Layout responds to terminal size
- [x] **Documentation:** Document UI components
- [x] **Git tag:** `v0.7.3-layout`

---

## Phase 7.5: Configuration System ✅ Complete

### 7.5.1 YAML Configuration
- [x] Implement Config singleton class
- [x] Integrate rapidyaml library for YAML parsing
- [x] Create comprehensive config.yml file
- [x] Add command-line argument override support
- [x] Configure map dimensions (Angband standard 198x66)
- [x] Configure room generation parameters
- [x] Configure player starting stats
- [x] Configure FOV radius
- [x] Configure lit room probability (95%)
- [x] **Test:** Configuration loading and override tests pass ✅
- [ ] **Git tag:** `v0.7.5-config`

---

## Phase 8: Monster System (Day 10-11)

### 8.1 Monster Entity ✅ Complete
- [x] Create `Monster` class extending Entity
- [x] Add HP, attack, defense, speed, XP stats
- [x] Load monster definitions from JSON
- [x] Implement MonsterFactory with singleton pattern
- [x] Add 5 initial monsters (Gutter Rat, Cave Spider, Kobold, Orc Rookling, Zombie)
- [x] Integrate with EntityManager
- [x] **Test:** Monster creation and JSON loading tests pass ✅
- [x] **Documentation:** Created DOC/PHASES/8.1_MONSTER_ENTITY.md
- [ ] **Git tag:** `v0.8.1-monster-entity`

### 8.2 Monster Spawning ✓ COMPLETE
- [x] Add monsters to map generation
- [x] Implement spawn distribution (95% rooms, 5% corridors)
- [x] Create all 5 base monsters (Gutter Rat, Cave Spider, Kobold, Orc Rookling, Zombie)
- [x] Implement SpawnManager for initial and dynamic spawning
- [x] Add configurable spawn parameters (rate, distance, FOV checks)
- [x] Fix monster rendering in MapRenderer
- [x] **Test:** Monsters spawn in valid locations
- [x] **Test:** Room preference spawning works correctly
- [x] **Documentation:** [Phase 8.2 Documentation](PHASES/8.2_MONSTER_SPAWNING.md)
- [x] **Git tag:** `v0.8.2-spawning`

### 8.3 Basic AI ✅
- [x] Implement 8-way movement for monsters (N, NE, E, SE, S, SW, W, NW)
- [x] Add room-bound behavior (monsters stay in spawn room unless chasing)
- [x] Implement AI states (IDLE, ALERT, HOSTILE, FLEEING, RETURNING)
- [x] Add A* pathfinding with diagonal support and line-of-sight
- [x] Implement chase behavior with room-leaving exception
- [x] Add return-to-room behavior after losing player
- [x] Implement FOV-based player detection
- [x] Add random wander within boundaries when idle
- [x] Add fleeing behavior for low-health monsters
- [x] **Test:** Monsters use 8-way movement correctly
- [x] **Test:** Room-bound monsters stay in rooms when not chasing
- [x] **Test:** Monsters chase and return appropriately
- [x] **Test:** AI state transitions work correctly
- [x] **Test:** Pathfinding navigates obstacles properly
- [x] **Documentation:** [Phase 8.3 Requirements](PHASES/8.3_BASIC_AI_REQUIREMENTS.md)
- [x] **Documentation:** [Phase 8.3 Design Notes](PHASES/8.3_AI_DESIGN_NOTES.md)
- [x] **Git tag:** `v0.8.3-basic-ai`
- **Notes:** Complete AI system with MonsterAI class, Pathfinding system, integrated turn processing, and comprehensive unit tests

---

## Phase 9: Combat System (Day 12)

### 9.1 Combat Stats ✅
- [x] Add HP, attack, defense to all entities
- [x] Create `CombatSystem` class
- [x] Implement damage calculation
- [x] Integrate with Phase 8.3 AI system
- [x] Add comprehensive combat unit tests
- [x] **Test:** Damage calculates correctly
- [x] **Test:** AI combat integration works properly
- [x] **Documentation:** [Phase 9.1 Requirements](PHASES/9.1_COMBAT_STATS_REQUIREMENTS.md)
- [x] **Documentation:** [Phase 9.1 Implementation Plan](PHASES/9.1_IMPLEMENTATION_PLAN.md)
- [x] **Documentation:** [Phase 9.1 AI Integration](PHASES/9.1_AI_INTEGRATION_POINTS.md)
- [ ] **Git tag:** `v0.9.1-combat-stats`

### 9.2 Bump Combat ✅
- [x] Detect bump-to-attack
- [x] Apply damage on collision
- [x] Add combat messages to log
- [x] **Test:** Bumping causes damage
- [ ] **Git tag:** `v0.9.2-bump-combat`

### 9.3 Death Handling ✅
- [x] Remove entities at 0 HP
- [x] Handle player death (game over)
- [x] Handle monster death (remove from map)
- [x] **Test:** Death states work correctly
- [x] **Documentation:** Document combat formulas
- [x] **Git tag:** `v0.9.3-death-handling`

---

## Phase 10: Item System (Day 13-14)

### 10.1 Item Entity ✅ COMPLETE
- [x] Create `Item` class with full property system
- [x] Add item types (POTION, SCROLL, WEAPON, ARMOR, FOOD, GOLD, MISC)
- [x] Load item definitions from JSON (12 types implemented)
- [x] ItemFactory singleton for item creation
- [x] ItemManager for world item lifecycle
- [x] **Test:** Items load from data files (all tests passing)
- [x] **Git tag:** `v0.10.1-item-entity` (ready to tag)

### 10.2 Ground Items ✅ COMPLETE (merged with 10.1)
- [x] Spawn items during map generation (5-10 items, 3-6 gold piles)
- [x] Render items on map with appropriate colors and symbols
- [x] Handle multiple items per tile
- [x] **Test:** Items appear on map (verified)
- [x] **Merged with 10.1**

### 10.3 Pickup System ✅ COMPLETE (merged with 10.1)
- [x] Implement 'g' get command
- [x] Gold automatically added to player wealth
- [x] Remove items from map on pickup
- [x] **Test:** Can pick up items (verified)
- [x] **Note:** Full inventory system pending Phase 11

---

## Phase 11: Inventory System (Day 15)

### 11.1 Inventory Storage ✅
- [x] Create `Inventory` class with 26-slot capacity
- [x] Implement smart stacking for stackable items
- [x] Add inventory to player with special gold handling
- [x] Integrate with game pickup system
- [x] **Test:** All inventory tests passing (332 lines of tests)
- [x] **Git tag:** `v0.11.1-inventory-storage`
- **Notes:** Complete implementation with stacking, weight tracking, full test coverage

### 11.2 Inventory UI
- [ ] Implement 'i' inventory command
- [ ] Display numbered item list
- [ ] Show item details
- [ ] **Test:** Inventory displays correctly
- [ ] **Git tag:** `v0.11.2-inventory-ui`

### 11.3 Item Usage
- [ ] Implement 'u' use command
- [ ] Add healing potion effect
- [ ] Implement 'D' drop command
- [ ] **Test:** Items can be used and dropped
- [ ] **Documentation:** Document item effects
- [ ] **Git tag:** `v0.11.3-item-usage`

---

## Phase 12: Save System (Day 16)

### 12.1 Game Serialization
- [ ] Create save game JSON structure
- [ ] Serialize game state
- [ ] Serialize map and entities
- [ ] **Test:** Game state serializes correctly
- [ ] **Git tag:** `v0.12.1-serialization`

### 12.2 Save on Quit
- [ ] Implement autosave on 'q'
- [ ] Write save.json file
- [ ] Handle save errors gracefully
- [ ] **Test:** Save file created on quit
- [ ] **Git tag:** `v0.12.2-save-on-quit`

### 12.3 Load on Start
- [ ] Check for save file on launch
- [ ] Load and restore game state
- [ ] Handle missing/corrupt saves
- [ ] **Test:** Game resumes from save
- [ ] **Git tag:** `v0.12.3-load-on-start`

---

## Phase 13: Polish & Balance (Day 17-18)

### 13.1 Combat Balance
- [ ] Test and adjust damage values
- [ ] Balance monster HP and spawn rates
- [ ] Tune healing potion effectiveness
- [ ] **Test:** Combat feels balanced
- [ ] **Git tag:** `v0.13.1-balance`

### 13.2 Victory Condition
- [ ] Add "reach stairs" victory check
- [ ] Implement 'N' new game command
- [ ] Add victory/death messages
- [ ] **Test:** Can win and restart game
- [ ] **Git tag:** `v0.13.2-victory`

### 13.3 Visual Polish
- [ ] Add colors for all entities
- [ ] Improve message formatting
- [ ] Add welcome/help text
- [ ] **Test:** Game looks polished
- [ ] **Git tag:** `v0.13.3-polish`

---

## Phase 14: Testing Suite (Day 19)

### 14.1 Unit Tests
- [ ] Test map generation connectivity
- [ ] Test FOV symmetry
- [ ] Test combat calculations
- [ ] Test save/load integrity
- [ ] **Run:** All unit tests pass
- [ ] **Git tag:** `v0.14.1-unit-tests`

### 14.2 Integration Tests
- [ ] Test full game flow
- [ ] Test all keyboard commands
- [ ] Test edge cases (death, inventory full)
- [ ] **Run:** All integration tests pass
- [ ] **Git tag:** `v0.14.2-integration-tests`

### 14.3 Platform Testing
- [ ] Test on macOS
- [ ] Test on Linux
- [ ] Test on Windows
- [ ] **Verify:** Works on all platforms
- [ ] **Documentation:** Document platform requirements
- [ ] **Git tag:** `v0.14.3-platform-tests`

---

## Phase 15: Documentation & Release (Day 20)

### 15.1 User Documentation
- [ ] Write README.md with build instructions
- [ ] Create CONTROLS.md reference
- [ ] Add screenshots
- [ ] **Review:** Documentation is complete
- [ ] **Git tag:** `v0.15.1-documentation`

### 15.2 Developer Documentation
- [ ] Document code architecture
- [ ] Add inline code comments
- [ ] Create CONTRIBUTING.md
- [ ] **Review:** Code is well-documented
- [ ] **Git tag:** `v0.15.2-dev-docs`

### 15.3 Release Package
- [ ] Create release build
- [ ] Package with data files
- [ ] Create release notes
- [ ] **Final test:** Release package works
- [ ] **Git tag:** `v0.1.0-mvp-release`

---

## Testing Checklist (Run Before Each Merge)

- [ ] Code compiles without warnings
- [ ] Unit tests pass
- [ ] No memory leaks (valgrind/sanitizers)
- [ ] Game launches and quits cleanly
- [ ] New features work as expected
- [ ] No regressions in existing features

## Documentation Checklist (Update as Needed)

- [ ] Update CHANGELOG.md
- [ ] Update README if needed
- [ ] Document new JSON formats
- [ ] Update architecture docs
- [ ] Add code comments for complex logic

## Git Commit Message Format

```
[Phase X.Y] Brief description

- Detailed change 1
- Detailed change 2
- Tests: What was tested
- Docs: What was documented
```

## Estimated Timeline

- **Week 1:** Phases 0-5 (Foundation)
- **Week 2:** Phases 6-10 (Core Systems)
- **Week 3:** Phases 11-15 (Polish & Release)

## Success Criteria

1. All acceptance criteria from MVP documentation met
2. All tests passing
3. Runs on macOS, Linux, Windows
4. Save/load works reliably
5. No critical bugs
6. Documentation complete

---

## Notes

- Each phase should take 0.5-2 days maximum
- Test after every significant change
- Commit working code frequently
- Tag stable milestones for rollback points
- Keep the scope controlled - save features for post-MVP