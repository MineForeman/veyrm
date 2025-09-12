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

### 3.1 Entity Base
- [x] Create `Entity` base class with position
- [x] Create `Player` class extending Entity
- [x] Add glyph and color properties
- [x] Create `EntityManager` class for entity lifecycle
- [x] Integrate entities with GameManager
- [x] Update movement to use entity system
- [x] **Test:** Entities can be created and positioned
- [x] **Git tag:** `v0.3.1-entity-base`
- **Notes:** Complete entity system with Player class, EntityManager, and movement integration

### 3.2 Player on Map
- [ ] Add player to game state
- [ ] Render player on map
- [ ] Ensure player renders above floor
- [ ] **Test:** Player appears at spawn position
- [ ] **Git tag:** `v0.3.2-player-render`

### 3.3 Entity Manager
- [ ] Create `EntityManager` class
- [ ] Track all entities in single container
- [ ] Add entity lookup by position
- [ ] **Test:** Can add, remove, find entities
- [ ] **Documentation:** Document entity architecture
- [ ] **Git tag:** `v0.3.3-entity-manager`

---

## Phase 4: Movement System (Day 5)

### 4.1 Basic Movement
- [ ] Implement 4-directional movement (arrow keys)
- [ ] Add collision detection with walls
- [ ] Update player position on valid moves
- [ ] **Test:** Player moves in empty spaces, stops at walls
- [ ] **Git tag:** `v0.4.1-basic-movement`

### 4.2 8-Directional Movement
- [ ] Add diagonal movement (yubn keys)
- [ ] Implement hjkl vim-style movement
- [ ] Add wait command (.)
- [ ] **Test:** All 8 directions + wait work
- [ ] **Git tag:** `v0.4.2-eight-direction`

### 4.3 Movement Validation
- [ ] Create `MovementSystem` class
- [ ] Add bounds checking
- [ ] Add entity collision checking
- [ ] **Test:** Movement validation prevents invalid moves
- [ ] **Git tag:** `v0.4.3-movement-validation`

---

## Phase 5: Map Generation (Day 6-7)

### 5.1 Room Generation
- [ ] Create `Room` class with bounds
- [ ] Implement random room placement
- [ ] Add overlap detection
- [ ] **Test:** Generates non-overlapping rooms
- [ ] **Git tag:** `v0.5.1-room-generation`

### 5.2 Corridor Generation
- [ ] Implement L-shaped corridor algorithm
- [ ] Connect room centers
- [ ] Carve corridors in map
- [ ] **Test:** All rooms connected
- [ ] **Git tag:** `v0.5.2-corridors`

### 5.3 Map Validation
- [ ] Implement BFS connectivity check
- [ ] Ensure all floor tiles reachable
- [ ] Place stairs at farthest point
- [ ] **Test:** Map always fully connected
- [ ] **Documentation:** Document generation algorithm
- [ ] **Git tag:** `v0.5.3-map-validation`

---

## Phase 6: Field of View (Day 8)

### 6.1 FOV Algorithm
- [ ] Implement symmetric shadowcasting
- [ ] Create `FOV` class with calculate method
- [ ] Handle 8 octants properly
- [ ] **Test:** FOV calculates correctly
- [ ] **Git tag:** `v0.6.1-fov-algorithm`

### 6.2 Visibility System
- [ ] Add visible/revealed states to tiles
- [ ] Update visibility on player move
- [ ] Implement fog of war rendering
- [ ] **Test:** Only visible area shown
- [ ] **Git tag:** `v0.6.2-visibility`

### 6.3 Map Memory
- [ ] Track explored tiles
- [ ] Render explored but not visible tiles dimmed
- [ ] Persist exploration between turns
- [ ] **Test:** Map memory persists correctly
- [ ] **Git tag:** `v0.6.3-map-memory`

---

## Phase 7: UI Framework (Day 9)

### 7.1 Message Log
- [ ] Create `MessageLog` class
- [ ] Implement 5-line display
- [ ] Add message history storage
- [ ] **Test:** Messages display and scroll
- [ ] **Git tag:** `v0.7.1-message-log`

### 7.2 Status Bar
- [ ] Create status bar component
- [ ] Display HP, position, turn count
- [ ] Add color coding for HP levels
- [ ] **Test:** Status updates in real-time
- [ ] **Git tag:** `v0.7.2-status-bar`

### 7.3 Layout System
- [ ] Implement three-panel layout (map, log, status)
- [ ] Handle window resizing
- [ ] Ensure minimum 80x24 size
- [ ] **Test:** Layout responds to terminal size
- [ ] **Documentation:** Document UI components
- [ ] **Git tag:** `v0.7.3-layout`

---

## Phase 8: Monster System (Day 10-11)

### 8.1 Monster Entity
- [ ] Create `Monster` class extending Entity
- [ ] Add HP, attack, defense stats
- [ ] Load monster definitions from JSON
- [ ] **Test:** Monsters load from data files
- [ ] **Git tag:** `v0.8.1-monster-entity`

### 8.2 Monster Spawning
- [ ] Add monsters to map generation
- [ ] Implement spawn distribution
- [ ] Create Gutter Rat and Orc Rookling
- [ ] **Test:** Monsters spawn in valid locations
- [ ] **Git tag:** `v0.8.2-spawning`

### 8.3 Basic AI
- [ ] Implement simple chase behavior
- [ ] Add 4-direction pathfinding (BFS)
- [ ] Add random wander when no target
- [ ] **Test:** Monsters move toward player
- [ ] **Git tag:** `v0.8.3-basic-ai`

---

## Phase 9: Combat System (Day 12)

### 9.1 Combat Stats
- [ ] Add HP, attack, defense to all entities
- [ ] Create `CombatSystem` class
- [ ] Implement damage calculation
- [ ] **Test:** Damage calculates correctly
- [ ] **Git tag:** `v0.9.1-combat-stats`

### 9.2 Bump Combat
- [ ] Detect bump-to-attack
- [ ] Apply damage on collision
- [ ] Add combat messages to log
- [ ] **Test:** Bumping causes damage
- [ ] **Git tag:** `v0.9.2-bump-combat`

### 9.3 Death Handling
- [ ] Remove entities at 0 HP
- [ ] Handle player death (game over)
- [ ] Handle monster death (remove from map)
- [ ] **Test:** Death states work correctly
- [ ] **Documentation:** Document combat formulas
- [ ] **Git tag:** `v0.9.3-death-handling`

---

## Phase 10: Item System (Day 13-14)

### 10.1 Item Entity
- [ ] Create `Item` class
- [ ] Add item types (POTION, SCROLL)
- [ ] Load item definitions from JSON
- [ ] **Test:** Items load from data files
- [ ] **Git tag:** `v0.10.1-item-entity`

### 10.2 Ground Items
- [ ] Spawn items during map generation
- [ ] Render items on map
- [ ] Handle multiple items per tile
- [ ] **Test:** Items appear on map
- [ ] **Git tag:** `v0.10.2-ground-items`

### 10.3 Pickup System
- [ ] Implement 'g' get command
- [ ] Add items to inventory
- [ ] Remove items from map on pickup
- [ ] **Test:** Can pick up items
- [ ] **Git tag:** `v0.10.3-pickup`

---

## Phase 11: Inventory System (Day 15)

### 11.1 Inventory Storage
- [ ] Create `Inventory` class
- [ ] Implement 10-slot limit
- [ ] Add inventory to player
- [ ] **Test:** Inventory stores items correctly
- [ ] **Git tag:** `v0.11.1-inventory-storage`

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