# Veyrm Door System

## Overview

The door system in Veyrm provides interactive barriers between rooms and corridors. Doors can be opened and closed by the player, affecting both movement and visibility.

## Door Types

### Tile Types

- **DOOR_CLOSED** (`TileType::DOOR_CLOSED`)
  - Symbol: `▦`
  - Color: Yellow
  - Blocks movement: Yes
  - Blocks visibility: Yes
  - Can be opened by player

- **DOOR_OPEN** (`TileType::DOOR_OPEN`)
  - Symbol: `▢`
  - Color: Yellow
  - Blocks movement: No
  - Blocks visibility: No
  - Can be closed by player

## Player Interaction

### Controls

- **Key:** `o` (lowercase letter O)
- **Action:** Toggle adjacent doors open/closed
- **Range:** All 8 adjacent tiles (cardinal and diagonal)

### Behavior

1. When the player presses 'o', the game checks all 8 adjacent tiles
2. Any closed doors are opened
3. Any open doors are closed
4. Multiple doors can be toggled in a single action
5. Opening/closing doors takes one turn
6. Appropriate messages are added to the message log

### Example Messages

```
You open the door.
You close the door.
There is no door here.
(Multiple doors toggled)
```

## Map Generation

### Automatic Placement

Doors are automatically placed during procedural map generation:

1. **Room Entrances:** Doors are placed at connection points between rooms and corridors
2. **Corridor Junctions:** Doors may appear at corridor intersections
3. **Strategic Locations:** Doors are positioned to create tactical choices

### Placement Algorithm

The `placeDoorsAtRoomEntrances()` function in `MapGenerator`:

1. Scans the perimeter of each room
2. Identifies potential doorway positions (floor tiles adjacent to walls)
3. Places doors at suitable locations
4. Ensures doors don't block critical paths

## Technical Implementation

### Code Structure

- **Input Handling:** `src/input_handler.cpp` - Maps 'o' key to `OPEN_DOOR` action
- **Door Logic:** `src/game_screen.cpp` - `handleDoorInteraction()` method
- **Map Generation:** `src/map_generator.cpp` - Door placement during generation
- **Tile Properties:** `src/map.cpp` - Door tile properties and behavior

### Door Interaction Code

```cpp
bool GameScreen::handleDoorInteraction() {
    // Check all 8 directions for doors
    const int dirs[8][2] = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0},   // Cardinal
        {-1, -1}, {1, -1}, {-1, 1}, {1, 1}  // Diagonal
    };

    for (const auto& dir : dirs) {
        int check_x = player->x + dir[0];
        int check_y = player->y + dir[1];

        TileType tile = map->getTile(check_x, check_y);

        if (tile == TileType::DOOR_CLOSED) {
            map->setTile(check_x, check_y, TileType::DOOR_OPEN);
            msg_log->addMessage("You open the door.");
        } else if (tile == TileType::DOOR_OPEN) {
            map->setTile(check_x, check_y, TileType::DOOR_CLOSED);
            msg_log->addMessage("You close the door.");
        }
    }
}
```

## Gameplay Impact

### Tactical Considerations

1. **Line of Sight:** Closed doors block monster vision
2. **Chokepoints:** Doors create defensive positions
3. **Escape Routes:** Can close doors to slow pursuit
4. **Exploration:** Closed doors hide room contents
5. **Sound:** (Future feature) Door operations could alert nearby monsters

### Monster Interaction

Currently, monsters cannot open doors. This creates:
- Safe zones when doors are closed
- Strategic retreat options
- Puzzle-like navigation challenges

## Logging

Door interactions are logged to `logs/veyrm_env.log`:

```
18:45:12.358 [ENV] Door placed at (26, 27)
18:45:40.116 [ENV] Door opened at (91, 22)
18:45:40.167 [ENV] Door closed at (91, 22)
```

## Future Enhancements

### Planned Features

1. **Door Materials:** Wooden, stone, metal doors with different properties
2. **Locked Doors:** Require keys or lockpicking
3. **Breakable Doors:** Can be destroyed by strong monsters
4. **Secret Doors:** Hidden doors requiring detection
5. **Door Sounds:** Audio feedback for door operations
6. **Monster Door AI:** Some monsters can open doors
7. **Door Traps:** Trapped doors that trigger effects

### Potential Mechanics

- **Door HP:** Doors could have hit points and be damaged
- **Door States:** Stuck, jammed, or barricaded doors
- **Automatic Doors:** Magic or mechanical self-opening doors
- **Portcullises:** Vertical sliding doors
- **Double Doors:** Wide passages with paired doors

## Configuration

Door behavior can be configured in future versions via `config.yaml`:

```yaml
doors:
  spawn_chance: 0.3
  monster_can_open: false
  require_adjacent: true
  break_on_damage: false
  auto_close_turns: 0
```

## Testing

### Test Coverage

- Door placement during map generation
- Player door interaction
- Door state persistence
- Movement blocking when closed
- Visibility blocking when closed
- Multiple door handling

### Debug Commands

Future debug commands for door testing:
- Toggle all doors open/closed
- Spawn door at cursor
- Show door states overlay
- Test door pathfinding