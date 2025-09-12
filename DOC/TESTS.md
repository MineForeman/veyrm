# Veyrm Test Suite Documentation

Complete documentation of the test suite for the Veyrm roguelike game.

## Overview

The Veyrm project uses Catch2 v3.5.1 for comprehensive unit testing. All tests must pass before any new feature or bug fix is considered complete.

**Current Status:** ✅ All tests passing  
**Test Cases:** 57  
**Assertions:** 404

## Running Tests

### Quick Commands

```bash
# Run all tests
./build.sh test

# Run all tests with detailed output
./build/bin/veyrm_tests --verbosity high

# Run specific test category
./build/bin/veyrm_tests "[entity]"
./build/bin/veyrm_tests "[map_generator]"

# Run specific test section
./build/bin/veyrm_tests -c "Player: Movement"

# Clean build and test
./build.sh clean build test
```

### Build Script Integration

The `build.sh` script provides convenient test commands:

```bash
./build.sh test           # Run all tests
./build.sh keys "q\n"     # Test with automated input
./build.sh dump           # Frame dump mode for analysis
```

## Test Files Overview

### Core System Tests

| File | Test Cases | Focus |
|------|------------|-------|
| `test_basic.cpp` | 2 | Basic functionality and build verification |
| `test_json.cpp` | 1 | JSON library integration |
| `test_main.cpp` | 1 | Main entry point testing |

### Entity System Tests

| File | Test Cases | Focus |
|------|------------|-------|
| `test_entity.cpp` | 6 | Base Entity class functionality |
| `test_entity_manager.cpp` | 6 | Entity lifecycle management |
| `test_player.cpp` | 6 | Player-specific functionality |

### Map System Tests

| File | Test Cases | Focus |
|------|------------|-------|
| `test_map.cpp` | 6 | Map tile storage and properties |
| `test_map_generator.cpp` | 6 | Map generation and validation |
| `test_map_validator.cpp` | 7 | Map connectivity and validation |

### UI and Input Tests

| File | Test Cases | Focus |
|------|------------|-------|
| `test_input_handler.cpp` | 6 | Keyboard input processing |
| `test_message_log.cpp` | 6 | Message logging and display |

### Game Logic Tests

| File | Test Cases | Focus |
|------|------------|-------|
| `test_turn_manager.cpp` | 4 | Turn-based game logic |

## Detailed Test Breakdown

### Entity System (`test_entity.cpp`)

Tests the base Entity class and its core functionality:

- **Entity: Basic properties** - Tests default entity initialization
- **Entity: Position** - Validates position tracking and updates  
- **Entity: Entity types** - Confirms proper entity type handling
- **Entity: Glyph and color** - Tests visual representation properties
- **Entity: Collision properties** - Validates movement blocking behavior
- **Entity: Property modifications** - Tests runtime property changes

### Entity Manager (`test_entity_manager.cpp`)

Tests the EntityManager class responsible for entity lifecycle:

- **EntityManager: Player creation** - Tests player entity creation
- **EntityManager: Monster creation** - Tests monster entity creation
- **EntityManager: Item creation** - Tests item entity creation
- **EntityManager: Entity removal** - Tests entity cleanup
- **EntityManager: Position queries** - Tests spatial entity queries
- **EntityManager: Entity counting** - Tests entity tracking

### Player (`test_player.cpp`)

Tests Player-specific functionality extending Entity:

- **Player: Initialization** - Tests player setup and default stats
- **Player: Movement** - Tests movement validation and collision
- **Player: Collision with entities** - Tests entity-to-entity collision
- **Player: Stat modifications** - Tests HP, level, and combat stats
- **Player: Special movement cases** - Tests edge cases and diagonal movement

### Map System (`test_map.cpp`)

Tests the core Map class and tile management:

- **Map: Basic operations** - Tests map creation and tile access
- **Map: Bounds checking** - Tests coordinate validation
- **Map: Tile properties** - Tests tile type properties and walkability
- **Map: Visibility system** - Tests tile visibility and exploration
- **Map: Map dimensions** - Tests map size handling
- **Map: Color and rendering** - Tests tile foreground/background colors

### Map Generation (`test_map_generator.cpp`)

Tests procedural map generation across all map types:

