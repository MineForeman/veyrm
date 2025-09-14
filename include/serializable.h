/**
 * @file serializable.h
 * @brief Serialization interface and save game data structures
 * @author Veyrm Team
 * @date 2025
 */

#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @class ISerializable
 * @brief Interface for objects that can be serialized to/from JSON
 *
 * The ISerializable interface defines the contract for objects that need
 * to be saved to and loaded from JSON format. This is used by the save/load
 * system to persist game state across sessions.
 *
 * Classes implementing this interface must provide:
 * - serialize(): Convert object to JSON representation
 * - deserialize(): Restore object from JSON data
 *
 * @see GameSerializer
 * @see SaveInfo
 */
class ISerializable {
public:
    /** @brief Virtual destructor for proper cleanup */
    virtual ~ISerializable() = default;

    /**
     * @brief Serialize object to JSON format
     * @return JSON representation of the object
     * @note Must include all data needed to restore object state
     */
    virtual json serialize() const = 0;

    /**
     * @brief Deserialize object from JSON format
     * @param data JSON data to deserialize from
     * @return true on successful deserialization, false on failure
     * @note Object state should remain unchanged on failure
     */
    virtual bool deserialize(const json& data) = 0;
};

/**
 * @struct SaveInfo
 * @brief Metadata about a save game file
 *
 * Contains summary information about a save file, used for displaying
 * save game listings and validating save file compatibility. This data
 * is typically stored in the save file header for quick access without
 * loading the entire game state.
 *
 * @see GameSerializer::loadSaveInfo()
 * @see SaveLoadScreen
 */
struct SaveInfo {
    std::string filename;       ///< Save file name
    std::string timestamp;      ///< Creation/modification time
    std::string player_name;    ///< Character name
    int depth = 1;              ///< Current dungeon depth
    int level = 1;              ///< Player level (future feature)
    int turn_count = 0;         ///< Number of game turns elapsed
    int play_time = 0;          ///< Total play time in seconds
    int player_hp = 0;          ///< Current player hit points
    int player_max_hp = 0;      ///< Maximum player hit points
    bool exists = false;        ///< True if save file exists on disk
    bool is_valid = false;      ///< True if save file is valid/loadable
    std::string version;        ///< Save format version
    std::string game_version;   ///< Game version that created the save

    /**
     * @brief Convert save info to JSON format
     * @return JSON object with save information
     * @note Used for UI display and metadata storage
     */
    json toJson() const {
        return json{
            {"filename", filename},
            {"timestamp", timestamp},
            {"player_name", player_name},
            {"level", level},
            {"turn_count", turn_count},
            {"play_time", play_time},
            {"is_valid", is_valid},
            {"version", version},
            {"game_version", game_version}
        };
    }

    /**
     * @brief Create SaveInfo from JSON data
     * @param data JSON object containing save information
     * @return SaveInfo struct populated from JSON
     * @note Safely handles missing fields with default values
     */
    static SaveInfo fromJson(const json& data) {
        SaveInfo info;
        if (data.contains("filename")) info.filename = data["filename"];
        if (data.contains("timestamp")) info.timestamp = data["timestamp"];
        if (data.contains("metadata")) {
            auto& meta = data["metadata"];
            if (meta.contains("player_name")) info.player_name = meta["player_name"];
            if (meta.contains("level")) info.level = meta["level"];
            if (meta.contains("turn_count")) info.turn_count = meta["turn_count"];
            if (meta.contains("play_time")) info.play_time = meta["play_time"];
        }
        if (data.contains("version")) info.version = data["version"];
        if (data.contains("game_version")) info.game_version = data["game_version"];
        info.is_valid = true;
        return info;
    }
};