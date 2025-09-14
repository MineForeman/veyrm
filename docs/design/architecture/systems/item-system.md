# Item System Architecture

## Overview

The Item System in Veyrm provides a flexible, data-driven framework for managing collectible objects in the game world. It follows the factory pattern for item creation and uses a centralized manager for world item lifecycle.

## Core Components

### 1. Item (`include/item.h`)
The base representation of all collectible objects.

```cpp
class Item {
public:
    // Position
    int x, y;

    // Identity
    std::string id;
    std::string name;
    std::string description;

    // Visual
    char symbol;
    std::string color;

    // Properties
    ItemType type;
    int value;
    int weight;
    bool stackable;
    int stack_size;
    int max_stack;

    // Flexible properties for effects
    std::map<std::string, int> properties;
};
```

### 2. ItemFactory (`include/item_factory.h`)
Singleton factory for creating items from JSON templates.

```cpp
class ItemFactory {
    static ItemFactory& getInstance();
    void loadFromJson(const std::string& filename);
    std::unique_ptr<Item> create(const std::string& item_id);
    std::string getRandomItemForDepth(int depth) const;
};
```

### 3. ItemManager (`include/item_manager.h`)
Manages all items currently in the game world.

```cpp
class ItemManager {
    void spawnItem(const std::string& item_id, int x, int y);
    void spawnRandomItem(int x, int y, int depth);
    void removeItem(Item* item);
    Item* getItemAt(int x, int y);
    void spawnGold(int x, int y, int amount);
};
```

## Data Flow

```
items.json → ItemFactory → Item Instance → ItemManager → Game World
                ↓                              ↓
           Template Storage              World Position
                                              ↓
                                         Rendering System
                                              ↓
                                         Player Interaction
```

## JSON Data Format

Items are defined in `data/items.json`:

```json
{
  "id": "potion_minor",
  "name": "Minor Healing Potion",
  "description": "A small vial of red liquid",
  "type": "potion",
  "symbol": "!",
  "color": "red",
  "value": 50,
  "weight": 1,
  "stackable": false,
  "properties": {
    "heal": 10
  },
  "min_depth": 1,
  "max_depth": 5
}
```

## Integration Points

### Map Generation
```cpp
// In GameManager::initializeMap()
item_manager->spawnRandomItem(x, y, current_depth);
item_manager->spawnGold(x, y, amount);
```

### Rendering
```cpp
// In MapRenderer::renderTerrainWithPlayer()
if (map.isVisible(x, y)) {
    for (const auto& item : items) {
        if (item->x == x && item->y == y) {
            // Render item symbol with color
        }
    }
}
```

### Player Interaction
```cpp
// In GameScreen input handling
case InputAction::GET_ITEM:
    auto item = item_manager->getItemAt(player->x, player->y);
    if (item) {
        if (item->type == Item::ItemType::GOLD) {
            player->gold += item->properties["amount"];
        }
        item_manager->removeItem(item);
    }
```

## Memory Management

- **ItemFactory**: Singleton pattern, cleaned up on program exit
- **Items**: Owned by ItemManager using `std::unique_ptr`
- **Templates**: Stored in ItemFactory, shared across all items
- **Cleanup**: Automatic via RAII and smart pointers

## Performance Considerations

### Current Implementation
- O(n) lookup for items at position
- Linear search through all items
- Suitable for ~100 items per level

### Future Optimizations
- Spatial indexing (quadtree/grid)
- Item pooling for frequently created/destroyed items
- Lazy loading of item descriptions

## Extensibility

### Adding New Item Types
1. Add entry to `items.json`
2. No code changes required for basic items
3. Special effects need handler in use system

### Adding New Properties
1. Add to properties map in JSON
2. Access via `item->properties["key"]`
3. Type-safe wrappers can be added as needed

## Configuration

### config.yml Integration
```yaml
paths:
  data_dir: "data"  # Location of items.json

game:
  item_spawn_rate: 1.0  # Multiplier for item generation
  gold_spawn_rate: 1.0  # Multiplier for treasure
```

## Testing

### Unit Tests
- Item creation and properties
- Factory template loading
- Manager spawn/remove operations
- Edge cases (invalid positions, etc.)

### Integration Tests
- Items spawn during map generation
- Items render correctly
- Pickup system works
- Gold accumulation

## Future Enhancements

### Phase 10.2: Inventory System
- Player inventory container
- Weight/encumbrance limits
- Inventory UI screen

### Phase 10.3: Item Use
- Consumable effects
- Equipment system
- Item identification

### Phase 10.4: Advanced Items
- Procedural generation
- Magic properties
- Cursed items
- Artifacts

## Best Practices

1. **Always validate spawn positions** - Check bounds and walkability
2. **Use factory for all creation** - Ensures consistent initialization
3. **Clean up on item removal** - Prevent memory leaks
4. **Log item events** - For debugging and balance
5. **Test with edge cases** - Empty maps, full stacks, etc.

## Common Patterns

### Spawning Items in Rooms
```cpp
for (const Room& room : map->getRooms()) {
    int x = room.x + 1 + (rand() % (room.width - 2));
    int y = room.y + 1 + (rand() % (room.height - 2));
    if (map->isWalkable(x, y)) {
        item_manager->spawnRandomItem(x, y, depth);
    }
}
```

### Checking for Items
```cpp
auto items = item_manager->getItemsAt(x, y);
for (auto* item : items) {
    // Process each item at position
}
```

### Gold Handling
```cpp
auto item = item_manager->getItemAt(x, y);
if (item && item->type == Item::ItemType::GOLD) {
    int amount = item->properties["amount"];
    player->gold += amount;
    message_log->addMessage("You gain " + std::to_string(amount) + " gold.");
    item_manager->removeItem(item);
}
```

## Debugging

### Logging Categories
- `LOG_INFO` - Item spawning
- `LOG_DEBUG` - Template loading
- `LOG_ERROR` - Invalid operations

### Common Issues
1. Items not appearing - Check FOV and visibility
2. Pickup not working - Verify input mapping
3. Wrong colors - Check color string mapping
4. Memory leaks - Ensure proper cleanup

## Dependencies

- **nlohmann/json** - JSON parsing
- **FTXUI** - Rendering colors
- **Standard Library** - Containers and algorithms