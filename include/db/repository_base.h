#pragma once

#include "db/database_manager.h"
#include <string>
#include <optional>
#include <vector>
#include <random>
#include <boost/json.hpp>

namespace db {

// Base repository class for common database operations
template<typename T>
class RepositoryBase {
protected:
    DatabaseManager& db;

    explicit RepositoryBase()
        : db(DatabaseManager::getInstance()) {}

public:
    virtual ~RepositoryBase() = default;

    // CRUD operations
    virtual std::optional<T> findById(const std::string& id) = 0;
    virtual std::vector<T> findAll() = 0;
    virtual std::string create(const T& entity) = 0;
    virtual bool update(const T& entity) = 0;
    virtual bool remove(const std::string& id) = 0;

    // Utility functions
    bool exists(const std::string& id) {
        return findById(id).has_value();
    }

    size_t count() {
        return findAll().size();
    }

protected:
    // Generate UUID for new entities
    std::string generateUUID() {
        // Simple UUID v4 generation
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);

        const char* hex = "0123456789abcdef";
        std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";

        for (size_t i = 0; i < uuid.length(); ++i) {
            if (uuid[i] == 'x') {
                uuid[i] = hex[dis(gen)];
            } else if (uuid[i] == 'y') {
                uuid[i] = hex[(dis(gen) & 0x3) | 0x8];
            }
        }

        return uuid;
    }

    // Helper to check if table exists
    bool tableExists(const std::string& tableName) {
        return db.executeQuery([&tableName](Connection& conn) {
            auto result = conn.execParams(
                "SELECT EXISTS ("
                "  SELECT 1 FROM information_schema.tables "
                "  WHERE table_schema = 'public' "
                "  AND table_name = $1"
                ")",
                {tableName}
            );
            return result.isOk() && result.numRows() > 0 &&
                   result.getValue(0, 0) == "t";
        });
    }

    // Helper to convert JSON string to object
    template<typename JsonType>
    JsonType jsonToObject(const std::string& json) {
        return boost::json::value::parse(json).get<JsonType>();
    }

    // Helper to convert object to JSON string
    template<typename JsonType>
    std::string objectToJson(const JsonType& obj) {
        boost::json::value j = obj;
        return j.dump();
    }
};

} // namespace db