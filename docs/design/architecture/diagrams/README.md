# Architecture Diagrams

Visual documentation of system architecture.

## Diagram Types

### System Overview

High-level view of all systems and their interactions.

### Data Flow

How data moves through the game systems.

### Component Relationships

How components relate to each other.

### Sequence Diagrams

Step-by-step flow of key operations.

## Current Diagrams

(Diagrams will be added here as they are created)

## Diagram Format

Diagrams can be provided in:

- ASCII art (for terminal viewing)
- Mermaid format (for rendering)
- SVG/PNG files (for documentation)

## Example: Simple Data Flow

```
Input -> InputHandler -> GameManager -> Systems
             |               |            |
             v               v            v
         Commands        GameState    Entities
                             |            |
                             v            v
                         Renderer -> Terminal
```

## Tools

- Mermaid for flowcharts
- PlantUML for sequence diagrams
- ASCII art for simple diagrams
- draw.io for complex diagrams
