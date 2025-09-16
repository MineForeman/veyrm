# Veyrm Controls Reference

## Movement

### Arrow Keys & Numpad

The primary movement controls use arrow keys for 4-directional movement and numpad for 8-directional movement including diagonals.

| Key | Action |
|-----|--------|
| **↑** / **8** (numpad) | Move north |
| **↓** / **2** (numpad) | Move south |
| **←** / **4** (numpad) | Move west |
| **→** / **6** (numpad) | Move east |
| **7** (numpad) | Move northwest |
| **9** (numpad) | Move northeast |
| **1** (numpad) | Move southwest |
| **3** (numpad) | Move southeast |
| **5** (numpad) / **.** | Wait one turn |

**Note:** Vi-style movement keys (hjkl/yubn) are not currently implemented, though they appear in some older documentation.

## Actions

### Item Management

| Key | Action |
|-----|--------|
| **g** | Get/pickup item at current position |
| **i** | Open inventory screen |
| **u** | Use selected item |
| **D** | Drop selected item (uppercase D) |
| **E** | Examine selected item (uppercase E) |

### Game Management

| Key | Action |
|-----|--------|
| **S** | Save game (uppercase S) |
| **L** | Load game (uppercase L) |
| **N** | New game (from menu) |
| **q** / **Q** | Quit to menu |

### Environment

| Key | Action |
|-----|--------|
| **o** | Open/close adjacent doors |
| **.** | Wait one turn (pass) |

### System

| Key | Action |
|-----|--------|
| **?** | Show help screen |
| **Enter** | Confirm selection/action |
| **Escape** | Cancel/return to previous screen |
| **F1** | Toggle debug mode (when available) |

## Combat

Combat is automatic when you move into an enemy (bump-to-attack). No special keys required.

## Inventory Navigation

When the inventory screen is open:

| Key | Action |
|-----|--------|
| **↑** / **↓** | Navigate items |
| **a-z** | Direct selection (future) |
| **u** | Use selected item |
| **D** | Drop selected item |
| **E** | Examine selected item |
| **Escape** | Close inventory |

## Menu Navigation

In menus and selection screens:

| Key | Action |
|-----|--------|
| **↑** / **↓** | Navigate options |
| **Enter** | Select option |
| **Escape** | Go back/cancel |
| **q** | Quit |

## Build Script Testing Keys

For automated testing with `./build.sh keys`:

| Code | Produces |
|------|----------|
| `\n` | Enter key |
| `\u` | Up arrow |
| `\d` | Down arrow |
| `\l` | Left arrow |
| `\r` | Right arrow |
| Other chars | Literal character |

Example: `./build.sh keys '\njjjq'` sends Enter, three j's, and q.

## Quick Reference Card

### Essential Keys

- **Movement**: Arrow keys or Numpad 1-9
- **Get Item**: g
- **Inventory**: i
- **Save/Load**: S / L
- **Quit**: q
- **Help**: ?
- **Wait**: . or 5

### In Inventory

- **Navigate**: Arrow keys
- **Use**: u
- **Drop**: D (uppercase)
- **Examine**: E (uppercase)
- **Exit**: Escape

## Notes

1. **Case Sensitivity**: Some commands require uppercase letters (S, L, D, E) to avoid conflicts with future item slot selection.

2. **Diagonal Movement**: Currently only available via numpad, not letter keys.

3. **No Ranged Combat**: All combat is melee range (bump-to-attack).

4. **Auto-pickup**: Gold is automatically collected when you walk over it.

5. **Turn Consumption**: Most actions consume a turn, allowing monsters to act.

## Configuration

Key bindings are defined in `src/input_handler.cpp` and cannot currently be customized by users. Future versions may support key remapping through the configuration file.
