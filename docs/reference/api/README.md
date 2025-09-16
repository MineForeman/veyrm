# API Reference

Complete API documentation for the Veyrm codebase.

## Quick Links

- [Class Reference](classes/README.md) - All classes
- [System APIs](systems/README.md) - System interfaces
- [Generated Docs](generated/index.html) - Doxygen output

## Core APIs

### Game Management

- `GameManager` - Central game coordinator
- `GameState` - Game state management
- `GameScreen` - Main game UI

### Entity System

- `Entity` - Base entity class
- `Player` - Player character
- `Monster` - Monster entities
- `Item` - Item entities
- `EntityManager` - Entity lifecycle

### Map System

- `Map` - World representation
- `Tile` - Individual map cells
- `MapGenerator` - Procedural generation
- `Room` - Room structures

### Combat System

- `CombatSystem` - Combat resolution
- `Weapon` - Weapon items
- `Armor` - Armor items
- `DamageCalculator` - Damage math

### UI System

- `Renderer` - FTXUI renderer
- `InputHandler` - Input processing
- `MessageLog` - Game messages
- `StatusBar` - Status display

## Usage Examples

### Creating an Entity

```cpp
auto player = std::make_shared<Player>(x, y);
player->setHealth(100);
player->setName("Hero");
entity_manager->addEntity(player);
```

### Generating a Map

```cpp
MapGenerator generator;
auto map = generator.generate(MapType::PROCEDURAL, seed);
map->setTile(x, y, Tile::FLOOR);
```

### Processing Combat

```cpp
CombatSystem combat;
int damage = combat.calculateDamage(attacker, defender);
defender->takeDamage(damage);
```

## API Conventions

### Naming

- Classes: `PascalCase`
- Methods: `camelCase()`
- Members: `snake_case_`
- Constants: `UPPER_CASE`

### Memory Management

- Use `std::shared_ptr` for shared ownership
- Use `std::unique_ptr` for single ownership
- Never use raw `new`/`delete`

### Error Handling

- Throw exceptions for errors
- Return `std::optional` for nullable
- Use error codes for expected failures

## Generating Documentation

### Setup Doxygen

```bash
# Install doxygen
brew install doxygen graphviz

# Generate config
doxygen -g Doxyfile
```

### Generate Docs

```bash
# Generate HTML docs
doxygen Doxyfile

# View output
open docs/reference/api/generated/index.html
```

## API Categories

### [Classes](classes/README.md)

- Core Classes
- Entity Classes
- System Classes
- UI Classes
- Utility Classes

### [Systems](systems/README.md)

- Game Systems
- Entity Systems
- Map Systems
- Combat Systems
- UI Systems

### [Utilities](utilities/README.md)

- Math Utilities
- String Utilities
- File Utilities
- Random Utilities

## API Stability

### Stable APIs

These APIs are stable and unlikely to change:

- Entity base classes
- Map tile system
- Combat calculations
- Save/load format

### Unstable APIs

These APIs may change in future versions:

- AI system (being enhanced)
- Networking (not yet implemented)
- Graphics rendering (terminal only now)

## Deprecation Policy

When APIs are deprecated:

1. Mark with `[[deprecated]]` attribute
2. Document replacement API
3. Maintain for one major version
4. Remove in next major version

## Contributing

To add API documentation:

1. Use Doxygen comments
2. Include examples
3. Document parameters
4. Note thread safety
5. Update this index

---

*See also: [Code Style Guide](../../development/standards/code-style.md)*
