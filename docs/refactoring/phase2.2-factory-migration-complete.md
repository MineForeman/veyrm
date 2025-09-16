# Phase 2.2: Entity Factory & Migration - Implementation Complete

## Summary

Successfully implemented factories and migration adapters to bridge the legacy Entity system with the new ECS architecture. This provides a clean migration path from inheritance-based entities to component-based entities.

## Components Implemented

### 1. EntityBuilder (`include/ecs/entity_factory.h`)

**Features:**
- Fluent interface for building entities
- Chain methods for adding components
- Type-safe component configuration
- Reset capability for reuse

**Example Usage:**
```cpp
auto entity = EntityBuilder()
    .withPosition(10, 10)
    .withRenderable("@", ftxui::Color::Yellow)
    .withHealth(100)
    .withCombat(6, 3, 2)
    .withCombatName("Hero")
    .build();
```

### 2. Factory Classes

#### PlayerFactory
- Creates player entities with standard components
- Configurable name support
- Consistent player stats

#### MonsterFactoryECS
- Registry-based monster creation
- Pre-configured common monsters (goblin, orc, troll, skeleton, dragon)
- Extensible through `registerMonster()`
- Fallback for unknown types

#### ItemFactoryECS
- Registry-based item creation
- Pre-configured common items (potion, sword, gold, scroll)
- Extensible through `registerItem()`
- Simple component structure (position + rendering)

### 3. EntityAdapter (`include/ecs/entity_adapter.h`)

**Bidirectional Conversion:**
- Legacy → ECS: `fromLegacyEntity()`, `fromPlayer()`, `fromMonster()`, `fromItem()`
- ECS → Legacy: `updatePosition()`, `updateHealth()`, `updateRendering()`, `syncToLegacy()`

**Type Detection:**
- `isPlayer()` - Identifies player entities
- `isMonster()` - Identifies monster entities
- `isItem()` - Identifies item entities

**Migration Utilities:**
- `EntityMigrationHelper::migrateAll()` - Batch conversion
- `EntityMigrationHelper::createMigrationMap()` - Mapping for references

## Architecture Benefits

### 1. **Gradual Migration**
- Run both systems side-by-side
- Convert entities as needed
- No big-bang refactoring required

### 2. **Type Safety**
- Compile-time component checking
- No manual casting required
- Clear ownership semantics

### 3. **Flexibility**
- Add new entity types easily
- Mix components freely
- Runtime modification possible

### 4. **Testing**
- Each factory testable in isolation
- Adapter testing ensures compatibility
- No legacy system breakage

## Test Coverage

**New Tests:** 85 assertions in factory/adapter tests
**Total Tests:** 1992 assertions in 151 test cases

### Test Categories:
- EntityBuilder functionality
- PlayerFactory creation
- MonsterFactoryECS with multiple types
- ItemFactoryECS variations
- Legacy to ECS conversion
- ECS to legacy synchronization
- Type detection accuracy

## Migration Strategy

### Current State
```
Legacy System          Bridge              ECS System
┌─────────┐      ┌──────────────┐      ┌────────────┐
│ Entity  │◄────►│EntityAdapter │◄────►│ecs::Entity │
│ Player  │      │              │      │+Components │
│ Monster │      │  Conversion  │      │            │
│ Item    │      │   Methods    │      │ Factories  │
└─────────┘      └──────────────┘      └────────────┘
```

### Migration Steps

1. **Phase 2.3: System Implementation**
   - Create systems that operate on components
   - MovementSystem, RenderSystem, CombatSystem
   - Keep legacy systems running in parallel

2. **Phase 2.4: Incremental Replacement**
   - Replace one subsystem at a time
   - Use adapter for compatibility
   - Validate behavior matches

3. **Phase 2.5: Complete Migration**
   - Remove legacy Entity hierarchy
   - Remove adapter layer
   - Pure ECS architecture

## Code Examples

### Creating Entities with Factory

```cpp
// Create player
PlayerFactory playerFactory;
auto player = playerFactory.create("Hero", 10, 10);

// Create monster
MonsterFactoryECS monsterFactory;
auto goblin = monsterFactory.create("goblin", 5, 5);

// Create item
ItemFactoryECS itemFactory;
auto potion = itemFactory.create("potion", 3, 3);
```

### Converting Legacy Entities

```cpp
// Convert existing player
Player legacyPlayer(10, 10);
auto ecsPlayer = EntityAdapter::fromPlayer(legacyPlayer);

// Convert and sync back
auto ecsEntity = EntityBuilder()
    .withPosition(15, 15)
    .withHealth(80)
    .build();
EntityAdapter::syncToLegacy(*ecsEntity, legacyPlayer);
```

### Registering Custom Types

```cpp
MonsterFactoryECS factory;
factory.registerMonster("zombie", [](int x, int y) {
    return EntityBuilder()
        .withPosition(x, y)
        .withRenderable("z", ftxui::Color::Green)
        .withHealth(25)
        .withCombat(3, 1, 1)
        .withCombatName("Zombie")
        .build();
});
```

## Performance Considerations

### Current Impact
- Small overhead from adapter layer
- Dynamic allocation for components
- Virtual function calls for polymorphism

### Optimization Opportunities
- Component pools for allocation
- Batch conversion operations
- Cache adapter mappings
- Remove adapter after migration

## Metrics

### Code Quality
- **Flexibility:** High - mix and match components
- **Maintainability:** Improved - clear separation
- **Type Safety:** Strong - compile-time checking
- **Test Coverage:** Comprehensive - all paths tested

### Migration Risk
- **Low Risk:** Adapter ensures compatibility
- **Reversible:** Can revert to legacy if needed
- **Incremental:** No big-bang changes
- **Validated:** Tests ensure correctness

## Next Steps

### Immediate (Phase 2.3)
1. Implement MovementSystem for PositionComponent
2. Implement RenderSystem for RenderableComponent
3. Implement CombatSystem for combat resolution
4. Create SystemManager for update ordering

### Medium Term
1. Migrate EntityManager to use ECS
2. Update save/load for ECS entities
3. Profile performance differences
4. Remove redundant legacy code

### Long Term
1. Complete migration to pure ECS
2. Remove adapter layer
3. Optimize component storage
4. Add advanced ECS features (events, queries)

## Conclusion

Phase 2.2 successfully provides the bridge between legacy and modern architectures:
- **Factories** enable consistent entity creation
- **Adapters** ensure compatibility during migration
- **Tests** validate correctness at every step
- **Documentation** guides the migration process

The foundation is now complete for incremental migration from inheritance to composition, allowing the codebase to evolve safely without disrupting existing functionality.