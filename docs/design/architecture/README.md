# Architecture Documentation

System architecture and component design for Veyrm.

## Structure

### [Systems](systems/)

Individual system architectures and designs.

### [Components](components/)

Component specifications and interfaces.

### [Diagrams](diagrams/)

Architectural diagrams and visual documentation.

## Architectural Overview

Veyrm follows a component-based architecture with clear separation of concerns:

### Core Systems

- **Game Manager**: Central game state and flow control
- **Map System**: Tile-based world representation
- **Entity System**: Players, monsters, and items
- **Render System**: Terminal UI rendering with FTXUI
- **Input System**: Keyboard input handling

### Data Flow

```
Input -> Game Manager -> Systems -> Renderer -> Display
           |              ^
           v              |
        Game State -------+
```

## Design Principles

1. **Separation of Concerns**: Each system has a single responsibility
2. **Data-Driven**: Configuration through JSON files
3. **Testability**: Components are independently testable
4. **Performance**: Optimize hot paths, profile regularly
5. **Extensibility**: Easy to add new content and features

## Key Documents

- [AI Architecture](systems/ai-architecture.md)
- [Item System](systems/item-system.md)
