# Map Rendering: Problem Analysis & Improvements

## Problem: Invisible Walls Due to Layer Compositing Issue

### Initial Symptoms
- Walls were collision-detected (player couldn't walk through them)
- Walls were counted in debug output (182 walls found)
- Walls were NOT visible on screen in any terminal (black or white background)

### Root Cause Analysis

#### The Layering Problem
The original implementation used FTXUI's `dbox` to composite multiple rendering layers:

```cpp
// Original problematic code
Element composite = dbox({
    terrain,     // Layer 0: Walls, floors, etc.
    entities,    // Layer 1: Monsters, items (future)
    player,      // Layer 2: Player character
    effects      // Layer 3: Visual effects (future)
});
```

The `renderPlayer()` function created a full viewport-sized grid:
```cpp
// This created a grid of spaces with ONE @ character
for (int y = 0; y < viewport_height; y++) {
    for (int x = 0; x < viewport_width; x++) {
        if (x == player_x && y == player_y) {
            row_elements.push_back(text("@"));
        } else {
            row_elements.push_back(text(" ")); // PROBLEM: Spaces are opaque!
        }
    }
}
```

**Key Discovery:** FTXUI's `dbox` treats space characters as opaque. The player layer's spaces were overwriting the entire terrain layer beneath.

#### Color Visibility Issues
Secondary problems discovered:
1. Walls initially set to `Color::White` - invisible on white terminals
2. Void tiles set to `Color::Black` - invisible on black terminals  
3. No consideration for terminal theme compatibility

### Solution Implemented

#### Single-Pass Rendering
Eliminated the layering system in favor of direct rendering:
```cpp
Element MapRenderer::renderTerrainWithPlayer(const Map& map, const GameManager& game) {
    // Check player position first
    if (screen_x == player_screen.x && screen_y == player_screen.y) {
        row_elements.push_back(text("@") | color(Color::White) | bold);
        continue;
    }
    // Otherwise render terrain
    // ...
}
```

#### Terminal-Compatible Colors
Changed to high-visibility colors:
- Walls: `Color::Yellow` (visible on both black and white)
- Floors: `Color::White` (with gray for explored areas)
- Void: Black space (not rendered, natural transparency)

## Improvements to Implement

### 1. Priority-Based Rendering System

**Concept:** Each map position gets rendered based on highest priority element present.

```cpp
enum class RenderPriority {
    TERRAIN = 0,
    ITEMS = 10,
    CREATURES = 20,
    PLAYER = 30,
    EFFECTS = 40,
    UI_OVERLAY = 50
};

struct Renderable {
    char glyph;
    Color fg, bg;
    RenderPriority priority;
    bool blocks_lower; // Does this hide elements below?
};
```

**Benefits:**
- Extensible for future content (monsters, items, spells)
- Clear visual hierarchy
- No layer compositing issues

### 2. Terminal-Adaptive Color Schemes

**Implementation Plan:**
```cpp
class ColorScheme {
public:
    struct Colors {
        Color wall, wall_memory;
        Color floor, floor_memory;
        Color player;
        Color void_tile;
        Color ui_border, ui_text;
    };
    
    static Colors getDarkTheme();
    static Colors getLightTheme();
    static Colors getHighContrast();
    
    // Auto-detect based on terminal background
    static Colors detectBest();
};
```

**Color Palettes:**
- **Dark Terminal:** Bright colors on black
- **Light Terminal:** Dark colors on white  
- **High Contrast:** Maximum differentiation
- **Colorblind:** Patterns + safe colors

### 3. Visual Clarity Improvements

**Unicode Wall Characters:**
```cpp
// Intelligent wall connection
char getWallGlyph(int x, int y) {
    bool n = isWall(x, y-1), s = isWall(x, y+1);
    bool e = isWall(x+1, y), w = isWall(x-1, y);
    
    if (n && s && e && w) return '┼';  // Cross
    if (n && s && !e && !w) return '│'; // Vertical
    if (!n && !s && e && w) return '─'; // Horizontal
    if (n && e) return '└';  // Corner
    // ... etc
}
```

**Tile Variants for Atmosphere:**
- Floors: `.` `,` `·` `'` (subtle variation)
- Walls: Different textures based on position hash
- Water/Lava: Animated `~` `≈` characters

### 4. Debug & Development Tools

**Render Debug Overlay (F3 key):**
```cpp
class RenderDebugger {
    bool show_tile_types;      // Display T/W/F/V for tile types
    bool show_visibility;      // Highlight FOV calculations
    bool show_coordinates;     // Grid coordinates
    bool show_performance;     // FPS, render time
    bool show_color_test;      // All available colors
};
```

**Automated Testing:**
- Extend dump mode to output JSON
- Compare frame renders for regression testing
- Performance benchmarking

### 5. Performance Optimizations

**Dirty Rectangle Tracking:**
```cpp
class DirtyRectTracker {
    std::vector<Rect> dirty_regions;
    
    void markDirty(int x, int y);
    void markDirty(Rect region);
    bool needsRedraw(int x, int y) const;
    void clear();
};
```

**Render Caching:**
- Cache ANSI escape sequences
- Reuse formatted strings
- Skip unchanged cells

**Viewport Culling:**
- Don't process off-screen tiles
- Early exit for out-of-bounds

### 6. Accessibility Features

**Screen Reader Support:**
```cpp
struct TileDescription {
    std::string brief;    // "wall"
    std::string detailed; // "stone wall blocking passage north"
    std::string context;  // "corner of room"
};
```

**Display Modes:**
- **ASCII Only:** No Unicode, maximum compatibility
- **Large Text:** Double-height characters
- **Symbols:** Icons instead of letters for items

## Implementation Priority

1. **Terminal-adaptive colors** (immediate user benefit)
2. **Priority rendering** (foundation for future features)
3. **Visual improvements** (better game feel)
4. **Debug tools** (developer productivity)
5. **Performance** (when needed)
6. **Accessibility** (inclusive design)

## Lessons Learned

1. **Test rendering on multiple terminal themes** - What looks good in one may be invisible in another
2. **Avoid complex layer compositing in TUIs** - Single-pass rendering is simpler and more reliable
3. **Use debug output liberally** - The dump mode was crucial for diagnosis
4. **Spaces are not transparent** - In terminal rendering, every character matters
5. **Simple solutions often best** - Direct rendering fixed what complex layering couldn't

## Next Steps

Begin implementation of improvements in priority order, starting with terminal-adaptive color schemes to ensure all users can see the game properly.