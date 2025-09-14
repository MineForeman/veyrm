#include "spawn_manager.h"
#include "map.h"
#include "entity_manager.h"
#include "player.h"
#include "monster.h"
#include "monster_factory.h"
#include "config.h"
#include "fov.h"
#include "game_state.h"
#include "monster_ai.h"
#include <algorithm>
#include <random>
#include <cmath>

SpawnManager::SpawnManager()
    : game_manager(nullptr),
      turns_since_spawn(0),
      spawn_rate(100),
      max_monsters(30),
      initial_monster_count(10),
      min_spawn_distance(5),
      spawn_outside_fov(true),
      room_spawn_percentage(0.95f),
      rng(std::random_device{}()) {
    
    // Load config values
    Config& config = Config::getInstance();
    initial_monster_count = config.getInitialMonsterCount();
    spawn_rate = config.getMonsterSpawnRate();
    max_monsters = config.getMaxMonstersPerLevel();
    min_spawn_distance = config.getMinSpawnDistance();
    spawn_outside_fov = config.getSpawnOutsideFOV();
    room_spawn_percentage = config.getRoomSpawnPercentage();
    
    buildSpawnTable();
}

SpawnManager::SpawnManager(GameManager* gm)
    : game_manager(gm),
      turns_since_spawn(0),
      spawn_rate(100),
      max_monsters(30),
      initial_monster_count(10),
      min_spawn_distance(5),
      spawn_outside_fov(true),
      room_spawn_percentage(0.95f),
      rng(std::random_device{}()) {

    // Load config values
    Config& config = Config::getInstance();
    initial_monster_count = config.getInitialMonsterCount();
    spawn_rate = config.getMonsterSpawnRate();
    max_monsters = config.getMaxMonstersPerLevel();
    min_spawn_distance = config.getMinSpawnDistance();
    spawn_outside_fov = config.getSpawnOutsideFOV();
    room_spawn_percentage = config.getRoomSpawnPercentage();

    buildSpawnTable();
}

void SpawnManager::buildSpawnTable() {
    // Build spawn table from available monsters
    // For now, hardcode based on our 5 monsters
    spawn_table = {
        {"gutter_rat",    1,  5,  1.0f, 1},
        {"cave_spider",   1,  10, 0.8f, 2},
        {"kobold",        2,  15, 0.7f, 2},
        {"orc_rookling",  3,  20, 0.6f, 3},
        {"zombie",        5,  30, 0.5f, 4}
    };
}

void SpawnManager::spawnInitialMonsters(Map& map, EntityManager& entity_manager, 
                                       const Player* player, int depth) {
    // Get spawn points separated by type
    auto room_points = getRoomSpawnPoints(map, player);
    auto corridor_points = getCorridorSpawnPoints(map, player);
    
    // Shuffle both lists
    std::shuffle(room_points.begin(), room_points.end(), rng);
    std::shuffle(corridor_points.begin(), corridor_points.end(), rng);
    
    // Calculate how many monsters go in rooms vs corridors
    int monsters_in_rooms = static_cast<int>(initial_monster_count * room_spawn_percentage);
    int monsters_in_corridors = initial_monster_count - monsters_in_rooms;
    
    int monsters_spawned = 0;
    
    // Spawn monsters in rooms first
    for (const auto& point : room_points) {
        if (monsters_spawned >= monsters_in_rooms) break;
        
        std::string species = selectMonsterSpecies(depth, rng);
        if (!species.empty()) {
            auto monster = entity_manager.createMonster(species, point.x, point.y);
            if (monster && game_manager && game_manager->getMonsterAI()) {
                // Assign room to monster
                Room* room = map.getRoomAt(point.x, point.y);
                if (room) {
                    Monster* monster_ptr = dynamic_cast<Monster*>(monster.get());
                    if (monster_ptr) {
                        game_manager->getMonsterAI()->assignRoomToMonster(*monster_ptr, room);
                    }
                }
                monsters_spawned++;
            }
        }
    }
    
    // Spawn remaining monsters in corridors
    int corridor_spawned = 0;
    for (const auto& point : corridor_points) {
        if (corridor_spawned >= monsters_in_corridors) break;
        
        std::string species = selectMonsterSpecies(depth, rng);
        if (!species.empty()) {
            auto monster = entity_manager.createMonster(species, point.x, point.y);
            if (monster) {
                // Corridor monsters have no room assignment
                corridor_spawned++;
                monsters_spawned++;
            }
        }
    }
    
    // If we couldn't spawn enough in corridors, try more room points
    if (monsters_spawned < initial_monster_count) {
        for (size_t i = monsters_in_rooms; i < room_points.size() && monsters_spawned < initial_monster_count; i++) {
            const auto& point = room_points[i];
            std::string species = selectMonsterSpecies(depth, rng);
            if (!species.empty()) {
                auto monster = entity_manager.createMonster(species, point.x, point.y);
                if (monster && game_manager && game_manager->getMonsterAI()) {
                    // Assign room to monster
                    Room* room = map.getRoomAt(point.x, point.y);
                    if (room) {
                        Monster* monster_ptr = dynamic_cast<Monster*>(monster.get());
                        if (monster_ptr) {
                            game_manager->getMonsterAI()->assignRoomToMonster(*monster_ptr, room);
                        }
                    }
                    monsters_spawned++;
                }
            }
        }
    }
}