- **MapGenerator: Test room generation** - Tests single room generation
- **MapGenerator: Test dungeon generation** - Tests multi-room dungeons with stairs
- **MapGenerator: Corridor test** - Tests corridor generation and connectivity
- **MapGenerator: Safe spawn point** - Tests spawn point validation
- **MapGenerator: All map types generate** - Tests all 5 map types
- **MapGenerator: Map type spawn points** - Tests spawn points are walkable

### Map Validation (`test_map_validator.cpp`)

Tests map quality assurance and connectivity:

- **MapValidator: Basic validation** - Tests walkable tile detection
- **MapValidator: Connectivity checking** - Tests BFS connectivity verification
- **MapValidator: Wall counting** - Tests wall tile counting
- **MapValidator: Stairs detection** - Tests stairs placement validation
- **MapValidator: Room counting** - Tests room detection algorithms
- **MapValidator: Generated maps validation** - Tests all generated maps are valid
- **MapValidator: Error reporting** - Tests validation error messages

### Input Handler (`test_input_handler.cpp`)

Tests keyboard input processing and command mapping:

- **InputHandler: Movement keys** - Tests arrow key movement commands
- **InputHandler: Action keys** - Tests game action commands (quit, wait, inventory)
- **InputHandler: Special keys** - Tests special keys (Escape, Enter)
- **InputHandler: Unknown keys** - Tests handling of unmapped keys
- **InputHandler: Common input patterns** - Tests typical gameplay input sequences
- **InputHandler: Key mapping consistency** - Tests command consistency

### Message Log (`test_message_log.cpp`)

Tests the message logging system:

- **MessageLog: Basic message addition** - Tests message storage
- **MessageLog: Message types** - Tests combat and system message prefixes
- **MessageLog: Message limit** - Tests message history limits and overflow
- **MessageLog: Clear messages** - Tests log clearing functionality
- **MessageLog: Render output** - Tests FTXUI rendering integration
- **MessageLog: Special characters** - Tests Unicode and long message handling

### Turn Manager (`test_turn_manager.cpp`)

Tests turn-based game logic:

- **TurnManager: Basic turn tracking** - Tests turn counter functionality
- **TurnManager: Action processing** - Tests action-based turn advancement
- **TurnManager: Turn state** - Tests turn state management
- **TurnManager: Action timing** - Tests different action speeds

### Basic Tests (`test_basic.cpp`)

Tests fundamental build and runtime functionality:

- **Basic: Build verification** - Tests that the build system works
- **Basic: Integer arithmetic** - Tests basic mathematical operations

### JSON Integration (`test_json.cpp`)

Tests JSON library integration:

- **JSON: Basic parsing** - Tests nlohmann/json functionality

### Main Entry Point (`test_main.cpp`)

Tests application entry point:

- **Main: Entry point** - Tests main function execution

## Test Categories

Tests are organized by Catch2 tags for selective execution:

### System Categories

- `[basic]` - Fundamental build verification
- `[json]` - JSON library integration  
- `[main]` - Application entry point

### Core Game Systems

- `[entity]` - Entity system tests
- `[entity_manager]` - Entity lifecycle management
- `[player]` - Player-specific functionality
- `[map]` - Map storage and tile management
- `[map_generator]` - Procedural map generation
- `[map_validator]` - Map quality validation

### UI and Interaction

- `[input_handler]` - Input processing and commands
- `[message_log]` - Message logging and display
- `[turn_manager]` - Turn-based game logic

## Test Execution Examples

### Run by Category

```bash
# Test all entity-related functionality
./build/bin/veyrm_tests "[entity]"

# Test map generation and validation
./build/bin/veyrm_tests "[map_generator]" "[map_validator]"

# Test UI systems
./build/bin/veyrm_tests "[input_handler]" "[message_log]"
```

### Run Specific Tests

```bash
# Test only movement functionality
./build/bin/veyrm_tests -c "Player: Movement"

# Test map generation for all types
./build/bin/veyrm_tests -c "All map types generate"

# Test message logging limits
./build/bin/veyrm_tests -c "Message limit"
```

### Verbose Output

```bash
# Show detailed test execution
./build/bin/veyrm_tests --verbosity high

# Show successful tests too
./build/bin/veyrm_tests -s
```

## Continuous Integration

### Pre-commit Testing

All tests must pass before code commits:

```bash
# Standard pre-commit workflow
./build.sh clean
./build.sh build
./build.sh test
```

### Build Script Integration

The build script enforces testing requirements:

```bash
# These commands run tests automatically
./build.sh test      # Explicit test run
./build.sh ci        # CI-style build with tests
```

