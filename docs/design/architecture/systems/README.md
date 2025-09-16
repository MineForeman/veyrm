# System Architecture Documentation

Detailed architecture documentation for individual systems.

## Contents

- **[ai-architecture.md](ai-architecture.md)** - AI system design
- **[item-system.md](item-system.md)** - Item management architecture

## System Categories

### Core Systems

- Game Manager
- Map System
- Entity System
- Render System

### Gameplay Systems

- Combat System
- Movement System
- Inventory System
- Save/Load System

### Support Systems

- Input Handler
- Message Log
- FOV Calculator
- Pathfinding

## Architecture Principles

1. **Single Responsibility**: Each system handles one aspect
2. **Loose Coupling**: Systems communicate through interfaces
3. **Data Ownership**: Clear ownership of game data
4. **Testability**: Systems can be tested in isolation
