# Component Documentation

Component specifications and interface definitions.

## Component Architecture

Veyrm uses a component-based architecture where game objects are composed of reusable components.

## Current Components

### Entity Components

- **Position**: Location in the game world
- **Renderable**: Visual representation
- **Stats**: Health, attack, defense
- **AI**: Behavior patterns
- **Inventory**: Item storage

### Map Components

- **Tile**: Basic map unit
- **Room**: Rectangular areas
- **Corridor**: Connections between rooms

### UI Components

- **MapView**: Viewport rendering
- **MessageLog**: Message display
- **StatusBar**: Player information
- **InventoryView**: Item list display

## Component Guidelines

1. Components should be data-only when possible
2. Systems operate on components
3. Components can be combined freely
4. Prefer composition over inheritance

## Future Components

- **Effects**: Status effects and buffs
- **Equipment**: Wearable items
- **Skills**: Player abilities
- **Dialogue**: NPC conversations
