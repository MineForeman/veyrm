# C++ Class Optimization Implementation Plan

## Executive Summary

This document outlines a systematic approach to refactoring the Veyrm codebase to align with modern C++ best practices. The refactoring focuses on improving type safety, memory management, performance, and maintainability while preserving existing functionality.

## Refactoring Phases

### Phase 1: Critical Memory Safety (Week 1)
**Goal**: Eliminate raw pointers and unsafe memory management

#### 1.1 Entity System Memory Safety
- **Files**: `include/entity.h`, `src/entity.cpp`
- **Tasks**:
  - Replace `void* user_data` with type-safe `std::variant<MonsterAI*, nullptr_t>`
  - Create proper RAII wrapper for AI data management
  - Add static assertions for type safety
- **Testing**: Run existing entity tests, add memory leak detection

#### 1.2 GameManager Pointer Safety
- **Files**: `include/game_state.h`, `src/game_state.cpp`
- **Tasks**:
  - Replace `Room* current_room` with `std::shared_ptr<Room>`
  - Update all room references to use smart pointers
  - Ensure proper lifetime management
- **Testing**: Verify room transitions, save/load functionality

#### 1.3 MonsterAI Memory Pool Refactoring
- **Files**: `include/monster_ai.h`, `src/monster_ai.cpp`
- **Tasks**:
  - Simplify AI data pool management
  - Replace manual pool with direct `std::unique_ptr` ownership
  - Remove `assigned_room` raw pointer
- **Testing**: AI behavior tests, performance benchmarks

### Phase 2: Architecture Modernization (Week 2)
**Goal**: Move from inheritance to composition for better flexibility

#### 2.1 Entity Component System (ECS) Design
- **Files**: New files in `include/ecs/`
- **Tasks**:
  - Design component interfaces (Position, Renderable, Combat, AI, Inventory)
  - Create EntityComponent base class
  - Implement component registry
- **Testing**: Unit tests for each component

#### 2.2 Entity Refactoring to Components
- **Files**: `include/entity.h`, `include/player.h`, `include/monster.h`, `include/item.h`
- **Tasks**:
  - Extract position to PositionComponent
  - Extract rendering to RenderableComponent
  - Extract combat stats to CombatComponent
  - Remove type flags (is_player, is_monster, is_item)
- **Testing**: Integration tests, gameplay verification

#### 2.3 System Updates for Components
- **Files**: `include/entity_manager.h`, `include/combat_system.h`, `include/renderer.h`
- **Tasks**:
  - Update EntityManager to work with components
  - Modify combat system to use CombatComponent
  - Update renderer to use RenderableComponent
- **Testing**: Full gameplay testing, save/load compatibility

### Phase 3: Performance Optimizations (Week 3)
**Goal**: Implement move semantics and optimize hot paths

#### 3.1 Move Semantics Implementation
- **Files**: `include/item.h`, `include/combat_system.h`, `include/inventory.h`
- **Tasks**:
  - Add move constructors to Item class
  - Add move assignment operators
  - Implement perfect forwarding in factories
  - Add move semantics to CombatResult
- **Testing**: Performance benchmarks, memory profiling

#### 3.2 String Optimization
- **Files**: All classes with string members
- **Tasks**:
  - Use string_view for non-owning string parameters
  - Implement small string optimization where applicable
  - Reduce string copies in hot paths
- **Testing**: Performance benchmarks

#### 3.3 Container Optimizations
- **Files**: `include/map.h`, `include/entity_manager.h`
- **Tasks**:
  - Profile and optimize container usage
  - Consider flat_map for small collections
  - Reserve capacity for known-size vectors
- **Testing**: Memory usage analysis, performance tests

### Phase 4: API Improvements (Week 4)
**Goal**: Improve const-correctness and add modern C++ attributes

#### 4.1 Const-Correctness Pass
- **Files**: All header files
- **Tasks**:
  - Mark all non-modifying methods as const
  - Add const overloads where needed
  - Ensure consistent const usage
- **Testing**: Compilation tests, API usage verification

#### 4.2 Modern Attributes
- **Files**: All header files
- **Tasks**:
  - Add [[nodiscard]] to query methods
  - Add noexcept specifications
  - Add [[maybe_unused]] where appropriate
  - Use explicit constructors
- **Testing**: Compiler warning analysis

#### 4.3 Smart Pointer Audit
- **Files**: All classes using pointers
- **Tasks**:
  - Convert remaining raw pointers to smart pointers
  - Use weak_ptr to break circular dependencies
  - Document ownership semantics
- **Testing**: Memory leak detection, stress testing

### Phase 5: Testing and Documentation (Week 5)
**Goal**: Ensure comprehensive testing and documentation

#### 5.1 Unit Test Coverage
- **Files**: `tests/` directory
- **Tasks**:
  - Add tests for refactored components
  - Ensure 80%+ code coverage
  - Add memory leak tests
  - Add performance regression tests
- **Testing**: Run full test suite

#### 5.2 Integration Testing
- **Files**: New integration test files
- **Tasks**:
  - Test save/load compatibility
  - Test gameplay scenarios
  - Verify AI behavior
  - Test edge cases
