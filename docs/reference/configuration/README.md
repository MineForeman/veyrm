# Configuration Reference

Game configuration files and data formats.

## Contents

- **[game-config.md](game-config.md)** - Game configuration options

## Configuration Files

### Monster Data (data/monsters.json)

```json
{
  "id": "gutter_rat",
  "name": "Gutter Rat",
  "glyph": "r",
  "color": "brown",
  "hp": 3,
  "atk": [1, 3],
  "def": 0,
  "speed": 100,
  "depth_min": 1,
  "depth_max": 3
}
```

### Item Data (data/items.json)

```json
{
  "id": "potion_minor",
  "name": "Minor Healing Potion",
  "glyph": "!",
  "color": "red",
  "type": "consumable",
  "heal": 6,
  "weight": 0.5,
  "value": 10
}
```

### Save Game Format (saves/*.json)

```json
{
  "version": "1.0.0",
  "timestamp": "2025-09-14T12:00:00Z",
  "map_seed": 12345,
  "player": { ... },
  "entities": [ ... ],
  "items": [ ... ]
}
```

## Configuration Guidelines

- All data files use JSON format
- IDs must be unique within their category
- Colors use named colors or RGB values
- Numeric ranges use [min, max] format
