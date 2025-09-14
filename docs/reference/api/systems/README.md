# System API Reference

API documentation for Veyrm game systems.

## System Overview

Veyrm's systems handle specific aspects of the game logic.

## Core Systems

### Input System
Handles keyboard input and converts to game commands.

### Movement System
Processes entity movement and collision detection.

### Combat System
Resolves combat between entities.

### AI System
Controls monster behavior and decision making.

### Inventory System
Manages item storage and usage.

### Save/Load System
Handles game serialization and deserialization.

## System Interfaces

All systems follow a common pattern:
```cpp
class System {
    void update(GameState& state, float dt);
    void init();
    void shutdown();
};
```

## System Communication

Systems communicate through:
- Shared game state
- Event messages
- Direct method calls (minimal)

## Adding New Systems

1. Create system class
2. Register with GameManager
3. Define update logic
4. Add to build system
5. Write tests