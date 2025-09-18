# Testing Catch-Up Plan

## Current State Assessment

### Test Coverage Metrics (Updated)

- **Test Files**: 7 (test_main.cpp, test_basic.cpp, test_json.cpp, test_map.cpp, test_entity.cpp, test_player.cpp, test_entity_manager.cpp)
- **Test Cases**: 26 test cases with 195 assertions
- **Production Classes**: 19 classes
- **Classes with Tests**: 4/19 (21% coverage) - Map, Entity, Player, EntityManager
- **Completed Phases**: 0.1-0.4, 1.1-1.3, 2.1-2.3, 3.1
- **Phases with Tests**: 2/11 (18% coverage) - Partial coverage for Phase 2 (Map) and Phase 3.1 (Entity System)

### Risk Assessment

- **Critical**: No tests for core game loop or map generation
- **High**: No tests for turn management, input handling, or game state
- **Medium**: Limited tests for movement and collision (basic coverage exists)
- **Medium**: No tests for rendering, input handling, or save/load
- **Low**: Basic C++ and JSON functionality (already tested)

---

## Testing Strategy

### Principles

1. **Test What's Built**: Focus on completed phases first
2. **Risk-Based Priority**: Test critical path functionality first
3. **Incremental Coverage**: Add tests phase by phase
4. **Maintainable Tests**: Keep tests simple and focused
5. **Fast Feedback**: Unit tests should run in <1 second total

### Test Organization

```
tests/
├── test_main.cpp          # Catch2 main (existing)
├── test_basic.cpp         # Basic tests (existing)
├── test_json.cpp          # JSON tests (existing)
├── core/
│   ├── test_game_state.cpp
│   ├── test_input_handler.cpp
│   └── test_turn_manager.cpp
├── map/
│   ├── test_map.cpp
│   ├── test_map_generator.cpp
│   ├── test_map_validator.cpp
│   └── test_tile.cpp
├── entity/
│   ├── test_entity.cpp
│   ├── test_player.cpp
│   └── test_entity_manager.cpp
├── render/
│   ├── test_renderer.cpp
│   └── test_color_scheme.cpp
└── util/
    ├── test_point.cpp
    └── test_message_log.cpp
```

---

## Progress Update

### Completed (Phase 1)

- ✅ Created `test_map.cpp` with 6 test cases covering:
  - Tile operations, bounds checking, visibility/exploration
  - Collision detection, tile properties, custom sizes
- ✅ Created `test_entity.cpp` with 5 test cases covering:
  - Basic properties, movement validation, component flags
  - Virtual functions, entity type variations
- ✅ Created `test_player.cpp` with 5 test cases covering:
  - Initialization, movement, entity collision
  - Stat modifications, special movement cases
- ✅ Created `test_entity_manager.cpp` with 6 test cases covering:
  - Basic operations, player management, spatial queries
  - Blocking entities, entity removal, updates

## Implementation Phases

### Phase 1: Core Foundation Tests (Priority: CRITICAL)

**Timeline**: 2-3 hours
**Coverage Target**: 60% of core systems
**Status**: PARTIALLY COMPLETE

#### 1.1 Map System Tests

- [x] Create `test_map.cpp`
  - Test tile get/set operations
  - Test bounds checking
  - Test visibility and exploration tracking
  - Test collision detection
- [ ] Create `test_tile.cpp`
  - Test tile properties (walkable, opaque, etc.)
  - Test tile type conversions
  - Test glyph and color mappings

#### 1.2 Entity System Tests

- [ ] Create `test_entity.cpp`
  - Test entity creation and positioning
  - Test movement validation
  - Test collision detection
  - Test rendering properties
- [ ] Create `test_player.cpp`
  - Test player initialization (stats, position)
  - Test movement mechanics
  - Test stat modifications
  - Test level/experience calculations
- [ ] Create `test_entity_manager.cpp`
  - Test entity creation and deletion
  - Test entity queries (by position, by type)
  - Test player singleton management
  - Test entity updates

#### 1.3 Game State Tests

- [ ] Create `test_game_state.cpp`
  - Test state transitions
  - Test previous state tracking
  - Test state validation

### Phase 2: Map Generation Tests (Priority: HIGH)

**Timeline**: 2 hours
**Coverage Target**: 70% of map generation

#### 2.1 Generator Tests

- [ ] Create `test_map_generator.cpp`
  - Test room generation (no overlaps)
  - Test corridor generation (proper walls)
  - Test connectivity (all rooms reachable)
  - Test spawn point placement
  - Test each map type (room, dungeon, corridor, arena, stress)

#### 2.2 Validator Tests

- [ ] Create `test_map_validator.cpp`
  - Test connectivity validation
  - Test spawn point validation
  - Test wall integrity checks
  - Test minimum walkable space

### Phase 3: Game Logic Tests (Priority: HIGH)

