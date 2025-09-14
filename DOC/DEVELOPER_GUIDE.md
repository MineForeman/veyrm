# Veyrm Developer Guide

## Getting Started

Welcome to the Veyrm development team! This guide will help you set up your development environment and understand the codebase.

## Prerequisites

### Required Software
- **C++ Compiler**: C++23 compatible (GCC 12+, Clang 15+, MSVC 2022+)
- **CMake**: Version 3.25 or higher
- **Git**: For version control
- **Terminal**: UTF-8 capable with 256-color support

### Recommended Tools
- **IDE**: Visual Studio Code, CLion, or Vim/Neovim
- **Debugger**: GDB or LLDB
- **Profiler**: Valgrind or Instruments (macOS)
- **Documentation**: Doxygen for API docs

## Setting Up

### 1. Clone the Repository
```bash
git clone git@github.com:MineForeman/veyrm.git
cd veyrm
```

### 2. Build the Project
```bash
# Quick method using build script
./build.sh build

# Or manually with CMake
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j
cd ..
```

### 3. Run Tests
```bash
# Using build script
./build.sh test

# Or directly
./build/bin/veyrm_tests
```

### 4. Run the Game
```bash
# Using build script
./build.sh run

# Or directly (from project root)
./build/bin/veyrm
```

## Project Structure

```
veyrm/
├── include/         # Header files (.h)
│   ├── core/       # Core systems (game_manager, etc.)
│   ├── entities/   # Entity classes
│   ├── map/        # Map-related classes
│   ├── ui/         # UI components
│   └── utils/      # Utilities
├── src/            # Implementation files (.cpp)
├── tests/          # Unit tests
├── data/           # Game data (JSON)
├── DOC/            # Documentation
├── logs/           # Runtime logs (generated)
├── saves/          # Save games (generated)
├── tmp/            # Temporary files (gitignored)
└── build/          # Build output (generated)
```

## Architecture Overview

See [DOC/ARCHITECTURE.md](ARCHITECTURE.md) for detailed architecture documentation.

### Key Components
- **GameManager**: Central game coordinator
- **Map**: World representation (198x66 tiles)
- **EntityManager**: Entity lifecycle management
- **TurnManager**: Turn-based system
- **CombatSystem**: Combat resolution
- **Renderer**: FTXUI-based UI

## Development Workflow

### 1. Creating a New Feature

#### Branch Strategy
```bash
# Create feature branch from main
git checkout -b feature/your-feature-name

# Or bugfix branch
git checkout -b bugfix/issue-description
```

#### Implementation Steps
1. Write tests first (TDD approach)
2. Implement the feature
3. Run tests to verify
4. Update documentation
5. Create pull request

### 2. Adding a New System

#### Example: Adding a Magic System
1. Create header in `include/magic_system.h`
2. Implement in `src/magic_system.cpp`
3. Write tests in `tests/test_magic_system.cpp`
4. Update CMakeLists.txt
5. Integrate with GameManager
6. Document in DOC/

### 3. Adding Content

#### New Monster Type
Edit `data/monsters.json`:
```json
{
  "id": "dragon",
  "name": "Ancient Dragon",
  "glyph": "D",
  "color": "red",
  "hp": 100,
  "attack": 15,
  "defense": 10,
  "speed": 90,
  "xp_value": 500
}
```

#### New Item Type
Edit `data/items.json`:
```json
{
  "id": "sword_flame",
  "name": "Flaming Sword",
  "glyph": "|",
  "color": "orange",
  "type": "weapon",
  "damage": 10,
  "value": 500
}
```

## Testing

### Running Tests
```bash
# All tests
./build.sh test

# Specific test file
./build/bin/veyrm_tests "[map]"

# With verbose output
./build/bin/veyrm_tests -s
```

### Writing Tests
Tests use Catch2 framework. Example:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "your_class.h"

TEST_CASE("YourClass: Basic functionality", "[your_class]") {
    YourClass obj;

    SECTION("Initialization") {
        REQUIRE(obj.getValue() == 0);
    }

    SECTION("Setting values") {
        obj.setValue(42);
        REQUIRE(obj.getValue() == 42);
    }
}
```

### Test Categories
- `[map]` - Map generation and validation
- `[entity]` - Entity management
- `[combat]` - Combat calculations
- `[save]` - Save/load functionality
- `[ui]` - UI components
- `[input]` - Input handling

## Debugging

### Enable Debug Mode
```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Run with debug output
./build/bin/veyrm --debug
```

### Using GDB
```bash
gdb ./build/bin/veyrm
(gdb) break GameManager::update
(gdb) run
(gdb) bt  # backtrace on crash
```

### Logging
Check logs in `logs/` directory:
- `veyrm_debug.log` - All events
- `veyrm_player.log` - Player actions
- `veyrm_ai.log` - AI decisions
- `veyrm_combat.log` - Combat events

### Memory Debugging
```bash
# Using valgrind
valgrind --leak-check=full ./build/bin/veyrm

