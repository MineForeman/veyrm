# Reference Documentation

Technical reference for APIs, commands, and configuration.

## Contents

### [API Reference](api/)
Code documentation and API references.

### [Commands](commands/)
Command-line tools and build scripts.

### [Configuration](configuration/)
Game configuration and data formats.

## Quick Reference

### Build Commands
```bash
./build.sh         # Interactive menu
./build.sh build   # Build the game
./build.sh run     # Run the game
./build.sh test    # Run tests
```

### Game Commands
```bash
./build/bin/veyrm --map procedural  # Procedural map
./build/bin/veyrm --map room        # Single room
./build/bin/veyrm --data-dir path   # Custom data directory
```

### Data Files
- `data/monsters.json` - Monster definitions
- `data/items.json` - Item definitions
- `saves/*.json` - Save game files

## Documentation Standards

- API docs use Doxygen-style comments
- Commands documented with examples
- Configuration includes JSON schemas
- All options have descriptions