**Timeline**: 2 hours
**Coverage Target**: 50% of game logic

#### 3.1 Turn System Tests

- [ ] Create `test_turn_manager.cpp`
  - Test turn incrementing
  - Test action speed calculations
  - Test world time tracking
  - Test player/world turn distinction

#### 3.2 Input Handler Tests

- [ ] Create `test_input_handler.cpp`
  - Test keyboard mapping
  - Test action conversion
  - Test invalid input handling
  - Test state-specific input

#### 3.3 Message Log Tests

- [ ] Create `test_message_log.cpp`
  - Test message addition
  - Test message history
  - Test color coding
  - Test maximum message limit

### Phase 4: Utility Tests (Priority: MEDIUM)

**Timeline**: 1 hour
**Coverage Target**: 80% of utilities

#### 4.1 Point Tests

- [ ] Create `test_point.cpp`
  - Test point arithmetic
  - Test distance calculations
  - Test equality comparisons

#### 4.2 Color Scheme Tests

- [ ] Create `test_color_scheme.cpp`
  - Test theme selection
  - Test color mappings
  - Test terminal detection

### Phase 5: Integration Tests (Priority: MEDIUM)

**Timeline**: 2 hours
**Coverage Target**: Key workflows

#### 5.1 Movement Integration

- [ ] Test player movement through map
- [ ] Test collision with walls
- [ ] Test entity blocking
- [ ] Test turn advancement

#### 5.2 Map Generation Integration

- [ ] Test full dungeon generation
- [ ] Test player spawn placement
- [ ] Test map validation pass

---

## Test Implementation Guidelines

### Test Structure Template

```cpp
#include <catch2/catch_test_macros.hpp>
#include "component_under_test.h"

TEST_CASE("Component: Feature description", "[component]") {
    // Arrange
    ComponentClass component;
    
    SECTION("Specific behavior") {
        // Act
        auto result = component.method();
        
        // Assert
        REQUIRE(result == expected);
    }
    
    SECTION("Edge case handling") {
        REQUIRE_THROWS_AS(component.invalidOp(), std::exception);
    }
}
```

### Testing Best Practices

1. **Isolation**: Each test should be independent
2. **Clarity**: Test names describe what's being tested
3. **Focus**: One assertion per test when possible
4. **Speed**: Mock expensive operations
5. **Coverage**: Test happy path, edge cases, and errors

---

## Execution Plan

### Week 1: Foundation (Days 1-3)

- **Day 1**: Implement Phase 1.1 (Map System Tests)
- **Day 2**: Implement Phase 1.2 (Entity System Tests)
- **Day 3**: Implement Phase 1.3 (Game State Tests)

### Week 1: Core Systems (Days 4-5)

- **Day 4**: Implement Phase 2 (Map Generation Tests)
- **Day 5**: Implement Phase 3 (Game Logic Tests)

### Week 2: Completion (Days 6-7)

- **Day 6**: Implement Phase 4 (Utility Tests)
- **Day 7**: Implement Phase 5 (Integration Tests)

---

## Success Metrics

### Minimum Viable Testing

- [ ] All completed phases have basic tests
- [ ] Core game loop has integration test
- [ ] Map generation validated by tests
- [ ] Entity system fully tested
- [ ] All tests pass consistently
- [ ] Test execution time < 1 second

### Target Coverage

- **Line Coverage**: >70% of production code
- **Branch Coverage**: >60% of decision points
- **Function Coverage**: >80% of public methods

### Quality Indicators

- No flaky tests (100% consistent pass rate)
- Clear test failure messages
- Tests serve as documentation
- New features have tests before merge

---

## Continuous Testing Strategy

### Going Forward

1. **Test-First Development**: Write tests for new phases before implementation
2. **Regression Prevention**: Add test for every bug fix
3. **Coverage Monitoring**: Track coverage metrics in CI
4. **Test Review**: Include test review in code review
5. **Performance Testing**: Add benchmarks for critical paths

### CI/CD Integration

```yaml
# Future .github/workflows/test.yml
steps:
  - name: Build
    run: ./build.sh build
  - name: Test
    run: ./build.sh test
  - name: Coverage
    run: ./build.sh coverage
```

---

## Priority Order Summary

### Must Have (This Week)

1. Map system tests
2. Entity system tests
3. Map generation tests
4. Basic integration tests

### Should Have (Next Week)

1. Turn system tests
2. Input handler tests
3. Game state tests
4. Map validator tests

### Nice to Have (Future)

1. Rendering tests
2. Save/load tests
3. Performance benchmarks
4. Fuzz testing for map generation

---

## Notes

- Focus on testing behavior, not implementation
- Tests should enable refactoring, not prevent it
- Each test file should compile independently
- Use fixtures for common test setup
- Consider property-based testing for map generation
- Add memory leak detection in debug builds
