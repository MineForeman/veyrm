# Content Creation Guide

Learn how to add custom content to Veyrm - monsters, items, maps, and more.

## Quick Start

1. [Adding Monsters](monsters.md) - Create new enemy types
2. [Adding Items](items.md) - Design weapons, armor, and consumables
3. [Creating Maps](maps.md) - Build custom dungeons
4. [Modding Guide](modding.md) - Advanced modifications

## Content Types

### Data-Driven Content
These can be added by editing JSON files:
- **Monsters** - Edit `data/monsters.json`
- **Items** - Edit `data/items.json`
- **Game Balance** - Edit `config.yml`

### Code-Based Content
These require code changes:
- **Map Types** - New generation algorithms
- **AI Behaviors** - Custom monster AI
- **Game Systems** - New mechanics
- **UI Elements** - Interface modifications

## Adding a Monster

### Quick Example
```json
{
  "id": "fire_drake",
  "name": "Fire Drake",
  "glyph": "D",
  "color": "red",
  "hp": 50,
  "attack": 12,
  "defense": 8,
  "speed": 90,
  "xp_value": 200,
  "description": "A small dragon wreathed in flames"
}
```

Add this to `data/monsters.json` and it will appear in game!

## Adding an Item

### Quick Example
```json
{
  "id": "sword_frost",
  "name": "Frost Sword",
  "glyph": "|",
  "color": "cyan",
  "type": "weapon",
  "damage": 8,
  "value": 250,
  "description": "A blade of eternal ice"
}
```

Add to `data/items.json` to include in the game.

## Content Guidelines

### Balance Considerations
- **HP**: 3-100 for normal enemies
- **Attack**: 1-20 for most monsters
- **Defense**: 0-10 typical range
- **Speed**: 100 is normal, lower is faster
- **Value**: 1-1000 for items

### Visual Design
- **Glyphs**: Single ASCII character
- **Colors**: Use standard terminal colors
- **Names**: Descriptive but concise
- **Descriptions**: Add flavor text

## Testing Your Content

### Quick Test
```bash
# Run game with your content
./build.sh run

# Test specific map type
./build/bin/veyrm --map test

# Enable debug mode
./build/bin/veyrm --debug
```

### Validation
- Check JSON syntax is valid
- Test spawning rates
- Verify balance
- Playtest thoroughly

## Advanced Modding

### Custom Map Generation
1. Create new MapGenerator subclass
2. Implement generation algorithm
3. Register in MapType enum
4. Add to CLI arguments

### Custom AI
1. Extend Monster class
2. Override update() method
3. Implement behavior
4. Register in MonsterFactory

### New Game Systems
1. Design system architecture
2. Integrate with GameManager
3. Add UI components
4. Update save/load

## Best Practices

### Content Creation
- Start small and iterate
- Test each addition
- Maintain game balance
- Consider player fun
- Document your additions

### File Organization
- Keep backups of original files
- Use version control
- Comment complex additions
- Follow existing patterns

## Sharing Content

### Distribution
- Share JSON files directly
- Create content packs
- Submit pull requests
- Document on wiki

### Community
- Share on GitHub
- Get feedback
- Collaborate on packs
- Build on others' work

## Tools

### JSON Validators
- Online: jsonlint.com
- Command line: `jq`
- VS Code: JSON extension

### Testing Tools
```bash
# Validate JSON
jq . data/monsters.json

# Quick spawn test
./build.sh run --debug --spawn fire_drake

# Performance test
./build.sh run --benchmark
```

## Troubleshooting

### Common Issues
- **JSON syntax errors**: Use validator
- **Content not appearing**: Check spawn rates
- **Crashes**: Check data types
- **Balance issues**: Playtest more

### Getting Help
- Check error logs
- Read existing content
- Ask in Issues
- Share your JSON

---

*Start creating: [Adding Monsters](monsters.md) →*