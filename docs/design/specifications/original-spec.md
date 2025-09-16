# Veyrm Game Specification - Complete Documentation Index

A comprehensive index and summary of all documentation for the Veyrm roguelike project, covering both the MVP implementation specifications and the world lore.

---

## Part 1: MVP Implementation Documentation

### [00_README.md](MVP/00_README.md) - MVP Overview

**Summary:** Introduction to the minimal viable product for a modern C++ Angband-style roguelike using FTXUI for terminal UI. Establishes the foundation starting in Ring 1: Woundworks with basic enemies and items.

- **Tech Stack:** C++23, CMake ≥3.25, FTXUI, nlohmann/json, Catch2
- **Starting Point:** Ring 1: Woundworks with Gutter Rats, Orc Rooklings, healing potions

### [01_scope.md](MVP/01_scope.md) - Core Features

**Summary:** Defines what ships in v0.1 and what doesn't.

- **Included:** Turn-based combat, single floor (80×24), FOV & memory, 2 monsters, healing potions, 10-slot inventory, autosave
- **Excluded:** Multiple floors, ranged/magic, shops, quests, factions, complex AI
- **Controls:** Arrow/hjkl movement, g (get), i (inventory), u (use), D (drop), q (quit)

### [02_tech_stack.md](MVP/02_tech_stack.md) - Technology Choices

**Summary:** Technical foundation and dependencies.

- **UI:** FTXUI for clean terminal layouts with Unicode and color support
- **Build:** CMake with FetchContent or vcpkg for dependencies
- **Data:** nlohmann/json for saves and content tables
- **RNG:** std::mt19937_64 (PCG planned for future)
- **CI:** GitHub Actions for cross-platform builds

### [03_architecture.md](MVP/03_architecture.md) - System Design

**Summary:** Component-based architecture breakdown.

- **Core Systems:** Game (state), Map (tiles), DungeonGen, FOV, Entities, Systems, RendererTUI
- **Map Tiles:** # (wall), · (floor), > (stairs)
- **AI:** Simple chase or wander behavior
- **Data Files:** monsters.json, items.json, save.json

### [04_algorithms.md](MVP/04_algorithms.md) - Core Algorithms

**Summary:** Key algorithmic implementations.

- **Map Gen:** Random room placement (10-14 rooms), L-shaped corridors
- **FOV:** Symmetric shadowcasting with exploration memory
- **Pathfinding:** 4-directional BFS for monster movement
- **Combat:** Bump-to-attack, player 1d6 damage, armor flat reduction

### [05_milestones.md](MVP/05_milestones.md) - Development Phases

**Summary:** Nine development milestones from skeleton to v0.1.

1. Skeleton (build, main loop, input)
2. Map generation with player movement
3. FOV and memory system
4. Monsters and combat
5. Items and inventory
6. Stairs placement
7. Save/load system
8. Balance and testing
9. Package v0.1

### [06_acceptance.md](MVP/06_acceptance.md) - Acceptance Criteria

**Summary:** Checklist for MVP completion.

- Map connectivity and stairs reachability
- FOV hides/reveals properly with memory
- Two functional monster types
- Working potion healing and inventory
- Autosave round-trip integrity
- Cross-platform terminal support

### [07_controls.md](MVP/07_controls.md) - Control Scheme

**Summary:** Complete keyboard controls.

- **Movement:** Arrows/hjkl + diagonals (yubn)
- **Actions:** g (get), i (inventory), u (use item 0-9), D (drop item 0-9)
- **System:** . (wait), N (new game), q (quit with autosave)

### [08_build_run.md](MVP/08_build_run.md) - Build Instructions

**Summary:** CMake setup and build process.

- Quick build commands for Release/Debug modes
- CMakeLists.txt template with FTXUI and nlohmann/json
- UTF-8 terminal requirement
- Minimum 80×24 terminal size

### [09_json_seeds.md](MVP/09_json_seeds.md) - Data Templates

**Summary:** JSON templates for game content matching Ring 1-2.

- **Monsters:** gutter_rat (3hp), orc_rookling (8hp) with attack/defense stats
- **Items:** potion_minor (+6 heal), scroll_light (reveal FOV), rope_spike (place ladder)

### [10_future.md](MVP/10_future.md) - Post-MVP Features

**Summary:** Planned enhancements after v0.1.

- Multiple floors with hunger clock
- Ranged combat and spells
- Advanced AI with FOV awareness
- Key rebinding and config files
- Seeded runs for sharing
- Turning stairs mechanic

---

## Part 2: World Documentation

### [01_high_concept.md](WORLD/01_high_concept.md) - Core Premise

