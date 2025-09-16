# Class Reference

API documentation for Veyrm classes.

## Core Classes

### Game Management

- **GameManager** - Central game controller
- **GameState** - Game state container
- **TurnManager** - Turn-based system controller

### Map & World

- **Map** - Tile-based map container
- **Tile** - Individual map tile
- **Room** - Rectangular room
- **MapGenerator** - Procedural map generation
- **MapValidator** - Map connectivity validation

### Entities

- **Entity** - Base entity class
- **Player** - Player character
- **Monster** - Enemy creatures
- **Item** - Collectable objects
- **EntityManager** - Entity lifecycle management

### Systems

- **FOV** - Field of view calculation
- **Pathfinding** - Movement path calculation
- **CombatSystem** - Combat resolution
- **InventorySystem** - Item management

### UI/Rendering

- **Renderer** - Terminal rendering system
- **MessageLog** - Game message display
- **StatusBar** - Player status display

## Documentation Format

Each class should document:

- Purpose and responsibilities
- Public interface
- Key methods
- Usage examples
- Related classes
