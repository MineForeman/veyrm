# Performance Documentation

Performance analysis, optimization guides, and benchmarks for Veyrm.

## Contents

- **[benchmarks.md](benchmarks.md)** - Current performance benchmarks
- **[rendering-improvements.md](rendering-improvements.md)** - Rendering optimization plans

## Performance Goals

- Frame time < 16ms (60 FPS)
- Memory usage < 100MB
- Startup time < 2 seconds
- Save/load time < 1 second

## Optimization Areas

### Rendering

- Map viewport rendering
- Entity rendering
- UI updates

### Memory

- Entity management
- Map storage
- Message history

### Algorithms

- FOV calculation
- Pathfinding
- Map generation

## Tools

- Profiling with `perf`
- Memory analysis with `valgrind`
- Frame timing measurements