void SpawnManager::update(Map& map, EntityManager& entity_manager, 
                         const Player* player, int depth) {
    turns_since_spawn++;
    
    // Check if it's time to spawn
    if (turns_since_spawn < spawn_rate) return;
    
    // Check if we're at monster limit
    auto monsters = entity_manager.getMonsters();
    if (monsters.size() >= static_cast<size_t>(max_monsters)) return;
    
    // Decide whether to spawn in room or corridor
    std::uniform_real_distribution<float> room_roll(0.0f, 1.0f);
    bool spawn_in_room = room_roll(rng) < room_spawn_percentage;
    
    // Get appropriate spawn points
    std::vector<Point> spawn_points;
    if (spawn_in_room) {
        spawn_points = getRoomSpawnPoints(map, player);
        // Fall back to corridors if no room points available
        if (spawn_points.empty()) {
            spawn_points = getCorridorSpawnPoints(map, player);
        }
    } else {
        spawn_points = getCorridorSpawnPoints(map, player);
        // Fall back to rooms if no corridor points available
        if (spawn_points.empty()) {
            spawn_points = getRoomSpawnPoints(map, player);
        }
    }
    
    if (spawn_points.empty()) return;
    
    // Pick a random spawn point
    std::uniform_int_distribution<size_t> dist(0, spawn_points.size() - 1);
    const Point& spawn_point = spawn_points[dist(rng)];
    
    // Select and spawn a monster
    std::string species = selectMonsterSpecies(depth, rng);
    if (!species.empty()) {
        entity_manager.createMonster(species, spawn_point.x, spawn_point.y);
        turns_since_spawn = 0;
    }
}

std::vector<Point> SpawnManager::getValidSpawnPoints(const Map& map, const Player* player) const {
    std::vector<Point> valid_points;
    
    for (int y = 1; y < map.getHeight() - 1; y++) {
        for (int x = 1; x < map.getWidth() - 1; x++) {
            if (isValidSpawnPoint(map, player, x, y)) {
                valid_points.push_back({x, y});
            }
        }
    }
    
    return valid_points;
}

std::vector<Point> SpawnManager::getRoomSpawnPoints(const Map& map, const Player* player) const {
    std::vector<Point> room_points;
    
    for (int y = 1; y < map.getHeight() - 1; y++) {
        for (int x = 1; x < map.getWidth() - 1; x++) {
            if (isValidSpawnPoint(map, player, x, y)) {
                // Check if this point is in a room
                const Room* room = map.getRoomAt(Point(x, y));
                if (room != nullptr) {
                    room_points.push_back({x, y});
                }
            }
        }
    }
    
    return room_points;
}

std::vector<Point> SpawnManager::getCorridorSpawnPoints(const Map& map, const Player* player) const {
    std::vector<Point> corridor_points;
    
    for (int y = 1; y < map.getHeight() - 1; y++) {
        for (int x = 1; x < map.getWidth() - 1; x++) {
            if (isValidSpawnPoint(map, player, x, y)) {
                // Check if this point is NOT in a room (i.e., in a corridor)
                const Room* room = map.getRoomAt(Point(x, y));
                if (room == nullptr) {
                    corridor_points.push_back({x, y});
                }
            }
        }
    }
    
    return corridor_points;
}

bool SpawnManager::isValidSpawnPoint(const Map& map, const Player* player, int x, int y) const {
    // Must be walkable
    if (!map.isWalkable(x, y)) return false;
    
    // Must not be a special tile (stairs, etc.)
    TileType tile = map.getTile(x, y);
    if (tile == TileType::STAIRS_DOWN || tile == TileType::STAIRS_UP) return false;
    
    if (player) {
        // Check minimum distance from player
        int dist = distance(x, y, player->x, player->y);
        if (dist < min_spawn_distance) return false;
        
        // Check if in player's FOV (if spawn_outside_fov is true)
        if (spawn_outside_fov) {
            // Simple FOV check - just use distance for now
            // TODO: Use actual FOV calculation
            int fov_radius = Config::getInstance().getFOVRadius();
            if (dist <= fov_radius) return false;
        }
    }
    
    return true;
}

std::string SpawnManager::selectMonsterSpecies(int depth, std::mt19937& random_gen) const {
    // Build list of valid monsters for this depth
    std::vector<std::pair<std::string, float>> candidates;
    
    for (const auto& entry : spawn_table) {
        if (depth >= entry.min_depth && depth <= entry.max_depth) {
            candidates.push_back({entry.species, entry.weight});
        }
    }
    
    if (candidates.empty()) return "";
    
    // Calculate total weight
    float total_weight = 0.0f;
    for (const auto& [species, weight] : candidates) {
        total_weight += weight;
    }
    
    // Weighted random selection
    std::uniform_real_distribution<float> dist(0.0f, total_weight);
    float roll = dist(random_gen);
    
    float current_weight = 0.0f;
    for (const auto& [species, weight] : candidates) {
        current_weight += weight;
        if (roll <= current_weight) {
            return species;
        }
    }
    
    // Fallback (shouldn't happen)
    return candidates.front().first;
}

int SpawnManager::getCurrentThreatLevel(const EntityManager& entity_manager) const {
    int total_threat = 0;
    
    auto monsters = entity_manager.getMonsters();
    for (const auto& entity : monsters) {
        // Cast to Monster to access species
        auto monster = dynamic_cast<Monster*>(entity.get());
        if (monster) {
            // Get threat value based on species
            for (const auto& entry : spawn_table) {
                if (entry.species == monster->species) {
                    total_threat += entry.threat_value;
                    break;
                }
            }
        }
    }
    
    return total_threat;
}

int SpawnManager::distance(int x1, int y1, int x2, int y2) const {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return static_cast<int>(std::sqrt(dx * dx + dy * dy));
}