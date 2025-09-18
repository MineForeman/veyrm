/**
 * @file save_game_repository.cpp
 * @brief Implementation of cloud save game repository
 * @author Veyrm Team
 * @date 2025
 */

#include "db/save_game_repository.h"
#include "db/database_manager.h"
#include "log.h"
#include <map>
#include <sstream>
#include <iomanip>

namespace db {

SaveGameRepository::SaveGameRepository(DatabaseManager& db_manager)
    : db_manager(db_manager) {
}

// === CRUD Operations ===

std::optional<SaveGame> SaveGameRepository::create(const SaveGame& save) {
    try {
        return db_manager.executeTransaction([this, &save](Connection& conn) -> std::optional<SaveGame> {
            // First check if slot already exists for user
            if (save.slot_number != 0) {  // 0 means auto-assign
                std::string check_sql =
                    "SELECT id FROM save_games WHERE user_id = $1 AND slot_number = $2";

                auto check_result = conn.execParams(check_sql, {std::to_string(save.user_id), std::to_string(save.slot_number)});
                if (check_result.numRows() > 0) {
                    // Slot exists, update instead
                    SaveGame existing_save = save;
                    PGresult* check_res = check_result.get();
                    int id_col = PQfnumber(check_res, "id");
                    existing_save.id = check_result.getValue(0, id_col);
                    if (update(existing_save)) {
                        return existing_save;
                    }
                    return std::nullopt;
                }
            }

            std::string sql = R"(
                INSERT INTO save_games (
                    user_id, slot_number, character_name, character_level,
                    map_depth, play_time, turn_count, save_data,
                    save_version, game_version, device_id, device_name,
                    sync_status
                ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
                RETURNING id, created_at, updated_at, last_played_at
            )";

            auto result = conn.execParams(sql, {
                std::to_string(save.user_id),
                std::to_string(save.slot_number),
                save.character_name,
                std::to_string(save.character_level),
                std::to_string(save.map_depth),
                std::to_string(save.play_time),
                std::to_string(save.turn_count),
                jsonToString(save.save_data),
                save.save_version,
                save.game_version,
                save.device_id,
                save.device_name,
                save.sync_status
            });

            if (result.numRows() > 0) {
                SaveGame created = save;
                PGresult* res = result.get();
                int id_col = PQfnumber(res, "id");
                int created_at_col = PQfnumber(res, "created_at");
                int updated_at_col = PQfnumber(res, "updated_at");
                int last_played_at_col = PQfnumber(res, "last_played_at");
                created.id = result.getValue(0, id_col);
                created.created_at = stringToTimestamp(result.getValue(0, created_at_col));
                created.updated_at = stringToTimestamp(result.getValue(0, updated_at_col));
                created.last_played_at = stringToTimestamp(result.getValue(0, last_played_at_col));

                Log::info("Created save game: " + created.id + " for user " + std::to_string(save.user_id));
                return created;
            }

            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to create save game: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<SaveGame> SaveGameRepository::findById(const std::string& id) {
    try {
        return db_manager.executeQuery([this, &id](Connection& conn) -> std::optional<SaveGame> {
            std::string sql = R"(
                SELECT * FROM save_games WHERE id = $1
            )";

            auto result = conn.execParams(sql, {id});
            if (result.numRows() > 0) {
                return rowToSaveGame(result, 0);
            }

            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to find save by ID: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<SaveGame> SaveGameRepository::findByUserId(int user_id) {
    try {
        return db_manager.executeQuery([this, user_id](Connection& conn) -> std::vector<SaveGame> {
            std::string sql = R"(
                SELECT * FROM save_games
                WHERE user_id = $1
                ORDER BY slot_number, updated_at DESC
            )";

            auto result = conn.execParams(sql, {std::to_string(user_id)});
            std::vector<SaveGame> saves;

            for (int i = 0; i < result.numRows(); ++i) {
                saves.push_back(rowToSaveGame(result, i));
            }

            return saves;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to find saves by user ID: " + std::string(e.what()));
        return {};
    }
}

std::optional<SaveGame> SaveGameRepository::findByUserAndSlot(int user_id, int slot) {
    try {
        return db_manager.executeQuery([this, user_id, slot](Connection& conn) -> std::optional<SaveGame> {
            std::string sql = R"(
                SELECT * FROM save_games
                WHERE user_id = $1 AND slot_number = $2
            )";

            auto result = conn.execParams(sql, {std::to_string(user_id), std::to_string(slot)});
            if (result.numRows() > 0) {
                return rowToSaveGame(result, 0);
            }

            return std::nullopt;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to find save by user and slot: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool SaveGameRepository::update(const SaveGame& save) {
    try {
        return db_manager.executeTransaction([this, &save](Connection& conn) -> bool {
            // Create backup before update
            createBackup(save.id, "pre_update");

            std::string sql = R"(
                UPDATE save_games SET
                    character_name = $1,
                    character_level = $2,
                    map_depth = $3,
                    play_time = $4,
                    turn_count = $5,
                    save_data = $6,
                    save_version = $7,
                    game_version = $8,
                    updated_at = CURRENT_TIMESTAMP,
                    last_played_at = CURRENT_TIMESTAMP,
                    device_id = $9,
                    device_name = $10,
                    sync_status = $11
                WHERE id = $12
            )";

            auto result = conn.execParams(sql, {
                save.character_name,
                std::to_string(save.character_level),
                std::to_string(save.map_depth),
                std::to_string(save.play_time),
                std::to_string(save.turn_count),
                jsonToString(save.save_data),
                save.save_version,
                save.game_version,
                save.device_id,
                save.device_name,
                save.sync_status,
                save.id
            });

            PGresult* res = result.get();
            const char* affected = PQcmdTuples(res);
            if (affected && std::atoi(affected) > 0) {
                Log::info("Updated save game: " + save.id);
                return true;
            }

            return false;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to update save game: " + std::string(e.what()));
        return false;
    }
}

bool SaveGameRepository::deleteById(const std::string& id) {
    try {
        return db_manager.executeTransaction([this, &id](Connection& conn) -> bool {
            // Create backup before deletion
            createBackup(id, "pre_delete");

            std::string sql = "DELETE FROM save_games WHERE id = $1";
            auto result = conn.execParams(sql, {id});

            PGresult* res = result.get();
            const char* affected = PQcmdTuples(res);
            if (affected && std::atoi(affected) > 0) {
                Log::info("Deleted save game: " + id);
                return true;
            }

            return false;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to delete save game: " + std::string(e.what()));
        return false;
    }
}

bool SaveGameRepository::deleteByUserAndSlot(int user_id, int slot) {
    try {
        return db_manager.executeTransaction([this, user_id, slot](Connection& conn) -> bool {
            // Get save ID for backup
            std::string get_id_sql =
                "SELECT id FROM save_games WHERE user_id = $1 AND slot_number = $2";
            auto id_result = conn.execParams(get_id_sql, {std::to_string(user_id), std::to_string(slot)});

            if (id_result.numRows() > 0) {
                PGresult* id_res = id_result.get();
                int id_col = PQfnumber(id_res, "id");
                std::string save_id = id_result.getValue(0, id_col);
                createBackup(save_id, "pre_delete");
            }

            std::string sql = "DELETE FROM save_games WHERE user_id = $1 AND slot_number = $2";
            auto result = conn.execParams(sql, {std::to_string(user_id), std::to_string(slot)});

            PGresult* res = result.get();
            const char* affected = PQcmdTuples(res);
            if (affected && std::atoi(affected) > 0) {
                Log::info("Deleted save for user " + std::to_string(user_id) + " slot " + std::to_string(slot));
                return true;
            }

            return false;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to delete save by user and slot: " + std::string(e.what()));
        return false;
    }
}

// === Sync Operations ===

std::vector<SaveGame> SaveGameRepository::getUnsyncedSaves(int user_id) {
    try {
        return db_manager.executeQuery([this, user_id](Connection& conn) -> std::vector<SaveGame> {
            std::string sql = R"(
                SELECT * FROM save_games
                WHERE user_id = $1 AND sync_status != 'synced'
                ORDER BY updated_at DESC
            )";

            auto result = conn.execParams(sql, {std::to_string(user_id)});
            std::vector<SaveGame> saves;

            for (int i = 0; i < result.numRows(); ++i) {
                saves.push_back(rowToSaveGame(result, i));
            }

            return saves;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to get unsynced saves: " + std::string(e.what()));
        return {};
    }
}

bool SaveGameRepository::markAsSynced(const std::string& save_id) {
    try {
        return db_manager.executeQuery([&save_id](Connection& conn) -> bool {
            std::string sql = R"(
                UPDATE save_games
                SET sync_status = 'synced', updated_at = CURRENT_TIMESTAMP
                WHERE id = $1
            )";

            auto result = conn.execParams(sql, {save_id});
            PGresult* res = result.get();
            const char* affected = PQcmdTuples(res);
            return affected && std::atoi(affected) > 0;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to mark save as synced: " + std::string(e.what()));
        return false;
    }
}

bool SaveGameRepository::markAsConflicted(const std::string& save_id) {
    try {
        return db_manager.executeQuery([&save_id](Connection& conn) -> bool {
            std::string sql = R"(
                UPDATE save_games
                SET sync_status = 'conflict', updated_at = CURRENT_TIMESTAMP
                WHERE id = $1
            )";

            auto result = conn.execParams(sql, {save_id});
            PGresult* res = result.get();
            const char* affected = PQcmdTuples(res);
            return affected && std::atoi(affected) > 0;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to mark save as conflicted: " + std::string(e.what()));
        return false;
    }
}

// === Conflict Resolution ===

bool SaveGameRepository::createConflict(const SaveConflict& conflict) {
    try {
        return db_manager.executeTransaction([this, &conflict](Connection& conn) -> bool {
            std::string sql = R"(
                INSERT INTO save_conflicts (
                    save_id, conflicting_data, device_id, device_name
                ) VALUES ($1, $2, $3, $4)
            )";

            auto result = conn.execParams(sql, {
                conflict.save_id,
                jsonToString(conflict.conflicting_data),
                conflict.device_id,
                conflict.device_name
            });

            if (result.isOk()) {
                markAsConflicted(conflict.save_id);
                Log::info("Created conflict for save: " + conflict.save_id);
                return true;
            }

            return false;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to create conflict: " + std::string(e.what()));
        return false;
    }
}

std::vector<SaveConflict> SaveGameRepository::getUnresolvedConflicts(int user_id) {
    try {
        return db_manager.executeQuery([this, user_id](Connection& conn) -> std::vector<SaveConflict> {
            std::string sql = R"(
                SELECT c.* FROM save_conflicts c
                JOIN save_games s ON s.id = c.save_id
                WHERE s.user_id = $1 AND c.resolved = FALSE
                ORDER BY c.created_at DESC
            )";

            auto result = conn.execParams(sql, {std::to_string(user_id)});
            std::vector<SaveConflict> conflicts;

            for (int i = 0; i < result.numRows(); ++i) {
                conflicts.push_back(rowToConflict(result, i));
            }

            return conflicts;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to get unresolved conflicts: " + std::string(e.what()));
        return {};
    }
}

bool SaveGameRepository::resolveConflict(const std::string& conflict_id, ResolutionType type) {
    try {
        return db_manager.executeTransaction([this, &conflict_id, type](Connection& conn) -> bool {
            // Mark conflict as resolved
            std::string resolution_str;
            switch (type) {
                case ResolutionType::LOCAL_WINS: resolution_str = "local_wins"; break;
                case ResolutionType::CLOUD_WINS: resolution_str = "cloud_wins"; break;
                case ResolutionType::MERGE: resolution_str = "merge"; break;
                case ResolutionType::BACKUP_BOTH: resolution_str = "backup_both"; break;
            }

            std::string sql = R"(
                UPDATE save_conflicts
                SET resolved = TRUE, resolution_type = $1
                WHERE id = $2
            )";

            auto result = conn.execParams(sql, {resolution_str, conflict_id});

            PGresult* res = result.get();
            const char* affected = PQcmdTuples(res);
            if (affected && std::atoi(affected) > 0) {
                // Get save_id to update sync status
                std::string get_save_sql = "SELECT save_id FROM save_conflicts WHERE id = $1";
                auto save_result = conn.execParams(get_save_sql, {conflict_id});

                if (save_result.numRows() > 0) {
                    PGresult* save_res = save_result.get();
                    int save_id_col = PQfnumber(save_res, "save_id");
                    std::string save_id = save_result.getValue(0, save_id_col);
                    markAsSynced(save_id);
                }

                Log::info("Resolved conflict: " + conflict_id + " with " + resolution_str);
                return true;
            }

            return false;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to resolve conflict: " + std::string(e.what()));
        return false;
    }
}

// === Backup Operations ===

bool SaveGameRepository::createBackup(const std::string& save_id, const std::string& reason) {
    try {
        return db_manager.executeTransaction([this, &save_id, &reason](Connection& conn) -> bool {
            // Get current save data
            std::string get_sql = "SELECT save_data FROM save_games WHERE id = $1";
            auto get_result = conn.execParams(get_sql, {save_id});

            if (get_result.numRows() == 0) {
                return false;
            }

            PGresult* get_res = get_result.get();
            int save_data_col = PQfnumber(get_res, "save_data");
            std::string save_data = get_result.getValue(0, save_data_col);

            // Create backup
            std::string sql = R"(
                INSERT INTO save_backups (save_id, backup_data, backup_reason)
                VALUES ($1, $2, $3)
            )";

            auto result = conn.execParams(sql, {save_id, save_data, reason});

            if (result.isOk()) {
                // Prune old backups
                pruneOldBackups(save_id, 5);
                Log::debug("Created backup for save: " + save_id + " (" + reason + ")");
                return true;
            }

            return false;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to create backup: " + std::string(e.what()));
        return false;
    }
}

std::vector<SaveBackup> SaveGameRepository::getBackups(const std::string& save_id, int limit) {
    try {
        return db_manager.executeQuery([this, &save_id, limit](Connection& conn) -> std::vector<SaveBackup> {
            std::string sql = R"(
                SELECT * FROM save_backups
                WHERE save_id = $1
                ORDER BY created_at DESC
                LIMIT $2
            )";

            auto result = conn.execParams(sql, {save_id, std::to_string(limit)});
            std::vector<SaveBackup> backups;

            for (int i = 0; i < result.numRows(); ++i) {
                backups.push_back(rowToBackup(result, i));
            }

            return backups;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to get backups: " + std::string(e.what()));
        return {};
    }
}

int SaveGameRepository::pruneOldBackups(const std::string& save_id, int keep_count) {
    try {
        return db_manager.executeTransaction([&save_id, keep_count](Connection& conn) -> int {
            // Delete all but the most recent N backups
            std::string sql = R"(
                DELETE FROM save_backups
                WHERE save_id = $1 AND id NOT IN (
                    SELECT id FROM save_backups
                    WHERE save_id = $1
                    ORDER BY created_at DESC
                    LIMIT $2
                )
            )";

            auto result = conn.execParams(sql, {save_id, std::to_string(keep_count)});
            PGresult* res = result.get();
            const char* affected = PQcmdTuples(res);
            int deleted = affected ? std::atoi(affected) : 0;

            if (deleted > 0) {
                Log::debug("Pruned " + std::to_string(deleted) + " old backups for save: " + save_id);
            }

            return deleted;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to prune backups: " + std::string(e.what()));
        return 0;
    }
}

// === Statistics ===

std::map<std::string, int> SaveGameRepository::getSaveStatistics(int user_id) {
    try {
        return db_manager.executeQuery([user_id](Connection& conn) -> std::map<std::string, int> {
            std::map<std::string, int> stats;

            // Total saves
            std::string total_sql = "SELECT COUNT(*) FROM save_games WHERE user_id = $1";
            auto total_result = conn.execParams(total_sql, {std::to_string(user_id)});
            stats["total_saves"] = std::atoi(total_result.getValue(0, 0).c_str());

            // Manual saves
            std::string manual_sql = "SELECT COUNT(*) FROM save_games WHERE user_id = $1 AND slot_number > 0";
            auto manual_result = conn.execParams(manual_sql, {std::to_string(user_id)});
            stats["manual_saves"] = std::atoi(manual_result.getValue(0, 0).c_str());

            // Auto saves
            std::string auto_sql = "SELECT COUNT(*) FROM save_games WHERE user_id = $1 AND slot_number < 0";
            auto auto_result = conn.execParams(auto_sql, {std::to_string(user_id)});
            stats["auto_saves"] = std::atoi(auto_result.getValue(0, 0).c_str());

            // Unsynced saves
            std::string unsynced_sql = "SELECT COUNT(*) FROM save_games WHERE user_id = $1 AND sync_status != 'synced'";
            auto unsynced_result = conn.execParams(unsynced_sql, {std::to_string(user_id)});
            stats["unsynced_saves"] = std::atoi(unsynced_result.getValue(0, 0).c_str());

            // Conflicts
            std::string conflict_sql = R"(
                SELECT COUNT(*) FROM save_conflicts c
                JOIN save_games s ON s.id = c.save_id
                WHERE s.user_id = $1 AND c.resolved = FALSE
            )";
            auto conflict_result = conn.execParams(conflict_sql, {std::to_string(user_id)});
            stats["unresolved_conflicts"] = std::atoi(conflict_result.getValue(0, 0).c_str());

            return stats;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to get save statistics: " + std::string(e.what()));
        return {};
    }
}

size_t SaveGameRepository::getTotalSaveSize(int user_id) {
    try {
        return db_manager.executeQuery([user_id](Connection& conn) -> size_t {
            std::string sql = R"(
                SELECT SUM(LENGTH(save_data::text)) as total_size
                FROM save_games
                WHERE user_id = $1
            )";

            auto result = conn.execParams(sql, {std::to_string(user_id)});
            if (result.numRows() > 0 && !result.isNull(0, 0)) {
                PGresult* res = result.get();
                int total_size_col = PQfnumber(res, "total_size");
                return static_cast<size_t>(std::stoll(result.getValue(0, total_size_col)));
            }

            return 0;
        });
    } catch (const std::exception& e) {
        Log::error("Failed to get total save size: " + std::string(e.what()));
        return 0;
    }
}

// === Helper Methods ===

SaveGame SaveGameRepository::rowToSaveGame(const Result& row, int index) const {
    SaveGame save;
    const PGresult* res = row.get();
    int id_col = PQfnumber(res, "id");
    int user_id_col = PQfnumber(res, "user_id");
    int slot_number_col = PQfnumber(res, "slot_number");
    int character_name_col = PQfnumber(res, "character_name");
    int character_level_col = PQfnumber(res, "character_level");
    int map_depth_col = PQfnumber(res, "map_depth");
    int play_time_col = PQfnumber(res, "play_time");
    int turn_count_col = PQfnumber(res, "turn_count");
    int save_data_col = PQfnumber(res, "save_data");
    int save_version_col = PQfnumber(res, "save_version");
    int game_version_col = PQfnumber(res, "game_version");
    int created_at_col = PQfnumber(res, "created_at");
    int updated_at_col = PQfnumber(res, "updated_at");
    int last_played_at_col = PQfnumber(res, "last_played_at");
    int device_id_col = PQfnumber(res, "device_id");
    int device_name_col = PQfnumber(res, "device_name");
    int sync_status_col = PQfnumber(res, "sync_status");

    save.id = row.getValue(index, id_col);
    save.user_id = std::atoi(row.getValue(index, user_id_col).c_str());
    save.slot_number = std::atoi(row.getValue(index, slot_number_col).c_str());
    save.character_name = row.getValue(index, character_name_col);
    save.character_level = std::atoi(row.getValue(index, character_level_col).c_str());
    save.map_depth = std::atoi(row.getValue(index, map_depth_col).c_str());
    save.play_time = std::atoi(row.getValue(index, play_time_col).c_str());
    save.turn_count = std::atoi(row.getValue(index, turn_count_col).c_str());
    save.save_data = stringToJson(row.getValue(index, save_data_col));
    save.save_version = row.getValue(index, save_version_col);
    save.game_version = row.getValue(index, game_version_col);
    save.created_at = stringToTimestamp(row.getValue(index, created_at_col));
    save.updated_at = stringToTimestamp(row.getValue(index, updated_at_col));
    save.last_played_at = stringToTimestamp(row.getValue(index, last_played_at_col));
    save.device_id = row.getValue(index, device_id_col);
    save.device_name = row.getValue(index, device_name_col);
    save.sync_status = row.getValue(index, sync_status_col);
    return save;
}

SaveConflict SaveGameRepository::rowToConflict(const Result& row, int index) const {
    SaveConflict conflict;
    const PGresult* res = row.get();
    int id_col = PQfnumber(res, "id");
    int save_id_col = PQfnumber(res, "save_id");
    int conflicting_data_col = PQfnumber(res, "conflicting_data");
    int device_id_col = PQfnumber(res, "device_id");
    int device_name_col = PQfnumber(res, "device_name");
    int created_at_col = PQfnumber(res, "created_at");
    int resolved_col = PQfnumber(res, "resolved");
    int resolution_type_col = PQfnumber(res, "resolution_type");

    conflict.id = row.getValue(index, id_col);
    conflict.save_id = row.getValue(index, save_id_col);
    conflict.conflicting_data = stringToJson(row.getValue(index, conflicting_data_col));
    conflict.device_id = row.getValue(index, device_id_col);
    conflict.device_name = row.getValue(index, device_name_col);
    conflict.created_at = stringToTimestamp(row.getValue(index, created_at_col));
    std::string resolved_str = row.getValue(index, resolved_col);
    conflict.resolved = (resolved_str == "t" || resolved_str == "true" || resolved_str == "1");
    conflict.resolution_type = row.getValue(index, resolution_type_col);
    return conflict;
}

SaveBackup SaveGameRepository::rowToBackup(const Result& row, int index) const {
    SaveBackup backup;
    const PGresult* res = row.get();
    int id_col = PQfnumber(res, "id");
    int save_id_col = PQfnumber(res, "save_id");
    int backup_data_col = PQfnumber(res, "backup_data");
    int backup_reason_col = PQfnumber(res, "backup_reason");
    int created_at_col = PQfnumber(res, "created_at");

    backup.id = row.getValue(index, id_col);
    backup.save_id = row.getValue(index, save_id_col);
    backup.backup_data = stringToJson(row.getValue(index, backup_data_col));
    backup.backup_reason = row.getValue(index, backup_reason_col);
    backup.created_at = stringToTimestamp(row.getValue(index, created_at_col));
    return backup;
}

std::string SaveGameRepository::jsonToString(const boost::json::value& json) const {
    return boost::json::serialize(json);
}

boost::json::value SaveGameRepository::stringToJson(const std::string& str) const {
    try {
        return boost::json::parse(str);
    } catch (const std::exception& e) {
        Log::error("Failed to parse JSON: " + std::string(e.what()));
        return boost::json::object();
    }
}

std::string SaveGameRepository::timestampToString(const std::chrono::system_clock::time_point& tp) const {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::chrono::system_clock::time_point SaveGameRepository::stringToTimestamp(const std::string& str) const {
    std::tm tm = {};
    std::stringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

} // namespace db