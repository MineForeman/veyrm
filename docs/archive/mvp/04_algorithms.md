# 4) Algorithms

- **Room placement:** Random rectangles, reject on overlap, 10–14 rooms target.
- **Corridors:** L-shaped carve between sorted room centers, with a few extra nearest links.
- **Connectivity check:** BFS implicitly enforced during stairs placement; farthest reachable tile gets `>`.
- **FOV:** Symmetric shadowcasting; explore memory set when visible.
- **Pathfinding:** 4-dir BFS to compute first step toward player; fallback to wandering.
- **Combat:** Bump to attack; player damage 1d6; monsters use [min,max]; armor is a flat reduction.
