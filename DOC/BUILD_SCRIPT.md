# Build Script Documentation

## Overview

The `build.sh` script is a comprehensive build helper for the Veyrm project, providing an easy-to-use interface for building, running, and testing the game. It simplifies CMake operations and provides additional utilities for development.

## Features

- **Interactive menu** - User-friendly interface for common tasks
- **Build configurations** - Quick switching between Debug and Release builds
- **Map selection** - Easy testing of different map types
- **Terminal management** - Automatic terminal reset after crashes
- **Test automation** - Run unit tests and dump mode tests
- **Color output** - Clear visual feedback for build status

## Usage

### Interactive Mode (Default)

```bash
./build.sh
```

Shows an interactive menu with options:
1. Build (Debug)
2. Build (Release)
3. Clean Build (Debug)
4. Clean Build (Release)
5. Run Game
6. Run Tests
7. Run System Check
8. Run Dump Mode Test
9. Clean
10. Reset Terminal
0. Exit

### Command Line Mode

```bash
./build.sh [command] [options]
```

#### Commands

| Command | Description | Example |
|---------|-------------|---------|
| `build [debug\|release]` | Build the project | `./build.sh build release` |
| `clean` | Clean build directory | `./build.sh clean` |
| `run [map_type]` | Run the game | `./build.sh run arena` |
| `test` | Run unit tests | `./build.sh test` |
| `dump [keystrokes]` | Run dump mode test | `./build.sh dump '\n\u\r'` |
| `keys [keystrokes]` | Run with automated key input | `./build.sh keys '\n3\njjjq'` |
| `check` | Run system checks | `./build.sh check` |
| `reset` | Reset terminal | `./build.sh reset` |
| `menu` | Show interactive menu | `./build.sh menu` |
| `help` | Show help message | `./build.sh help` |

### Map Selection

When running the game, you can specify a map type:

```bash
# Interactive selection
./build.sh run

# Direct selection
./build.sh run room      # Single 20x20 room
./build.sh run dungeon   # 5-room dungeon (default)
./build.sh run corridor  # Corridor test map
./build.sh run arena     # Combat arena
./build.sh run stress    # Stress test map
```

If no map type is specified, an interactive menu appears:

```
Select map type:
1) Test Dungeon (default, 5 rooms)
2) Test Room (single 20x20 room)
3) Corridor Test (long corridors)
4) Combat Arena (open space)
5) Stress Test (large map)
```

### Examples

```bash
# Quick build and run
./build.sh build && ./build.sh run

# Clean build in release mode
./build.sh clean build release

# Run with specific map
./build.sh run corridor

# Test with automated input (dump mode)
./build.sh dump '\n\u\u\r\d'

# Test AI behavior with automated keys
./build.sh keys '\n3\njjjq'

# Reset terminal after crash
./build.sh reset
```

## Automated Testing

The build script supports two modes of automated testing:

### Dump Mode Testing
The dump mode provides frame-by-frame output for debugging:

```bash
# Default test sequence
./build.sh dump

# Custom keystroke sequence
./build.sh dump '\n\u\u\r\r\d\l'
```

### Automated Key Input Testing
For testing AI behavior and gameplay features:

```bash
# Test AI with Arena map (option 3) and movement
./build.sh keys '\n3\njjjq'

# Test different map selection and actions
./build.sh keys '\n2\nhlllq'
```

### Keystroke Format

- `\n` - Enter/Return
- `\u` - Up arrow
- `\d` - Down arrow
- `\l` - Left arrow
- `\r` - Right arrow
- `\e` - Escape
- `\t` - Tab
- `\b` - Backspace
- `\\` - Literal backslash
- Regular characters are sent as-is

## Build Directories

The script manages the following structure:

```
verym/
├── build/           # CMake build directory
│   ├── bin/         # Compiled executables
│   │   ├── veyrm    # Main game executable
│   │   └── veyrm_tests # Test executable
│   └── ...          # CMake generated files
└── build.sh         # This script
```

## Environment Variables

- `VEYRM_DEBUG=1` - Enable debug mode in the game

## Terminal Reset

The script includes terminal reset functionality that:
- Disables all mouse tracking modes
- Shows cursor
- Resets terminal to default state
- Restores sane terminal settings

This is automatically called after running the game and can be manually triggered with `./build.sh reset`.

## Error Handling

The script includes:
- Exit on error (`set -e`)
- Color-coded error messages
- Automatic terminal cleanup
- Build status indicators

## Platform Support

The script is designed for Unix-like systems:
- **macOS** - Full support
- **Linux** - Full support
- **Windows** - Use with WSL or Git Bash

## Tips

1. **Quick iteration**: Use `./build.sh` menu for rapid development
2. **Testing maps**: Use direct map selection to test specific layouts
3. **After crashes**: Run `./build.sh reset` to fix terminal
4. **CI/CD**: Use command-line mode for automation
5. **Debug builds**: Default for development with debug symbols
6. **Release builds**: Use for performance testing

## Troubleshooting

### Terminal Issues

If the terminal becomes unresponsive or shows artifacts:
```bash
./build.sh reset
```

### Build Failures

For clean rebuild:
```bash
./build.sh clean build
```

### Missing Dependencies

Check system requirements:
```bash
./build.sh check
```

## Integration with Veyrm

The script is integrated with Veyrm's features:
- Map type selection passes `--map` argument to the game
- Dump mode uses `--dump` argument for automated testing
- System check runs `--test` argument

## Future Enhancements

Planned improvements:
- Profile-guided optimization builds
- Parallel test execution
- Installation targets
- Package generation
- Cross-compilation support