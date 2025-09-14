# 3) Minimal Architecture

- **Game** — owns world state, message log, turns.
- **Map** — tile grid (# wall, · floor, > stairs) + visibility + memory.
- **DungeonGen** — rooms & corridors; farthest-tile stairs placement (BFS).
- **FOV** — symmetric shadowcasting (8 octants).
- **Entities** — Player, Monster, Item (simple structs).
- **Systems** — Input, Movement, Combat (bump-to-attack), Pickup/Inventory, Monster AI (chase or wander).
- **RendererTUI** — FTXUI layout: map, log, status.
- **Data** — `monsters.json`, `items.json`, `save.json` (autosave).