# Using sanitizers
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" ..
```

## Code Style

### General Guidelines
- Use modern C++23 features
- Prefer smart pointers over raw pointers
- RAII for resource management
- Const-correctness throughout
- Clear, descriptive names

### Naming Conventions
- **Classes**: PascalCase (`GameManager`)
- **Methods**: camelCase (`updatePosition`)
- **Members**: snake_case with trailing underscore (`player_position_`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_HEALTH`)
- **Files**: snake_case (`game_manager.cpp`)

### Example Class
```cpp
class Monster : public Entity {
public:
    Monster(int x, int y, const MonsterData& data);

    void update(float delta_time) override;
    void takeDamage(int amount);

    int getHealth() const { return current_hp_; }
    bool isAlive() const { return current_hp_ > 0; }

private:
    int current_hp_;
    int max_hp_;
    AIState ai_state_;

    static constexpr int DEFAULT_SPEED = 100;
};
```

## Common Tasks

### Adding a New Command
1. Add to `InputAction` enum in `input_handler.h`
2. Add key binding in `input_handler.cpp`
3. Handle in `game_screen.cpp`
4. Update `DOC/CONTROLS.md`
5. Add test in `test_input_handler.cpp`

### Creating a New Map Type
1. Add enum value to `MapType`
2. Implement generation in `MapGenerator`
3. Add CLI argument in `main.cpp`
4. Update build script menu
5. Document in map generation docs

### Implementing a New UI Panel
1. Create component in `game_screen.cpp`
2. Add to layout system
3. Handle input events
4. Update render method
5. Test with different terminal sizes

## Performance Optimization

### Profiling
```bash
# Generate profile data
./build.sh profile

# Analyze with gprof
gprof ./build/bin/veyrm gmon.out
```

### Common Optimizations
- Use const references for large objects
- Minimize allocations in hot paths
- Cache frequently accessed data
- Use appropriate data structures
- Profile before optimizing

## Documentation

### Code Documentation
Use Doxygen-style comments:
```cpp
/**
 * @brief Calculates damage for an attack
 * @param attacker The attacking entity
 * @param defender The defending entity
 * @return Damage dealt (minimum 1)
 */
int calculateDamage(const Entity& attacker, const Entity& defender);
```

### Updating Documentation
- Update relevant MD files in `DOC/`
- Keep README.md current
- Update CHANGELOG.md
- Document breaking changes

## Troubleshooting

### Build Issues
```bash
# Clean rebuild
./build.sh clean
./build.sh build

# Update dependencies
rm -rf build
mkdir build && cd build
cmake ..
```

### Runtime Issues
- Check `logs/veyrm_debug.log` for errors
- Verify data files exist in `data/`
- Ensure terminal supports UTF-8
- Check save file compatibility

### Test Failures
- Run specific failing test in isolation
- Check for hardcoded paths
- Verify test data files
- Review recent changes

## Contributing

### Before Submitting
1. Run all tests: `./build.sh test`
2. Check for warnings: Build with `-Wall -Wextra`
3. Update documentation
4. Format code consistently
5. Write descriptive commit messages

### Commit Message Format
```
Type: Brief description

Detailed explanation if needed.
Multiple lines are fine.

Fixes #123
```

Types: feat, fix, docs, test, refactor, perf, chore

### Pull Request Process
1. Create feature branch
2. Make changes with tests
3. Push to your fork
4. Open PR against main
5. Address review feedback
6. Squash and merge

## Resources

### Internal Documentation
- [Architecture](ARCHITECTURE.md)
- [Controls](CONTROLS.md)
- [Implementation Plan](IMPLEMENTATION_PLAN.md)
- [Current State](CURRENT_STATE.md)

### External Resources
- [FTXUI Documentation](https://github.com/ArthurSonzogni/FTXUI)
- [Catch2 Tutorial](https://github.com/catchorg/Catch2)
- [CMake Documentation](https://cmake.org/documentation/)
- [C++23 Reference](https://en.cppreference.com/)

## Getting Help

### Project Resources
- GitHub Issues: Report bugs and request features
- Documentation: Check DOC/ directory
- Code Comments: Read inline documentation
- Tests: Learn from test cases

### Community
- Create issues for questions
- Check existing issues first
- Provide minimal reproducible examples
- Be respectful and constructive

## License

This project is currently private. Do not distribute without permission.

---

*Last Updated: 2025-09-14*
*Version: 1.0.0-MVP*