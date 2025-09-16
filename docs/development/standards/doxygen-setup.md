# Doxygen Integration Plan

## Overview

Plan for integrating Doxygen to auto-generate API documentation from the Veyrm codebase.

## Prerequisites

### 1. Install Doxygen

```bash
# macOS
brew install doxygen
brew install graphviz  # For dependency graphs

# Linux
sudo apt-get install doxygen
sudo apt-get install graphviz

# Windows
# Download from https://www.doxygen.nl/download.html
```

## Implementation Steps

### Phase 1: Basic Setup

#### 1.1 Create Doxyfile

```bash
# Generate default configuration
doxygen -g Doxyfile

# Place in project root
mv Doxyfile /path/to/veyrm/
```

#### 1.2 Configure Doxyfile

Key settings to modify:

```
PROJECT_NAME           = "Veyrm"
PROJECT_NUMBER         = "1.0.0-MVP"
PROJECT_BRIEF          = "A modern roguelike game in C++"
OUTPUT_DIRECTORY       = docs/reference/api/generated
INPUT                  = include src
RECURSIVE              = YES
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = YES
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
GENERATE_TREEVIEW      = YES
HAVE_DOT               = YES
UML_LOOK               = YES
CALL_GRAPH             = YES
CALLER_GRAPH           = YES
```

### Phase 2: Code Documentation Standards

#### 2.1 Header Documentation Format

```cpp
/**
 * @file game_manager.h
 * @brief Central game state management
 * @author Veyrm Team
 * @date 2025
 */
```

#### 2.2 Class Documentation

```cpp
/**
 * @class GameManager
 * @brief Manages the main game loop and state
 *
 * The GameManager coordinates all game systems and maintains
 * the central game state. It handles initialization, updates,
 * and shutdown of all subsystems.
 *
 * @see Map
 * @see EntityManager
 */
class GameManager {
```

#### 2.3 Method Documentation

```cpp
/**
 * @brief Process one game turn
 *
 * Handles player input, updates entities, and renders the game.
 *
 * @param input The player's input command
 * @return true if the game should continue, false to quit
 *
 * @note This method modifies game state
 * @warning Must be called from main thread only
 */
bool processTurn(InputCommand input);
```

#### 2.4 Member Variable Documentation

```cpp
/// Current game state (MENU, PLAYING, PAUSED)
GameState m_state;

/// @brief Player entity reference
/// @details Managed by EntityManager, do not delete directly
Player* m_player;
```

### Phase 3: Documentation Coverage

#### 3.1 Priority Classes to Document

1. **Core Systems** (High Priority)
   - GameManager
   - Map
   - EntityManager
   - Renderer

2. **Entities** (High Priority)
   - Entity (base)
   - Player
   - Monster
   - Item

3. **Systems** (Medium Priority)
   - FOV
   - Pathfinding
   - CombatSystem
   - InventorySystem

4. **Utilities** (Low Priority)
   - Point
   - ColorScheme
   - MessageLog

#### 3.2 Documentation Checklist

- [ ] All public methods documented
- [ ] All public members documented
- [ ] Class overview provided
- [ ] Parameters described with @param
- [ ] Return values described with @return
- [ ] Exceptions documented with @throws
- [ ] Related classes linked with @see
- [ ] Examples provided with @code blocks

### Phase 4: Build Integration

#### 4.1 Add to build.sh

```bash
# Add new command to build.sh
docs)
    echo "Generating documentation..."
    if command -v doxygen &> /dev/null; then
        doxygen Doxyfile
        echo "Documentation generated in docs/reference/api/generated/"
    else
        echo "Error: Doxygen not installed"
        exit 1
    fi
    ;;
```

#### 4.2 CMake Integration (Optional)

```cmake
# Find Doxygen
find_package(Doxygen)

if(DOXYGEN_FOUND)
    # Configure Doxyfile
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                   ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    # Add documentation target
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()
```

### Phase 5: CI/CD Integration

#### 5.1 GitHub Actions Workflow

```yaml
name: Generate Documentation

on:
  push:
    branches: [main]
    paths:
      - 'include/**'
      - 'src/**'
      - 'Doxyfile'

jobs:
  doxygen:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install Doxygen
        run: |
          sudo apt-get update
          sudo apt-get install -y doxygen graphviz

      - name: Generate Documentation
        run: doxygen Doxyfile

      - name: Deploy to GitHub Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./docs/reference/api/generated/html
```

## Advanced Features

### Custom Styling

Create custom CSS for documentation:

```css
/* docs/reference/api/doxygen-custom.css */
.title {
    background-color: #2a2a2a;
    color: #00ff00;  /* Roguelike green */
}
```

### Markdown Support

Enable markdown in comments:

```
MARKDOWN_SUPPORT       = YES
AUTOLINK_SUPPORT       = YES
```

### Search Functionality

```
SEARCHENGINE           = YES
SERVER_BASED_SEARCH    = NO
```

### Dependency Graphs

```
HAVE_DOT               = YES
CLASS_DIAGRAMS         = YES
COLLABORATION_GRAPH    = YES
INCLUDE_GRAPH          = YES
INCLUDED_BY_GRAPH      = YES
GRAPHICAL_HIERARCHY    = YES
```

## Benefits

1. **Auto-generated documentation** from source code
2. **Always up-to-date** with code changes
3. **Interactive diagrams** showing relationships
4. **Searchable** API reference
5. **Cross-references** between related components
6. **Inheritance diagrams** for class hierarchies
7. **Call graphs** showing function relationships

## Maintenance

### Regular Tasks

- Run doxygen before releases
- Review undocumented items (`WARN_IF_UNDOCUMENTED = YES`)
- Update PROJECT_NUMBER for new versions
- Check for broken @see references
- Validate @param names match actual parameters

### Documentation Quality Metrics

- Target: >80% documentation coverage
- All public API documented
- Examples for complex methods
- Diagrams for key systems

## Timeline

1. **Week 1**: Basic setup and configuration
2. **Week 2**: Document core classes
3. **Week 3**: Document remaining systems
4. **Week 4**: Polish and deploy

## Next Steps

1. Install Doxygen locally
2. Create initial Doxyfile
3. Start documenting GameManager as pilot
4. Set up GitHub Pages deployment
5. Add documentation generation to release process

---

*Created: 2025-09-14*
*Status: Planning*
*Priority: Medium*