- **Testing**: Automated gameplay tests

#### 5.3 Documentation Update
- **Files**: All refactored headers, README.md
- **Tasks**:
  - Update Doxygen comments
  - Document new architecture
  - Create migration guide
  - Update CLAUDE.md with new patterns
- **Testing**: Documentation generation

## Implementation Strategy

### Incremental Refactoring Approach
1. **Branch Strategy**: Create feature branches for each phase
2. **Testing**: Run full test suite after each change
3. **Review**: Code review after each component
4. **Rollback Plan**: Keep ability to revert each phase independently

### Risk Mitigation
1. **Save Compatibility**: Maintain backward compatibility or provide migration
2. **Performance**: Benchmark before and after each optimization
3. **Functionality**: Extensive playtesting after each phase
4. **Memory Safety**: Use sanitizers (ASAN, MSAN) during development

### Success Metrics
- Zero memory leaks (validated by sanitizers)
- 10-20% performance improvement in hot paths
- 80%+ test coverage
- Zero compiler warnings with -Wall -Wextra
- Reduced cyclomatic complexity
- Improved compilation times

## Detailed Task Breakdown

### Phase 1.1: Entity Memory Safety (Estimated: 2 days)

```cpp
// Before: Unsafe void pointer
class Entity {
    void* user_data = nullptr;  // UNSAFE!
};

// After: Type-safe variant
class Entity {
    using AIData = std::variant<std::monostate, MonsterAIData>;
    AIData ai_data;
};
```

**Steps**:
1. Create AIData variant type
2. Update all user_data accesses
3. Add visitor pattern for AI processing
4. Remove manual memory management
5. Test with valgrind/ASAN

### Phase 2.1: Component System Design (Estimated: 3 days)

```cpp
// New component-based design
class IComponent {
public:
    virtual ~IComponent() = default;
    virtual ComponentType getType() const = 0;
};

class PositionComponent : public IComponent {
    Point position;
    ComponentType getType() const override { return ComponentType::Position; }
};

class Entity {
    std::unordered_map<ComponentType, std::unique_ptr<IComponent>> components;
public:
    template<typename T>
    T* getComponent() const;
};
```

**Steps**:
1. Design component interface
2. Implement core components
3. Create component factory
4. Add component queries
5. Test component system independently

### Phase 3.1: Move Semantics (Estimated: 1 day)

```cpp
// Add move semantics to Item
class Item {
public:
    Item(Item&&) noexcept = default;
    Item& operator=(Item&&) noexcept = default;

    // Perfect forwarding in factory
    template<typename... Args>
    static std::unique_ptr<Item> create(Args&&... args) {
        return std::make_unique<Item>(std::forward<Args>(args)...);
    }
};
```

**Steps**:
1. Identify move-heavy operations
2. Add move constructors/assignment
3. Use std::move in appropriate places
4. Benchmark performance improvements
5. Verify no regressions

## Testing Plan

### Unit Tests Required
- Component system tests
- Memory safety tests
- Move semantics tests
- Const-correctness tests
- Factory pattern tests

### Integration Tests Required
- Full game loop with new architecture
- Save/load with refactored entities
- Combat with component system
- AI behavior with new memory model
- Performance regression tests

### Performance Benchmarks
- Entity creation/destruction speed
- Map generation time
- FOV calculation performance
- Save/load speed
- Memory usage reduction

## Rollout Schedule

| Week | Phase | Key Deliverables | Risk Level |
|------|-------|------------------|------------|
| 1 | Memory Safety | No raw pointers, RAII everywhere | High |
| 2 | Architecture | Component system, reduced inheritance | High |
| 3 | Performance | Move semantics, optimized strings | Medium |
| 4 | API Polish | Const-correct, modern attributes | Low |
| 5 | Testing | 80% coverage, full documentation | Low |

## Dependencies and Prerequisites

### Required Tools
- C++23 compiler (GCC 13+, Clang 16+, MSVC 2022+)
- CMake 3.25+
- Valgrind or AddressSanitizer
- Catch2 for testing
- Doxygen for documentation

### Knowledge Requirements
- Modern C++ patterns (RAII, move semantics)
- Component-based architecture
- Performance profiling
- Memory debugging tools

## Success Criteria

### Must Have
- ✅ Zero memory leaks
- ✅ All tests passing
- ✅ No regression in functionality
- ✅ Improved type safety

### Should Have
- ✅ 10%+ performance improvement
- ✅ Reduced compilation time
- ✅ Better error messages
- ✅ Cleaner API

### Nice to Have
- ✅ 20%+ performance improvement
- ✅ Modular plugin system
- ✅ Hot-reload for development
- ✅ Compile-time validation

## Conclusion

This implementation plan provides a structured approach to modernizing the Veyrm codebase. By following these phases, we can incrementally improve the code quality while maintaining stability and performance. Each phase builds on the previous one, allowing for continuous integration and testing throughout the refactoring process.

The focus on memory safety, modern C++ patterns, and performance optimization will result in a more maintainable, efficient, and robust codebase that follows industry best practices.