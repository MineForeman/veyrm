# API Documentation Guide

## Overview

This document describes how to generate and maintain API documentation for the Veyrm codebase using Doxygen.

## Setup

### Installing Doxygen

#### macOS
```bash
brew install doxygen
brew install graphviz  # For class diagrams
```

#### Linux
```bash
sudo apt-get install doxygen
sudo apt-get install graphviz
```

#### Windows
Download from [Doxygen website](https://www.doxygen.nl/download.html)

## Configuration

### Doxyfile Configuration

Create a `Doxyfile` in the project root:

```bash
doxygen -g Doxyfile
```

### Recommended Settings

Edit the Doxyfile with these settings:

```ini
# Project settings
PROJECT_NAME           = "Veyrm"
PROJECT_NUMBER         = "1.0.0-MVP"
PROJECT_BRIEF          = "A modern roguelike inspired by Angband"
OUTPUT_DIRECTORY       = doc/api

# Input settings
INPUT                  = include src
RECURSIVE              = YES
FILE_PATTERNS          = *.h *.cpp
EXCLUDE_PATTERNS       = */tests/* */build/*

# Output settings
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
GENERATE_XML           = YES

# Extract settings
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = YES
EXTRACT_STATIC         = YES

# Graph settings
HAVE_DOT               = YES
CLASS_GRAPH            = YES
COLLABORATION_GRAPH    = YES
CALL_GRAPH             = YES
CALLER_GRAPH           = YES
```

## Documentation Standards

### Class Documentation

```cpp
/**
 * @brief Manages all entities in the game world
 *
 * The EntityManager is responsible for creating, updating, and destroying
 * all entities including the player, monsters, and items. It provides
 * efficient spatial queries and manages entity lifecycles.
 *
 * @see Entity
 * @see Player
 * @see Monster
 */
class EntityManager {
public:
    /**
     * @brief Constructs an EntityManager with optional initial capacity
     * @param initial_capacity Reserve space for this many entities
     */
    explicit EntityManager(size_t initial_capacity = 100);
```

### Method Documentation

```cpp
/**
 * @brief Calculates damage for a combat attack
 *
 * Uses d20 mechanics to determine hit success and damage dealt.
 * Critical hits (natural 20) deal double damage.
 *
 * @param attacker The entity making the attack
 * @param defender The entity being attacked
 * @return Damage dealt (minimum 1, 0 if miss)
 *
 * @note This method modifies combat state
 * @warning Attacker and defender must be valid
 *
 * Example:
 * @code
 * int damage = combat.calculateDamage(player, monster);
 * if (damage > 0) {
 *     monster.takeDamage(damage);
 * }
 * @endcode
 */
int calculateDamage(const Entity& attacker, const Entity& defender);
```

### Parameter Documentation

```cpp
/**
 * @brief Spawns a monster at the specified location
 *
 * @param[in] x X coordinate for spawn
 * @param[in] y Y coordinate for spawn
 * @param[in] type Monster type ID from monsters.json
 * @param[out] spawned Pointer to created monster (nullptr if failed)
 * @return true if spawn successful, false otherwise
 */
bool spawnMonster(int x, int y, const std::string& type, Monster** spawned);
```

### Enum Documentation

```cpp
/**
 * @brief AI states for monster behavior
 */
enum class AIState {
    IDLE,     ///< Default state, wanders randomly
    ALERT,    ///< Heard combat, moves toward sound
    HOSTILE,  ///< Can see player, actively pursues
    FLEEING,  ///< Low health, runs from player
    RETURNING ///< Lost player, returns to spawn area
};
```

## Special Commands

### Groups

```cpp
/**
 * @defgroup Combat Combat System
 * @brief D20-based combat mechanics
 * @{
 */

class CombatSystem { /* ... */ };
class Weapon { /* ... */ };
class Armor { /* ... */ };

/** @} */ // end of Combat group
```

### Todos and Bugs

```cpp
/**
 * @todo Implement ranged combat
 * @bug Critical hits don't work with magic weapons
 * @deprecated Use newMethod() instead
 * @warning This function is not thread-safe
 */
```

### Code Examples

```cpp
/**
 * @brief Main game loop example
 *
 * @code
 * GameManager game;
 * game.initialize();
 *
 * while (game.isRunning()) {
 *     game.processInput();
 *     game.update();
 *     game.render();
 * }
 *
 * game.cleanup();
 * @endcode
 */
```

## Generating Documentation

### Command Line

```bash
# Generate documentation
doxygen Doxyfile

# View HTML output
open doc/api/html/index.html
```

### Build Integration

Add to CMakeLists.txt:

```cmake
# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()
```

Then build with:
```bash
cmake --build build --target doc
```

### Build Script Integration

Add to build.sh:

```bash
doc)
    echo "Generating API documentation..."
    if command -v doxygen &> /dev/null; then
        doxygen Doxyfile
        echo "Documentation generated in doc/api/html/"
    else
        echo "Error: Doxygen not installed"
        exit 1
    fi
    ;;
```

## Best Practices

### What to Document

1. **Always Document**:
   - Public API (classes, methods, functions)
   - Complex algorithms
   - Non-obvious behavior
   - Parameters and return values
   - Exceptions thrown
   - Thread safety

2. **Consider Documenting**:
   - Private methods if complex
   - Implementation notes
   - Performance characteristics
   - Memory management

3. **Don't Document**:
   - Obvious getters/setters
   - Self-explanatory code
   - Implementation details users don't need

### Documentation Quality

1. **Be Concise**: One sentence brief, details in body
2. **Be Precise**: Specify exact behavior
3. **Use Examples**: Show common usage
4. **Cross-Reference**: Link related classes
5. **Keep Updated**: Update docs with code changes

### Common Tags

| Tag | Purpose |
|-----|---------|
| `@brief` | Short description |
| `@param` | Parameter description |
| `@return` | Return value description |
| `@throws` | Exceptions thrown |
| `@see` | Related items |
| `@note` | Additional information |
| `@warning` | Important cautions |
| `@todo` | Future improvements |
| `@bug` | Known issues |
| `@deprecated` | Obsolete features |

## Current Documentation Status

### Well Documented
- Core game systems
- Entity management
- Map generation

### Needs Documentation
- Combat formulas
- AI decision trees
- Save file format
- Network protocol (future)

## Maintenance

### Regular Tasks
1. Generate docs before releases
2. Review undocumented APIs
3. Update examples
4. Fix broken links
5. Remove obsolete documentation

### Documentation Reviews
- Check new PRs for documentation
- Ensure API changes update docs
- Verify examples still work
- Update diagrams as needed

## Tools and Extensions

### VS Code Extensions
- Doxygen Documentation Generator
- Doxygen Comment Generator

### Alternative Generators
- Sphinx (with Breathe)
- Natural Docs
- ROBODoc

## Example Output

The generated documentation includes:
- Class hierarchy
- Collaboration diagrams
- Call graphs
- File listings
- Module groupings
- Search functionality

## Troubleshooting

### Common Issues

1. **Missing Graphs**: Install Graphviz
2. **Broken Links**: Check file paths
3. **Missing Classes**: Check INPUT paths
4. **Slow Generation**: Disable call graphs
5. **Large Output**: Exclude test files

### Validation

```bash
# Check for undocumented items
doxygen -d Validate Doxyfile

# Count documentation coverage
grep -r "@brief" include/ | wc -l
```

## Future Enhancements

1. **Automated Generation**: CI/CD integration
2. **Coverage Reports**: Documentation metrics
3. **Interactive Diagrams**: PlantUML integration
4. **API Versioning**: Track API changes
5. **Online Hosting**: GitHub Pages deployment

---

*Last Updated: 2025-09-14*