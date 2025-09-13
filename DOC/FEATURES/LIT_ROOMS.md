# Lit Rooms

## Overview

Lit rooms are a classic roguelike feature from Angband where certain dungeon rooms are permanently illuminated. When the player enters a lit room, the entire room becomes immediately visible, revealing all its contents at once.

## Gameplay Mechanics

### Visibility

- **Entry Effect**: Upon entering a lit room, the entire room (walls and floor) becomes instantly visible
- **Memory**: Lit rooms remain bright in the player's memory, unlike normal tiles which dim
- **Exit Effect**: When leaving a lit room, it stays bright in memory but entities are no longer visible
- **Message**: "The room is lit!" appears when entering a lit room

### Strategic Considerations

**Advantages:**
- Full room visibility allows better tactical planning
- Can see all doors, items, and room layout immediately
- Easier to navigate and explore thoroughly
- No surprises from hidden corners

**Disadvantages:**
- All monsters in the room are revealed at once
- No element of surprise for the player
- Monsters can see you clearly as well
- Cannot use darkness for tactical advantage

### Generation

- **Probability**: 30% chance for each room to be lit during map generation
- **Distribution**: Randomly distributed throughout the dungeon
- **Types**: Currently applies to all room types equally

## Technical Implementation

### Data Structure

```cpp
class Room {
    bool lit = false;  // Angband-style lit room flag
    
    bool isLit() const { return lit; }
    void setLit(bool value) { lit = value; }
};
```

### FOV Integration

The Field of View system is overridden for lit rooms:

1. Normal FOV calculation occurs first
2. If player is in a lit room, entire room is added to visible tiles
3. Room walls and adjacent tiles are also revealed
4. Map memory preserves lit room brightness

### Rendering

Lit rooms use special rendering rules:

- **Visible**: Full bright colors when player is present
- **Memory**: Bright colors (not dimmed) when explored but not visible
- **Entities**: Only shown when room is in current FOV

## Configuration

Currently lit rooms use fixed parameters:

```cpp
const float LIT_ROOM_CHANCE = 0.3f;  // 30% probability
```

Future enhancements could include:
- Configurable lit room probability
- Different lighting levels (dim, bright, magical)
- Player-controlled lighting (torches, spells)
- Time-based lighting changes

## Testing

The lit room system includes comprehensive tests:

- Room attribute persistence
- Map storage and retrieval
- FOV override behavior  
- Rendering differentiation
- Message system integration

See `tests/test_lit_rooms.cpp` for test implementation.

## Future Enhancements

Potential improvements to the lit room system:

1. **Dynamic Lighting**
   - Torches that can light dark rooms
   - Spells to create/remove light
   - Light sources with radius effects

2. **Special Lit Room Types**
   - Treasury rooms (always lit, contain treasure)
   - Temple rooms (magically lit, contain altars)
   - Library rooms (well-lit, contain books/scrolls)

3. **Light Interactions**
   - Monsters that prefer/avoid light
   - Stealth mechanics affected by lighting
   - Light-based puzzles and triggers

4. **Visual Effects**
   - Gradual light transitions
   - Flickering torch effects
   - Colored lighting for special rooms

## References

- Original Angband source code
- Roguelike development forums discussions on lighting
- Classic roguelike lighting mechanics analysis