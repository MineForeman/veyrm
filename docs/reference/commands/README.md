# Command Reference

Command-line tools and scripts for Veyrm.

## Contents

- **[build-script.md](build-script.md)** - Complete build.sh reference

## Quick Command Reference

### Build Script (build.sh)

```bash
# Building
./build.sh build      # Debug build
./build.sh release    # Release build
./build.sh clean      # Clean build directory

# Running
./build.sh run        # Run with map selection
./build.sh play       # Alias for run

# Testing
./build.sh test       # Run all tests
./build.sh dump       # Run with frame dump
./build.sh keys 'seq' # Run with key sequence

# Development
./build.sh debug      # Run with debugger
./build.sh profile    # Run with profiler
```

### Game Executable

```bash
# Map options
--map procedural     # Procedural dungeon (default)
--map room          # Single room
--map arena         # Arena map
--map cross         # Cross-shaped map

# Other options
--data-dir path     # Custom data directory
--dump              # Frame dump mode
--help              # Show help
```