**Summary:** The world's broken state and the hero's quest.

- **Setting:** Veyrmspire citadel above the Spiral Vaults (100 descending rings)
- **Quest:** Shatter the last shard binding the fallen crown
- **Gameplay Loop:** Prepare → descend → return/perish → craft → pledge to Orders → push deeper
- **Tone:** Copper-age grit meets luminous mysticism

### [02_cosmology.md](WORLD/02_cosmology.md) - World Mythology

**Summary:** The cosmic structure and history.

- **Creation Myth:** The Never-King's shattered crown chains the world
- **The Vaults:** Semi-living dungeon that remembers delvers
- **Time Cycles:** Red Year (heat), Glass Year (fragility), Green Year (growth)

### [03_regions.md](WORLD/03_regions.md) - Surface Geography

**Summary:** The lands above the Vaults.

- **Veyrmspire:** Basalt city built on the dungeon entrance
- **Surrounding Regions:** Windbreak, Saltglass, Basilica territories
- **Trade Routes:** Ropeways, dune-skiffs, caravans

### [04_orders_and_callings.md](WORLD/04_orders_and_callings.md) - Class System

**Summary:** Six Orders mapping to classic roguelike classes.

- **Warrior:** Red Anvil order, soldiers and guards
- **Rogue:** Sable Ledger, thieves and information brokers
- **Ranger:** Windfall wardens, scouts and hunters
- **Mage:** Loomkeepers, scholars of ancient magic
- **Priest:** Candle-Wound clergy, healers with strict oaths
- **Paladin:** Bridge-builders, divine warriors

### [05_peoples.md](WORLD/05_peoples.md) - Playable Lineages

**Summary:** The various peoples of Veyrm.

- **Humans:** Versatile, dominant population
- **Other Lineages:** Various fantasy races adapted to the broken world
- **Cultural Notes:** Each lineage has ties to specific Orders

### [06_spiral_vaults.md](WORLD/06_spiral_vaults.md) - The Megadungeon

**Summary:** Structure and nature of the 100-ring descent.

- **Ring Organization:** Each ring deeper = older civilizations
- **Living Dungeon:** The Vaults remember and adapt to delvers
- **Turning Stairs:** Passages shift between expeditions
- **Notable Zones:** Woundworks (Ring 1), deeper mysteries below

### [07_foes.md](WORLD/07_foes.md) - Enemy Types

**Summary:** Monster castes and exemplars.

- **Tier 1:** Gutter Rats, Orc Rooklings (MVP enemies)
- **Boss Types:** The Rusted Bailiff (Ring 1 mini-boss)
- **Undark:** Corrupted creatures from deeper rings
- **Brass Mites:** Invasive mechanical pests

### [08_magic_and_relics.md](WORLD/08_magic_and_relics.md) - Magic System

**Summary:** Spells, prayers, and legendary items.

- **Cantrips:** Spark, Push, Fray, Tether, Light-Knot
- **Prayers:** Candle's Breath (heal), Warding Name, Mercy Pulse
- **Oath-Edges:** Paladin abilities (Bulwark, Brand, Bridge)
- **Relics:** Spindle of Quiet, Coin of the Eighth Face, Glaive of Notches

### [09_economy_time_travel.md](WORLD/09_economy_time_travel.md) - World Systems

**Summary:** Currency, supplies, and transportation.

- **Currency:** "Nails" (iron spikes) bundled and guild-stamped
- **Supplies:** Trail-resin (food), salt cakes, wax-weaves (torches)
- **Calendar:** 10 months × 30 days, three-year cycles
- **Travel:** Ropeways, dune-skiffs, caravans

### [10_key_npcs.md](WORLD/10_key_npcs.md) - Important Characters

**Summary:** Notable NPCs in Veyrmspire.

- **Prelate Liora:** Priest leader, won't heal oathbreakers
- **Master Hadrik:** Warrior trainer, sells "line-steel"
- **Mael:** Rogue fence, trades "memory phrases"
- **Archivist Sere:** Mage scholar, exchanges cantrip scripts
- **Warden Ysra:** Ranger leader, offers bounties on pests

### [11_first_delve_on_ramp.md](WORLD/11_first_delve_on_ramp.md) - Tutorial Area

**Summary:** Ring 1 starting area (The Sunken Gate).

- **Enemies:** Gutter Rats (3hp), Orc Rooklings (8hp)
- **Items:** Healing potions, light scrolls, rope spikes
- **Locations:** Collapsed Barracks, Thread-Knotted Chapel, Nail-Counter's Office
- **Boss:** The Rusted Bailiff (hollow armor animated by debt tallies)

