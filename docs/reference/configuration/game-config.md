# Veyrm Configuration Guide

## Overview

Veyrm uses a YAML-based configuration system that allows customization of game parameters without recompilation. The configuration file `config.yml` is loaded at startup, and values can be overridden via command-line arguments.

## Configuration File Location

The default configuration file is `config.yml` in the project root directory.

## Configuration Priority

1. **Command-line arguments** (highest priority)
2. **config.yml values**
3. **Hardcoded defaults** (lowest priority)

## Configuration Structure

### Game Settings

```yaml
game:
  # Starting map type: procedural, dungeon, room, corridor, arena, stress
  default_map: procedural
  
  # Enable debug mode (shows FPS, coordinates, etc.)
  debug_mode: false
  
  # Difficulty settings
  difficulty:
    monster_damage_multiplier: 1.0
    player_health_multiplier: 1.0
    experience_multiplier: 1.0
```

### Display Settings

```yaml
display:
  # Terminal theme: auto, dark, light, high_contrast
  theme: auto
  
  # Show FPS counter
  show_fps: false
  
  # Message log settings
  message_log:
    max_messages: 100      # History buffer size
    visible_messages: 5    # Lines shown in UI
  
  # UI Layout
  layout:
    min_terminal_width: 80
    min_terminal_height: 24
```

### Map Generation

```yaml
map_generation:
  # Random seed (empty for random, or specify a number)
  seed: ""
  
  procedural:
    # Map dimensions (Angband standard: 198x66)
    width: 198
    height: 66
    
    # Room generation
    min_rooms: 9
    max_rooms: 20
    min_room_size: 4
    max_room_size: 20
    
    # Corridor settings
    corridor_style: straight  # straight, l_shaped, winding
    
    # Chance for rooms to be lit (0.0 to 1.0)
    lit_room_chance: 0.95
    
    # Chance for doors between rooms (0.0 to 1.0)
    door_chance: 0.15
```

### Monster Settings

```yaml
monsters:
  # Maximum monsters per level
  max_per_level: 30
  
  # Spawn rate (turns between spawn checks)
  spawn_rate: 100
  
  # Monster behavior
  behavior:
    aggression_radius: 10      # Detection range in tiles
    door_pursuit_chance: 0.7   # Chance to pursue through doors
```

### Player Settings

```yaml
player:
  # Starting stats
  starting_hp: 20
  starting_attack: 5
  starting_defense: 2
  
  # Inventory
  inventory_capacity: 26  # a-z slots
  
  # Vision
  fov_radius: 10         # Field of view radius in tiles
```

### Path Settings

```yaml
paths:
  # Data directory (contains monsters.json, items.json, etc.)
  data_dir: "data"
  
  # Save game directory
  save_dir: "saves"
  
  # Log directory (for debug logs)
  log_dir: "logs"
```

### Performance Settings

```yaml
performance:
  # Target FPS (affects game loop timing)
  target_fps: 60
  
  # Enable multithreading for map generation
  multithread_generation: true
  
  # Cache size for FOV calculations
  fov_cache_size: 100
```

### Development Settings

```yaml
development:
  # Enable assertions in release builds
  release_assertions: false
  
  # Verbose logging
  verbose_logging: false
  
  # Auto-save interval (seconds, 0 to disable)
  autosave_interval: 300
```

### Keybindings

```yaml
keybindings:
  # Actions
  wait: "."
  pickup: "g"
  inventory: "i"
  drop: "d"
  use: "a"
  examine: "x"
  
  # System
  quit: "q"
  save: "S"
  help: "?"
  new_game: "N"
  
  # Note: Arrow keys are always enabled for movement
  # Vi-keys (hjkl) are NOT enabled by default
```

## Command-Line Overrides

Command-line arguments can override any configuration value:

```bash
# Override map type
./build/bin/veyrm --map arena

# Override data directory
./build/bin/veyrm --data-dir custom/data

# Enable debug mode
./build/bin/veyrm --debug

# Set specific seed
./build/bin/veyrm --seed 12345
```

## Creating Custom Configurations

1. Copy the default `config.yml`:
   ```bash
   cp config.yml my_config.yml
   ```

2. Edit your custom configuration:
   ```yaml
   # my_config.yml
   game:
     default_map: arena
   player:
     starting_hp: 100
   ```

3. Load with custom config (feature planned):
   ```bash
   ./build/bin/veyrm --config my_config.yml
   ```

## Configuration Tips

### For Testing
- Set `lit_room_chance: 1.0` to make all rooms lit
- Increase `starting_hp` for easier gameplay
- Set a fixed `seed` for reproducible maps

### For Development
- Enable `debug_mode: true` to see coordinates
- Set `verbose_logging: true` for detailed logs
- Reduce `autosave_interval` for frequent saves

### For Performance
- Adjust `target_fps` based on your system
- Toggle `multithread_generation` if having issues
- Increase `fov_cache_size` for complex maps

## Default Values Reference

If a value is not specified in config.yml, these defaults are used:

- Map size: 198x66 (Angband standard)
- Lit room chance: 30%
- Player HP: 20
- Player Attack: 5
- Player Defense: 2
- FOV Radius: 10
- Max messages: 100
- Visible messages: 5

## Validation

The configuration system validates values at load time:
- Numeric values must be within reasonable ranges
- Enum values (like map types) must be valid
- File paths are checked for existence where applicable

Invalid values will fall back to defaults with a warning message.