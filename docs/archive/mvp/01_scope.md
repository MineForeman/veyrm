# 1) Scope (What ships)

- **Turn-based core loop:** Player turn → Monster turn.
- **Map:** Single floor, ~80×24 visible; rooms + corridors; doors optional later.
- **FOV & memory:** Symmetric shadowcasting; explored tiles remembered.
- **Actors:** Player + 2 monsters (rat, orc); melee only.
- **Stats/Combat:** HP, Attack, Defense; simple energy scheduler optional later.
- **Items:** Ground pickups: healing potion; inventory 10 slots; use/drop.
- **UI panes:** Map, 5–6 line message log (scrollback later), status bar.
- **Input:** arrows/hjkl (+ diagonals y u b n), `g` get, `i` inventory, `u` use, `D` drop, `.` wait, `q` quit, `N` new game.
- **Save/Load:** One autosave in JSON on quit; auto-continue on boot.
- **Data:** Monsters & items defined in JSON tables.

## Non-goals (later)

Multiple floors, ranged/magic, shops, quests, factions, tilesets, mouse, rebindable keys, modding, netcode, hunger, complex AI.
