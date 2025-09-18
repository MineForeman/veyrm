/**
 * @file game_entity_repository.h
 * @brief Repository for storing and retrieving ECS entities from PostgreSQL
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <boost/json.hpp>

// Forward declarations
namespace ecs {
    class Entity;
    class World;
    class GameWorld;
}

namespace db {
    class DatabaseManager;
    class Connection;
}

namespace db {

/**
 * @struct GameEntityData
 * @brief Data structure representing an entity in the database
 */
struct GameEntityData {
    int64_t id;                         ///< Entity ID
    int user_id;                        ///< User who owns this entity
    int save_slot;                      ///< Save slot number (-1 for auto-save)
    std::string entity_type;            ///< Type: 'player', 'monster', 'item'
    int x;                              ///< X position
    int y;                              ///< Y position
    int map_level;                      ///< Map level/depth
    bool is_active;                     ///< Whether entity is active
    std::string definition_id;          ///< References monster_definitions.id or item_definitions.id
    std::string definition_type;        ///< Type of definition: 'monster', 'item', 'player'
    boost::json::object component_data; ///< Dynamic component state only (HP, buffs, etc.)
    std::vector<std::string> entity_tags; ///< Entity tags
};

/**
 * @struct GameSaveData
 * @brief Data structure representing a game save in the database
 */
struct GameSaveData {
    std::string id;                     ///< Save UUID
    int user_id;                        ///< User ID
    int save_slot;                      ///< Save slot number
    std::string character_name;         ///< Character name
    int character_level;                ///< Character level
    int map_level;                      ///< Current map level
    int play_time_seconds;              ///< Total play time
    std::string game_version;           ///< Game version
    std::string save_version;           ///< Save format version
    std::string device_id;              ///< Device identifier
    std::string device_name;            ///< Device name
    int map_width;                      ///< Map width
    int map_height;                     ///< Map height
    int64_t world_seed;                 ///< World generation seed
};

/**
 * @class GameEntityRepository
 * @brief Repository for managing ECS entities in PostgreSQL
 */
class GameEntityRepository {
public:
    /**
     * @brief Constructor
     */
    GameEntityRepository();

    /**
     * @brief Destructor
     */
    ~GameEntityRepository() = default;

    /**
     * @brief Save a complete game state (save metadata + all entities)
     * @param save_data Game save metadata
     * @param entities Vector of entity data to save
     * @return true if successful
     */
    bool saveGameState(const GameSaveData& save_data, const std::vector<GameEntityData>& entities);

    /**
     * @brief Load a complete game state (save metadata + all entities)
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return Optional pair of save data and entities, empty if not found
     */
    std::optional<std::pair<GameSaveData, std::vector<GameEntityData>>>
    loadGameState(int user_id, int save_slot);

    /**
     * @brief Save individual entity to database
     * @param entity_data Entity data to save
     * @return true if successful
     */
    bool saveEntity(const GameEntityData& entity_data);

    /**
     * @brief Load entities for a specific save
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return Vector of entity data
     */
    std::vector<GameEntityData> loadEntities(int user_id, int save_slot);

    /**
     * @brief Delete all entities for a save slot
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return true if successful
     */
    bool deleteEntities(int user_id, int save_slot);

    /**
     * @brief Save game metadata
     * @param save_data Save metadata to store
     * @return true if successful
     */
    bool saveMeta(const GameSaveData& save_data);

    /**
     * @brief Load game metadata
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return Optional save metadata, empty if not found
     */
    std::optional<GameSaveData> loadMeta(int user_id, int save_slot);

    /**
     * @brief Get list of available saves for a user
     * @param user_id User ID
     * @return Vector of save metadata
     */
    std::vector<GameSaveData> listSaves(int user_id);

    /**
     * @brief Delete a complete save (metadata + entities)
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return true if successful
     */
    bool deleteSave(int user_id, int save_slot);

    /**
     * @brief Convert ECS entity to database format
     * @param entity ECS entity to convert
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return Entity data for database storage
     */
    static GameEntityData entityToData(const ecs::Entity& entity, int user_id, int save_slot);

    /**
     * @brief Convert database entity data to ECS entity
     * @param data Database entity data
     * @param world ECS world to create entity in
     * @return Created ECS entity, or nullptr if failed
     */
    static std::unique_ptr<ecs::Entity> dataToEntity(const GameEntityData& data, ecs::World& world);

    /**
     * @brief Serialize ECS world to database entities
     * @param world ECS world to serialize
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return Vector of entity data for storage
     */
    static std::vector<GameEntityData> serializeWorld(const ecs::World& world, int user_id, int save_slot);

    /**
     * @brief Deserialize database entities to ECS world
     * @param entities Vector of entity data from database
     * @param game_world GameWorld to populate (with player tracking)
     * @return Number of entities successfully restored
     */
    static int deserializeWorld(const std::vector<GameEntityData>& entities, ecs::GameWorld& game_world);

private:
    /**
     * @brief Convert component data to JSON
     * @param entity ECS entity
     * @return JSON object containing component data
     */
    static boost::json::object serializeComponents(const ecs::Entity& entity);

    /**
     * @brief Restore components from JSON to entity
     * @param entity ECS entity to add components to
     * @param component_data JSON component data
     */
    static void deserializeComponents(ecs::Entity& entity, const boost::json::object& component_data);

private:
    DatabaseManager& db_manager;  ///< Database manager reference

    /**
     * @brief Save game metadata using existing connection
     * @param conn Database connection
     * @param save_data Save metadata to store
     * @return true if successful
     */
    bool saveMetaWithConn(Connection& conn, const GameSaveData& save_data);

    /**
     * @brief Delete entities using existing connection
     * @param conn Database connection
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return true if successful
     */
    bool deleteEntitiesWithConn(Connection& conn, int user_id, int save_slot);

    /**
     * @brief Save entity using existing connection
     * @param conn Database connection
     * @param entity_data Entity data to save
     * @return true if successful
     */
    bool saveEntityWithConn(Connection& conn, const GameEntityData& entity_data);

    /**
     * @brief Load game metadata using existing connection
     * @param conn Database connection
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return Optional save metadata, empty if not found
     */
    std::optional<GameSaveData> loadMetaWithConn(Connection& conn, int user_id, int save_slot);

    /**
     * @brief Load entities using existing connection
     * @param conn Database connection
     * @param user_id User ID
     * @param save_slot Save slot number
     * @return Vector of entity data
     */
    std::vector<GameEntityData> loadEntitiesWithConn(Connection& conn, int user_id, int save_slot);
};

} // namespace db