## Test Development Guidelines

### Writing New Tests

When adding new functionality, follow these patterns:

1. **Create test file:** `tests/test_[component].cpp`
2. **Use descriptive names:** `TEST_CASE("Component: Feature description", "[tag]")`
3. **Group related tests:** Use `SECTION()` for related test variants
4. **Test edge cases:** Include boundary conditions and error cases
5. **Verify state:** Use `REQUIRE()` for critical assertions

### Test Structure Example

```cpp
#include <catch2/catch_test_macros.hpp>
#include "your_component.h"

TEST_CASE("Component: Feature description", "[component]") {
    YourComponent component;
    
    SECTION("Normal operation") {
        // Test typical usage
        REQUIRE(component.doSomething() == expected_result);
    }
    
    SECTION("Edge cases") {
        // Test boundary conditions
        REQUIRE(component.handleEdgeCase() == expected_edge_result);
    }
    
    SECTION("Error conditions") {
        // Test error handling
        REQUIRE_THROWS_AS(component.invalidOperation(), std::exception);
    }
}
```

## Test Coverage

### Current Coverage Areas

✅ **Well Covered:**
- Entity system (creation, properties, movement)
- Map system (generation, validation, rendering) 
- Input handling (keyboard commands, edge cases)
- Message logging (types, limits, formatting)
- Turn management (timing, state tracking)

🔄 **Partially Covered:**
- JSON integration (basic parsing only)
- Rendering system (limited UI testing)
- Error handling (some edge cases)

⏳ **Future Coverage:**
- Combat system (not yet implemented)
- Field of view system (planned)
- Save/load functionality (planned)
- AI systems (planned)

### Adding Test Coverage

When implementing new features:

1. Write tests first (TDD approach)
2. Ensure 100% pass rate before merge
3. Add edge case and error condition tests
4. Update this documentation

## Performance Testing

### Stress Testing

The test suite includes performance validation:

```bash
# Run stress test map generation
./build/bin/veyrm_tests -c "STRESS_TEST generates without crash"

# Test large message volumes  
./build/bin/veyrm_tests -c "Maximum message history"
```

### Automated Testing

Use the build script for automated gameplay testing:

```bash
# Test basic movement sequence
./build.sh keys "→→→→←←←←q\n"

# Test menu navigation
./build.sh keys "\n\u\u\r\dq"

# Test frame dump analysis
./build.sh dump
```

## Debugging Failed Tests

### Common Issues

1. **Build Problems:** Ensure clean build with `./build.sh clean build`
2. **Test Data:** Check that test expectations match actual implementation
3. **Timing Issues:** Some tests may be sensitive to initialization order
4. **Platform Differences:** Test on target platforms

### Debugging Commands

```bash
# Run single failing test with verbose output
./build/bin/veyrm_tests -c "Test Name" --verbosity high

# Run test with debugger
gdb --args ./build/bin/veyrm_tests -c "Test Name"

# Check for memory issues
valgrind ./build/bin/veyrm_tests -c "Test Name"
```

## Test Maintenance

### Regular Updates

- Update tests when changing functionality
- Add tests for new features before implementation  
- Remove obsolete tests when removing features
- Keep test documentation current

### Best Practices

- Keep tests fast and focused
- Avoid external dependencies in tests
- Make tests deterministic and repeatable
- Use descriptive test and section names
- Group related tests logically

## Integration with Build System

### CMake Integration

Tests are automatically built with the main project:

```cmake
# Tests are included in CMakeLists.txt
add_executable(veyrm_tests ${TEST_SOURCES})
target_link_libraries(veyrm_tests PRIVATE Catch2::Catch2WithMain)
```

### Build Script Integration

The `build.sh` script provides test automation:

```bash
./build.sh test    # Run all tests
./build.sh ci      # Continuous integration workflow
./build.sh keys    # Automated input testing
./build.sh dump    # Frame analysis testing
```

## Conclusion

The Veyrm test suite provides comprehensive coverage of all implemented systems. All 57 test cases with 404 assertions must pass before any code is considered complete. This ensures code quality, prevents regressions, and maintains system reliability throughout development.

For questions about specific tests or to report test-related issues, refer to the test source files in the `tests/` directory or consult the implementation documentation.

---

*Last Updated: Phase 3.1 - Entity Base Complete*  
*Test Status: ✅ All 57 tests passing (404 assertions)*