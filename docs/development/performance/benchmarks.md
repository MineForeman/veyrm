# Performance Benchmarks

## Overview

This document tracks performance metrics and optimization opportunities for Veyrm.

## Current Performance Metrics

### Build Performance
| Metric | Value | Target |
|--------|-------|--------|
| Clean Build Time | ~30 seconds | <30s |
| Incremental Build | ~5 seconds | <5s |
| Test Suite Run | ~2 seconds | <5s |
| Binary Size (Release) | 2.5 MB | <5 MB |
| Binary Size (Debug) | 8.1 MB | <15 MB |

### Runtime Performance
| Metric | Value | Target |
|--------|-------|--------|
| Startup Time | <200ms | <500ms |
| Frame Rate | 60 FPS | 60 FPS |
| Memory Usage (Idle) | ~50 MB | <100 MB |
| Memory Usage (Playing) | ~55 MB | <150 MB |
| Save Time | <100ms | <200ms |
| Load Time | <200ms | <500ms |
| Map Generation | ~50ms | <100ms |

### Game Performance
| Metric | Value | Target |
|--------|-------|--------|
| Entities per Frame | 100+ | 200+ |
| FOV Calculation | <5ms | <10ms |
| Pathfinding (per entity) | <1ms | <2ms |
| Turn Processing | <10ms | <20ms |
| Render Time | <16ms | <16ms |

## Benchmarking Tools

### Build Profiling
```bash
# Time the build
time cmake --build build -j

# Analyze build times with ninja
cmake -G Ninja -B build
ninja -C build -t graph > build_graph.dot
dot -Tpng build_graph.dot -o build_times.png
```

### Runtime Profiling

#### Using Instruments (macOS)
```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# Profile with Instruments
instruments -t "Time Profiler" ./build/bin/veyrm
```

#### Using Perf (Linux)
```bash
# Record performance data
perf record -g ./build/bin/veyrm
perf report

# Generate flame graph
perf script | flamegraph.pl > flamegraph.svg
```

#### Using Valgrind
```bash
# Memory profiling
valgrind --tool=massif ./build/bin/veyrm
ms_print massif.out.*

# Cache profiling
valgrind --tool=cachegrind ./build/bin/veyrm
cg_annotate cachegrind.out.*
```

### Custom Benchmarks

#### Frame Time Measurement
```cpp
class FrameTimer {
    std::chrono::high_resolution_clock::time_point start;
    std::vector<double> frame_times;

public:
    void beginFrame() {
        start = std::chrono::high_resolution_clock::now();
    }

    void endFrame() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>
                       (end - start).count() / 1000.0;
        frame_times.push_back(duration);
    }

    double getAverageFrameTime() const {
        return std::accumulate(frame_times.begin(), frame_times.end(), 0.0)
               / frame_times.size();
    }
};
```

## Optimization Opportunities

### High Impact

#### 1. FOV Calculation Caching
**Current**: Recalculates entire FOV each frame
**Optimization**: Cache and update only on movement
**Expected Improvement**: 80% reduction in FOV overhead
```cpp
// Before
void updateFOV() {
    calculateFOV(player_x, player_y, radius);
}

// After
void updateFOV() {
    if (player_moved) {
        calculateFOV(player_x, player_y, radius);
        player_moved = false;
    }
}
```

#### 2. Entity Spatial Indexing
**Current**: Linear search for entities at position
**Optimization**: Spatial hash map
**Expected Improvement**: O(n) to O(1) lookup
```cpp
// Spatial hash map
std::unordered_map<Point, std::vector<Entity*>, PointHash> spatial_index;
```

#### 3. String Interning
**Current**: String allocations for entity types
**Optimization**: String interning with IDs
**Expected Improvement**: 50% reduction in string operations

### Medium Impact

#### 4. Render Culling
**Current**: Attempts to render entire map
**Optimization**: Only render visible viewport
**Expected Improvement**: 70% reduction in render calls

#### 5. Message Log Optimization
**Current**: Stores full message history
**Optimization**: Ring buffer with fixed size
**Expected Improvement**: Constant memory usage

#### 6. Pathfinding Cache
**Current**: Recalculates paths each turn
**Optimization**: Cache paths, invalidate on map change
**Expected Improvement**: 60% reduction in pathfinding

### Low Impact

#### 7. Color Caching
**Current**: Creates color objects each frame
**Optimization**: Pre-create color palette
**Expected Improvement**: Minor allocation reduction

