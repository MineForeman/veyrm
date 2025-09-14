# Veyrm Visualization Options

This document outlines various visualization tools and techniques that can be implemented to better understand and analyze the Veyrm codebase.

## 1. Code Metrics Dashboard

### Purpose
Visualize code complexity, size, and quality metrics across the project.

### Components
- Lines of code per file/module
- Cyclomatic complexity per function
- Code duplication percentage
- Technical debt indicators

### Implementation
```bash
# Install tools
brew install cloc
brew install sloccount
pip install lizard  # For complexity analysis

# Generate metrics
cloc --by-file --xml --out=tmp/cloc.xml src/ include/
lizard src/ include/ -o tmp/complexity.csv
```

### Visualization Ideas
- Bar charts for LOC per module
- Heatmap for complexity hotspots
- Treemap for file sizes

## 2. Dependency Graph

### Purpose
Understand include dependencies and identify potential architectural issues.

### Types
- **Include dependency graph**: Which headers include which
- **Link dependency graph**: Library dependencies
- **Circular dependency detection**: Find problematic includes

### Implementation
```bash
# Using include-what-you-use
brew install include-what-you-use
include-what-you-use src/*.cpp > tmp/iwyu.txt

# Using cmake graphviz
cd build
cmake --graphviz=../tmp/dependencies.dot ..
dot -Tsvg ../tmp/dependencies.dot -o ../tmp/dependencies.svg

# Custom header analysis
grep -h "^#include" src/*.cpp include/*.h | sort | uniq -c | sort -rn
```

### Visualization Output
- Directed graph showing include relationships
- Colored by module/subsystem
- Edge thickness based on frequency

## 3. Call Graph

### Purpose
Visualize function call hierarchy and identify hot paths.

### Implementation
```bash
# Compile with profiling
g++ -pg -o veyrm_prof src/*.cpp

# Run and generate gmon.out
./veyrm_prof

# Generate call graph
gprof veyrm_prof gmon.out | gprof2dot | dot -Tsvg -o tmp/callgraph.svg

# Alternative: Using clang
clang++ -S -emit-llvm src/*.cpp
opt -dot-callgraph *.ll
```

### Features
- Function-level granularity
- Call frequency annotations
- Performance bottleneck identification

## 4. Git Statistics

### Purpose
Analyze development patterns and code evolution.

### Metrics
- Code frequency (additions/deletions over time)
- Contributor activity patterns
- File churn (most frequently modified files)
- Commit patterns by hour/day

### Implementation
```bash
# Install gitstats
brew install gitstats

# Generate comprehensive stats
gitstats . tmp/gitstats
open tmp/gitstats/index.html

# Alternative: git-quick-stats
brew install git-quick-stats
git-quick-stats

# Custom analysis
git log --format=format: --name-only | sort | uniq -c | sort -rn | head -20  # Most changed files
git log --pretty=format:'%an' | sort | uniq -c | sort -rn  # Most active contributors
```

### Visualizations
- Timeline graphs
- Pie charts for contributions
- Activity heatmaps

## 5. Memory Layout Visualization

### Purpose
Understand memory usage and object sizes.

### Analysis Types
- Class/struct sizes
- Memory alignment and padding
- Cache line usage

### Implementation
```cpp
// Memory profiler code
#include <iostream>
#include <typeinfo>

template<typename T>
void analyzeType() {
    std::cout << typeid(T).name() << ": "
              << sizeof(T) << " bytes"
              << ", alignment: " << alignof(T) << std::endl;
}

// Usage
analyzeType<Player>();
analyzeType<Monster>();
analyzeType<Map>();
```

```bash
# Using pahole (Linux)
pahole build/bin/veyrm

# Using nm to analyze binary
nm -S build/bin/veyrm | c++filt | sort -k2 -r
```

## 6. Build Dependency Tree

### Purpose
Understand build structure and optimize compilation.

### Features
- Target dependencies
- Build time per target
- Parallel build opportunities

### Implementation
```bash
# CMake dependency graph
cd build
cmake --graphviz=deps.dot ..
dot -Tsvg deps.dot -o ../tmp/build_deps.svg

# Ninja build graph
ninja -t graph | dot -Tsvg > ../tmp/ninja_graph.svg

# Build time analysis
cmake --build . --target clean
time cmake --build . -j1  # Sequential
time cmake --build . -j8  # Parallel
```

## 7. Game-Specific Visualizations

### Map Heatmaps
Track and visualize player/monster movement patterns.

```cpp
// Add to game loop
class HeatmapTracker {
    std::vector<std::vector<int>> heatmap;

    void recordPosition(int x, int y) {
        if (isValid(x, y)) heatmap[y][x]++;
    }

    void saveHeatmap(const std::string& filename) {
        // Output as CSV for visualization
    }
};
```

### Pathfinding Visualization
Debug and optimize A* algorithm.

```cpp
// Add to pathfinder
void visualizePath(const std::vector<Point>& path) {
    // Generate SVG showing:
    // - Start/end points
    // - Explored nodes
    // - Final path
    // - Obstacles
}
```