### [12_naming_tables.md](WORLD/12_naming_tables.md) - Name Generation

**Summary:** Random tables for creating Veyrm-appropriate names.

- **Vault Names:** Combinations like "Black Gate", "Brass Court", "Quiet Vault"
- **Relic Epithets:** "of Quiet", "of the Eighth Face", "of Red Year"
- **Method:** Roll 1d20 twice and combine results

### [13_json_seeds.md](WORLD/13_json_seeds.md) - Data Examples

**Summary:** Ready-to-use JSON data for MVP implementation.

- Exact monster stats for gutter_rat and orc_rookling
- Item definitions for potions, scrolls, and utility items
- Direct mapping to MVP requirements

### [14_progression_arcs.md](WORLD/14_progression_arcs.md) - Character Development

**Summary:** How characters grow in power and story.

- **Power Scaling:** From rookie to legendary delver
- **Order Advancement:** Rising through faction ranks
- **Narrative Beats:** Key story moments at certain depths

### [15_style_guide.md](WORLD/15_style_guide.md) - Tone and Presentation

**Summary:** Writing and UI style guidelines.

- **Language:** Copper-age vocabulary, minimal whimsy
- **UI Text:** Terse, evocative messages
- **Color Palette:** Muted earth tones with selective highlights

### [16_roadmap_hooks.md](WORLD/16_roadmap_hooks.md) - Future Features

**Summary:** Post-MVP expansion ideas.

- **Hunger System:** Trail-resin degradation and "resin-jaw" debuff
- **Turning Stairs:** Rotating dungeon connections
- **Choir Mechanics:** Sound-based puzzles and area control
- **Crafting:** Nail bundles → traps, brass mites → pets
- **Factions:** Mycel Courts vs Basilica alignment system

### [README.md](WORLD/README.md) - World Overview

**Summary:** Table of contents and one-paragraph world pitch.

- Complete index of all world documentation
- Quick pitch: Dead god's crown shards chain the world; descend the Spiral Vaults to break them

---

## Quick Reference

### Essential Files for Development

1. **MVP/01_scope.md** - Core feature list
2. **MVP/03_architecture.md** - System design
3. **MVP/08_build_run.md** - Build instructions
4. **MVP/09_json_seeds.md** - Data templates
5. **[TESTS.md](TESTS.md)** - Comprehensive test suite documentation
6. **[IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md)** - Development progress tracking

### Essential Files for World Understanding

1. **WORLD/01_high_concept.md** - Core premise
2. **WORLD/06_spiral_vaults.md** - Dungeon structure
3. **WORLD/11_first_delve_on_ramp.md** - Starting area

### Data Format Examples

```json
// Monster template
{ "id": "gutter_rat", "glyph": "r", "color": "grey", "hp": 3, "atk": [1,3], "def": 0, "speed": 100 }

// Item template
{ "id": "potion_minor", "glyph": "!", "color": "magenta", "heal": 6 }
```

### Key Commands Summary

- **Currently Implemented:** arrows (movement), .(wait), q(quit), Esc(cancel), Enter(confirm)
- **Planned:** hjkl + yubn (diagonals), g(get), i(inventory), u(use), D(drop), N(new)

---

## Current Implementation Status

### ✅ Completed Phases (v0.3.1)

- **Phase 0:** Project Setup (CMake, dependencies, FTXUI window)
- **Phase 1:** Core Game Loop (state management, turn system, input handling)
- **Phase 2:** Map Foundation (tile system, rendering, 5 test map types)
- **Phase 3.1:** Entity System (Entity base class, Player, EntityManager)
- **Phase 3.5:** Comprehensive Test Suite (57 test cases, 404 assertions)

### 🏗️ Current Architecture

- **Entity System:** Base Entity class with Player implementation
- **Map System:** 5 map types (room, dungeon, corridor, arena, stress test)
- **Rendering:** FTXUI-based terminal UI with viewport system
- **Input:** Arrow key movement with collision detection
- **Testing:** Full Catch2 test suite with 100% pass rate

### 📊 Test Coverage

- **Entity System:** 18 test cases covering creation, movement, collision
- **Map System:** 19 test cases covering generation, validation, connectivity
- **UI Systems:** 12 test cases covering input handling and message logging
- **Core Systems:** 8 test cases covering turn management and basic functionality

### 🚀 Next Steps (Phase 4+)

- Monster entities and basic AI
- Combat system implementation  
- Field of view and map memory
- Item system and inventory
- Save/load functionality

### 🔧 Build and Test Commands

```bash
# Build and run
./build.sh run

# Run all tests  
./build.sh test

# Automated testing
./build.sh keys "qqqq\n"
./build.sh dump
```
