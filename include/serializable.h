#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Interface for objects that can be serialized to/from JSON
class ISerializable {
public:
    virtual ~ISerializable() = default;

    // Serialize the object to JSON
    virtual json serialize() const = 0;

    // Deserialize the object from JSON
    // Returns true on success, false on failure
    virtual bool deserialize(const json& data) = 0;
};

// Save file metadata
struct SaveInfo {
    std::string filename;
    std::string timestamp;
    std::string player_name;
    int depth = 1;  // Current dungeon depth
    int level = 1;  // Player level (if implemented)
    int turn_count = 0;
    int play_time = 0;  // in seconds
    int player_hp = 0;
    int player_max_hp = 0;
    bool exists = false;  // Whether the save file exists
    bool is_valid = false;
    std::string version;
    std::string game_version;

    // Convert to JSON for display
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

    // Create from JSON
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