### FOV Debugging
Visualize field of view calculations.

```cpp
void FOV::exportVisible(const std::string& filename) {
    // Output visible tiles as image
    // Different colors for:
    // - Fully visible
    // - Partially visible
    // - Remembered
    // - Unknown
}
```

### Combat Statistics
Analyze game balance.

```cpp
struct CombatStats {
    std::map<std::string, int> damageByMonster;
    std::map<int, int> damageDistribution;
    std::vector<float> hitRates;

    void generateReport() {
        // Output JSON for visualization
        // - Damage histograms
        // - Hit/miss ratios
        // - Critical hit frequency
    }
};
```

## 8. Code Similarity Map

### Purpose
Identify duplicate code and refactoring opportunities.

### Implementation
```bash
# Using CPD (Copy-Paste Detector)
brew install pmd
pmd cpd --files src --language cpp --minimum-tokens 50 --format xml > tmp/duplicates.xml

# Using cloc for similarity
cloc --diff src/ include/ --by-file

# Using simian (commercial)
java -jar simian.jar src/**/*.cpp include/**/*.h
```

### Visualization
- Matrix showing similar files
- Highlighted duplicate blocks
- Refactoring suggestions

## 9. Architecture Layers

### Purpose
Ensure clean architecture and identify layer violations.

### Layer Definition
```yaml
layers:
  presentation:
    - "*screen.cpp"
    - "*ui*.cpp"
  game_logic:
    - "game.cpp"
    - "*manager.cpp"
  domain:
    - "entity.cpp"
    - "player.cpp"
    - "monster.cpp"
  infrastructure:
    - "log.cpp"
    - "config.cpp"
```

### Implementation
```bash
# Custom script to check layer violations
#!/bin/bash
# Check if lower layers depend on higher layers
grep -r "include.*screen" src/entity.cpp  # Should be empty
```

### Visualization
- Layered diagram with boxes
- Arrows showing dependencies
- Red arrows for violations

## 10. Test Coverage Sunburst

### Purpose
Interactive visualization of test coverage by module.

### Implementation
```bash
# Generate coverage data
cmake -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON ..
make
make test
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --list coverage.info

# Generate HTML report
genhtml coverage.info --output-directory tmp/coverage

# Convert to sunburst (custom tool needed)
python coverage_to_sunburst.py coverage.info > tmp/sunburst.html
```

### Features
- Interactive drill-down
- Color-coded coverage percentages
- Module-level and file-level views

## 11. Performance Profiling Flamegraph

### Purpose
Visualize CPU usage and performance bottlenecks.

### Implementation
```bash
# On macOS
brew install flamegraph

# Profile the application
sudo dtrace -c './build/bin/veyrm' -o tmp/trace.txt -n 'profile-997 { @[ustack(100)] = count(); }'

# Generate flamegraph
stackcollapse.pl tmp/trace.txt | flamegraph.pl > tmp/flamegraph.svg
```

## 12. Entity Relationship Diagram

### Purpose
Visualize game entity relationships and data flow.

### Implementation
```python
# Python script to parse headers and generate ERD
import re
import graphviz

def parse_class(filename):
    with open(filename) as f:
        # Extract class members
        # Identify relationships
        # Generate DOT format
        pass

# Generate ERD
dot = graphviz.Digraph()
for header in headers:
    parse_class(header)
dot.render('tmp/erd', format='svg')
```

## Implementation Priority

### High Priority (Core Understanding)
1. Class Diagram ✅ (Already implemented)
2. Dependency Graph
3. Git Statistics
4. Build Dependencies

### Medium Priority (Development Aid)
5. Code Metrics Dashboard
6. Call Graph
7. Code Similarity Map

### Low Priority (Optimization)
8. Memory Layout
9. Performance Flamegraph
10. Test Coverage Sunburst

### Game-Specific (Debugging)
11. Map Heatmaps
12. FOV Visualization
13. Combat Statistics

## Adding to build.sh

Each visualization can be added as a command:

```bash
./build.sh visualize --type=deps     # Dependency graph
./build.sh visualize --type=stats    # Git statistics
./build.sh visualize --type=metrics  # Code metrics
./build.sh visualize --type=flame    # Performance flamegraph
```

## Resources

- [Graphviz Documentation](https://graphviz.org/documentation/)
- [Flamegraph Repository](https://github.com/brendangregg/FlameGraph)
- [LCOV Documentation](https://github.com/linux-test-project/lcov)
- [Gitstats](http://gitstats.sourceforge.net/)
- [Include What You Use](https://include-what-you-use.org/)
- [CPD Documentation](https://pmd.github.io/latest/pmd_userdocs_cpd.html)

## Notes

- Most visualizations output to `tmp/` directory (gitignored)
- SVG format preferred for interactive diagrams
- PNG format for documentation/reports
- Consider automation via GitHub Actions for continuous metrics