#pragma once

#include "point.h"
#include <vector>
#include <string>
#include <random>

class Map;
class EntityManager;
class Player;

// Spawn table entry for depth-based monster selection
struct SpawnTableEntry {
    std::string species;
    int min_depth;
    int max_depth;
    float weight;
    int threat_value;
};

class SpawnManager {
public:
    SpawnManager();
    ~SpawnManager() = default;
    
    // Initial map population
    void spawnInitialMonsters(Map& map, EntityManager& entity_manager, 
                             const Player* player, int depth = 1);
    
    // Dynamic spawning during gameplay
    void update(Map& map, EntityManager& entity_manager, 
               const Player* player, int depth = 1);
    
    // Get valid spawn points on the map
    std::vector<Point> getValidSpawnPoints(const Map& map, const Player* player) const;
    
    // Get spawn points separated by room/corridor
    std::vector<Point> getRoomSpawnPoints(const Map& map, const Player* player) const;
    std::vector<Point> getCorridorSpawnPoints(const Map& map, const Player* player) const;
    
    // Select appropriate monster for current depth
    std::string selectMonsterSpecies(int depth, std::mt19937& rng) const;
    
    // Check if a point is valid for spawning
    bool isValidSpawnPoint(const Map& map, const Player* player, int x, int y) const;
    
    // Get total threat value of all monsters
    int getCurrentThreatLevel(const EntityManager& entity_manager) const;
    
    // Configuration
    void setSpawnRate(int turns) { spawn_rate = turns; }
    void setMaxMonsters(int max) { max_monsters = max; }
    void setMinSpawnDistance(int dist) { min_spawn_distance = dist; }
    void setInitialMonsterCount(int count) { initial_monster_count = count; }
    
private:
    // Spawn timing
    int turns_since_spawn;
    int spawn_rate;  // Turns between spawn attempts
    
    // Spawn limits
    int max_monsters;
    int initial_monster_count;
    int min_spawn_distance;  // Minimum distance from player
    
    // Spawn outside FOV
    bool spawn_outside_fov;
    
    // Room spawn preference
    float room_spawn_percentage;
    
    // Random number generator
    mutable std::mt19937 rng;
    
    // Build spawn table based on available monsters
    void buildSpawnTable();
    std::vector<SpawnTableEntry> spawn_table;
    
    // Helper to calculate distance between points
    int distance(int x1, int y1, int x2, int y2) const;
};