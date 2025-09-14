# Veyrm Documentation

## Quick Navigation

### For Players
- [Game Controls](CONTROLS.md) - Complete key reference
- [Getting Started](../README.md) - Installation and basic gameplay

### For Developers
- [Developer Guide](DEVELOPER_GUIDE.md) - Setup and development workflow
- [Architecture Overview](ARCHITECTURE.md) - System design and components
- [Code Style Guide](CODE_STYLE_GUIDE.md) - Coding standards and conventions
- [API Documentation](API_DOCUMENTATION.md) - Doxygen setup and usage

### Project Information
- [Current State](CURRENT_STATE.md) - Project status and features
- [Implementation Plan](IMPLEMENTATION_PLAN.md) - Development roadmap
- [Changelog](../CHANGELOG.md) - Version history

### Technical Documentation
- [Performance Benchmarks](PERFORMANCE_BENCHMARKS.md) - Metrics and optimization
- [Build System](BUILD_SCRIPT.md) - Build script documentation
- [Configuration](CONFIGURATION.md) - Game configuration options
- [Testing](TESTS.md) - Test suite documentation

### Game Systems
- [Map Generation](SPEC.md#map-generation) - Procedural generation
- [Combat System](AI_ARCHITECTURE.md) - Combat mechanics
- [Monster AI](AI_ARCHITECTURE.md) - AI behavior system
- [Item System](ARCHITECTURE/ITEM_SYSTEM.md) - Items and inventory
- [Save/Load System](ARCHITECTURE.md#saveload-system) - Persistence

### Features
- [Logging System](LOGGING_SYSTEM.md) - Debug and event logging
- [Door System](DOOR_SYSTEM.md) - Interactive doors
- [Lit Rooms](FEATURES/LIT_ROOMS.md) - Room lighting
- [Visualization](VISUALIZATION_OPTIONS.md) - Data visualization tools

### World & Lore
- [World Overview](WORLD/README.md) - Game world and setting
- [Foes](WORLD/07_foes.md) - Monster descriptions
- [Key NPCs](WORLD/10_key_npcs.md) - Important characters

### MVP Documentation
- [MVP Scope](MVP/01_scope.md) - MVP requirements
- [MVP Complete](MVP_COMPLETE.md) - Completion announcement
- [MVP Summary](../MVP_SUMMARY.md) - Feature summary

### Development Phases
- [Phase Overview](PHASES/README.md) - Development phases
- [Archived Phases](PHASES/ARCHIVE/) - Completed phase documentation

### Refactoring
- [Documentation Improvements](REFACTORING/DOCUMENTATION_IMPROVEMENTS.md) - Doc refactoring plan
- [Refactoring Summary](REFACTORING/REFACTORING_SUMMARY.md) - Changes made

## Documentation Structure

```
DOC/
├── README.md                    # This file - navigation hub
├── ARCHITECTURE.md              # System architecture
├── CONTROLS.md                  # Consolidated controls
├── DEVELOPER_GUIDE.md           # Developer onboarding
├── CODE_STYLE_GUIDE.md          # Coding standards
├── API_DOCUMENTATION.md         # API doc generation
├── PERFORMANCE_BENCHMARKS.md    # Performance metrics
├── CURRENT_STATE.md             # Project status
├── IMPLEMENTATION_PLAN.md       # Roadmap
├── SPEC.md                      # Technical specifications
│
├── ARCHITECTURE/                # Architecture details
│   └── ITEM_SYSTEM.md
│
├── FEATURES/                    # Feature documentation
│   └── LIT_ROOMS.md
│
├── MVP/                         # MVP specifications
│   ├── 01_scope.md
│   └── ...
│
├── PHASES/                      # Development phases
│   ├── README.md
│   └── ARCHIVE/                 # Completed phases
│
├── WORLD/                       # Game lore
│   ├── README.md
│   └── ...
│
├── REFACTORING/                 # Refactoring docs
│   ├── DOCUMENTATION_IMPROVEMENTS.md
│   └── REFACTORING_SUMMARY.md
│
└── ARCHIVE/                     # Old/redundant docs
```

## Key Documents by Purpose

### Starting Development
1. [Developer Guide](DEVELOPER_GUIDE.md)
2. [Architecture Overview](ARCHITECTURE.md)
3. [Code Style Guide](CODE_STYLE_GUIDE.md)

### Understanding the Game
1. [Current State](CURRENT_STATE.md)
2. [Controls](CONTROLS.md)
3. [World Overview](WORLD/README.md)

### Making Changes
1. [Implementation Plan](IMPLEMENTATION_PLAN.md)
2. [Testing Guide](TESTS.md)
3. [Build System](BUILD_SCRIPT.md)

### Optimization
1. [Performance Benchmarks](PERFORMANCE_BENCHMARKS.md)
2. [Architecture](ARCHITECTURE.md#optimization-points)

## Recent Updates

- **2025-09-14**: Major documentation refactoring
  - Created consolidated Architecture document
  - Unified Controls reference
  - Added Developer Guide
  - Archived completed phases
  - Added API documentation guide
  - Created performance benchmarks
  - Established code style guide

## Contributing to Documentation

When adding or updating documentation:

1. **Location**: Place docs in appropriate subdirectory
2. **Naming**: Use UPPER_SNAKE_CASE for doc files
3. **Format**: Follow Markdown best practices
4. **Cross-reference**: Link related documents
5. **Update Index**: Add to this README if significant

## Documentation Standards

- Use **Markdown** for all documentation
- Include **Table of Contents** for long documents
- Add **Last Updated** date at bottom
- Use **relative links** between documents
- Keep **line length** under 100 characters
- Follow **consistent formatting**

## Tools

### Viewing Documentation
- Any Markdown viewer
- VS Code with Markdown preview
- GitHub web interface

### Generating API Docs
```bash
doxygen Doxyfile
open doc/api/html/index.html
```

### Checking Links
```bash
# Find broken links
grep -r "\[.*\](" DOC/ | grep -v http
```

## Help

If you can't find what you're looking for:
1. Check the [Current State](CURRENT_STATE.md) document
2. Search the codebase
3. Check archived documentation
4. Create an issue on GitHub

---

*Last Updated: 2025-09-14*
*Version: 1.0.0-MVP*