#### 8. Input Buffering
**Current**: Processes one input per frame
**Optimization**: Buffer and batch inputs
**Expected Improvement**: Smoother input handling

## Memory Optimization

### Current Memory Usage
```
Heap Usage:
- Map: ~1.5 MB (198x66 tiles)
- Entities: ~500 KB (average game)
- Strings: ~2 MB (messages, names)
- FTXUI: ~10 MB (UI components)
- JSON: ~1 MB (data files)
```

### Optimization Strategies

1. **Object Pooling**: Reuse entity objects
2. **Small String Optimization**: Use SSO for names
3. **Bit Packing**: Pack tile flags into bitfields
4. **Lazy Loading**: Load data files on demand
5. **Memory Mapping**: mmap save files

## Benchmark Suite

### Automated Benchmarks
```bash
#!/bin/bash
# benchmark.sh

echo "Running Veyrm Benchmarks..."

# Build benchmark
echo "Build Time:"
time cmake --build build -j

# Startup benchmark
echo "Startup Time:"
time ./build/bin/veyrm --benchmark-startup

# Map generation benchmark
echo "Map Generation (100 iterations):"
./build/bin/veyrm --benchmark-mapgen 100

# FOV benchmark
echo "FOV Calculation (1000 iterations):"
./build/bin/veyrm --benchmark-fov 1000

# Save/Load benchmark
echo "Save/Load Cycle:"
./build/bin/veyrm --benchmark-save-load
```

### Benchmark Results Tracking

#### Template
```markdown
## Benchmark Run: [Date]
Platform: [OS, CPU, RAM]
Compiler: [Version, Flags]
Build Type: [Debug/Release]

| Test | Time | Memory | Notes |
|------|------|--------|-------|
| Startup | X ms | X MB | |
| Map Gen | X ms | X MB | |
| FOV | X ms | X MB | |
| Turn | X ms | X MB | |
```

## Profiling Checklist

### Before Optimization
- [ ] Establish baseline metrics
- [ ] Profile to identify bottlenecks
- [ ] Document current implementation
- [ ] Write benchmark tests
- [ ] Set improvement targets

### During Optimization
- [ ] Make one change at a time
- [ ] Measure after each change
- [ ] Document what worked/didn't
- [ ] Check for regressions
- [ ] Update tests if needed

### After Optimization
- [ ] Compare final metrics to baseline
- [ ] Document improvements achieved
- [ ] Update performance benchmarks
- [ ] Add regression tests
- [ ] Update documentation

## Performance Guidelines

### Do's
- Profile before optimizing
- Focus on algorithmic improvements
- Use appropriate data structures
- Cache expensive calculations
- Minimize allocations in hot paths

### Don'ts
- Don't optimize prematurely
- Don't sacrifice readability without measurement
- Don't assume - always profile
- Don't micro-optimize without macro impact
- Don't break abstractions for minor gains

## Historical Performance

### Version 1.0.0-MVP
- Map generation: 50ms
- Save file size: 7-8 KB (98.6% reduction from original)
- Turn processing: <10ms
- Memory usage: ~50 MB

### Pre-Optimization (v0.9)
- Map generation: 200ms
- Save file size: 540+ KB
- Turn processing: ~30ms
- Memory usage: ~80 MB

## Future Optimizations

### Planned
1. Multithreaded FOV calculation
2. GPU-accelerated rendering (optional)
3. Compressed save files
4. Incremental map generation
5. Entity component system (ECS)

### Experimental
1. SIMD for FOV calculations
2. Custom memory allocator
3. Compile-time string hashing
4. Profile-guided optimization
5. Link-time optimization

## Monitoring

### Runtime Metrics
```cpp
class PerformanceMonitor {
    struct Metrics {
        double frame_time;
        size_t entity_count;
        size_t memory_usage;
        double fps;
    };

    void update() {
        metrics.frame_time = getFrameTime();
        metrics.entity_count = getEntityCount();
        metrics.memory_usage = getMemoryUsage();
        metrics.fps = 1000.0 / metrics.frame_time;
    }

    void report() {
        LOG(INFO) << "FPS: " << metrics.fps
                  << " Entities: " << metrics.entity_count
                  << " Memory: " << metrics.memory_usage;
    }
};
```

---

*Last Updated: 2025-09-14*
*Version: 1.0.0-MVP*