#pragma once

#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <functional>
#include <chrono>
#include <vector>

#ifdef ENABLE_DATABASE
#include <libpq-fe.h>
#endif

namespace db {

struct DatabaseConfig {
    std::string host = "localhost";
    int port = 5432;
    std::string database = "veyrm_db";
    std::string username = "veyrm_admin";
    std::string password = "";

    size_t min_connections = 2;
    size_t max_connections = 10;
    std::chrono::milliseconds connection_timeout{5000};

    // Get connection string for libpq
    std::string getConnectionString() const {
        return "host=" + host +
               " port=" + std::to_string(port) +
               " dbname=" + database +
               " user=" + username +
               " password=" + password;
    }
};

#ifdef ENABLE_DATABASE

// RAII wrapper for PGresult
class Result {
private:
    PGresult* res;

public:
    explicit Result(PGresult* r) : res(r) {}
    ~Result() { if (res) PQclear(res); }

    Result(Result&& other) noexcept : res(other.res) {
        other.res = nullptr;
    }

    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;

    PGresult* get() { return res; }
    const PGresult* get() const { return res; }

    bool isOk() const {
        if (!res) return false;
        ExecStatusType status = PQresultStatus(res);
        return status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK;
    }

    std::string getError() const {
        if (!res) return "No result";
        return PQresultErrorMessage(res);
    }

    int numRows() const { return res ? PQntuples(res) : 0; }
    int numCols() const { return res ? PQnfields(res) : 0; }

    std::string getValue(int row, int col) const {
        if (!res || PQgetisnull(res, row, col)) return "";
        return PQgetvalue(res, row, col);
    }

    bool isNull(int row, int col) const {
        return !res || PQgetisnull(res, row, col);
    }
};

// RAII wrapper for PGconn
class Connection {
private:
    PGconn* conn;
    std::chrono::steady_clock::time_point last_used;

public:
    Connection() : conn(nullptr) {}

    explicit Connection(const std::string& connStr)
        : conn(PQconnectdb(connStr.c_str())),
          last_used(std::chrono::steady_clock::now()) {
        if (PQstatus(conn) != CONNECTION_OK) {
            std::string error = PQerrorMessage(conn);
            PQfinish(conn);
            conn = nullptr;
            throw std::runtime_error("Connection failed: " + error);
        }
    }

    ~Connection() {
        if (conn) PQfinish(conn);
    }

    Connection(Connection&& other) noexcept
        : conn(other.conn), last_used(other.last_used) {
        other.conn = nullptr;
    }

    Connection& operator=(Connection&& other) noexcept {
        if (this != &other) {
            if (conn) PQfinish(conn);
            conn = other.conn;
            last_used = other.last_used;
            other.conn = nullptr;
        }
        return *this;
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    bool isValid() const {
        return conn && PQstatus(conn) == CONNECTION_OK;
    }

    void updateLastUsed() {
        last_used = std::chrono::steady_clock::now();
    }

    auto getLastUsed() const { return last_used; }

    // Execute a query
    Result exec(const std::string& query) {
        if (!isValid()) throw std::runtime_error("Invalid connection");
        updateLastUsed();
        return Result(PQexec(conn, query.c_str()));
    }

    // Execute with parameters
    Result execParams(const std::string& query,
                     const std::vector<std::string>& params) {
        if (!isValid()) throw std::runtime_error("Invalid connection");
        updateLastUsed();

        std::vector<const char*> paramValues;
        for (const auto& p : params) {
            paramValues.push_back(p.c_str());
        }

        return Result(PQexecParams(conn, query.c_str(),
                                  static_cast<int>(params.size()),
                                  nullptr,  // param types
                                  paramValues.data(),
                                  nullptr,  // param lengths
                                  nullptr,  // param formats
                                  0));      // result format (text)
    }

    // Escape string for safe inclusion in queries
    std::string escapeString(const std::string& str) {
        if (!isValid()) throw std::runtime_error("Invalid connection");

        std::vector<char> buffer(str.size() * 2 + 1);
        PQescapeStringConn(conn, buffer.data(), str.c_str(), str.size(), nullptr);
        return std::string(buffer.data());
    }

    // Begin transaction
    bool beginTransaction() {
        auto res = exec("BEGIN");
        return res.isOk();
    }

    // Commit transaction
    bool commit() {
        auto res = exec("COMMIT");
        return res.isOk();
    }

    // Rollback transaction
    bool rollback() {
        auto res = exec("ROLLBACK");
        return res.isOk();
    }
};

class ConnectionPool {
private:
    DatabaseConfig config;
    std::vector<std::unique_ptr<Connection>> connections;
    std::queue<Connection*> available;
    std::mutex mutex;
    std::condition_variable cv;
    bool shutdown = false;

    void createConnection();
    void validateConnection(Connection* conn);
    void cleanupStale();

public:
    explicit ConnectionPool(const DatabaseConfig& cfg);
    ~ConnectionPool();

    class PooledConnection {
    private:
        ConnectionPool* pool;
        Connection* conn;

    public:
        PooledConnection(ConnectionPool* p, Connection* c)
            : pool(p), conn(c) {}

        ~PooledConnection();

        PooledConnection(PooledConnection&& other) noexcept
            : pool(other.pool), conn(other.conn) {
            other.conn = nullptr;
        }

        PooledConnection(const PooledConnection&) = delete;
        PooledConnection& operator=(const PooledConnection&) = delete;

        Connection* operator->() { return conn; }
        Connection& operator*() { return *conn; }
        Connection* get() { return conn; }
    };

    std::optional<PooledConnection> acquire(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
    void release(Connection* conn);
    void initialize();
    void stop();
};

class DatabaseManager {
private:
    static std::unique_ptr<DatabaseManager> instance;
    DatabaseConfig config;
    std::unique_ptr<ConnectionPool> pool;
    bool initialized = false;

    DatabaseManager() = default;

public:
    static DatabaseManager& getInstance();

    void initialize(const DatabaseConfig& cfg);
    void shutdown();
    bool isInitialized() const { return initialized; }

    // Get a connection from the pool
    std::optional<ConnectionPool::PooledConnection> getConnection();

    // Execute a transaction with automatic retry
    template<typename F>
    auto executeTransaction(F&& func) {
        auto conn_opt = getConnection();
        if (!conn_opt) {
            throw std::runtime_error("Failed to get database connection");
        }

        auto& conn = *conn_opt;
        conn->beginTransaction();

        try {
            auto result = func(*conn);
            conn->commit();
            return result;
        } catch (...) {
            conn->rollback();
            throw;
        }
    }

    // Execute a read-only query
    template<typename F>
    auto executeQuery(F&& func) {
        auto conn_opt = getConnection();
        if (!conn_opt) {
            throw std::runtime_error("Failed to get database connection");
        }

        auto& conn = *conn_opt;
        return func(*conn);
    }

    // Database migrations
    void runMigrations();
    int getCurrentSchemaVersion();

    // Data management
    bool createTables();
    bool clearAllData();
    bool loadInitialData();
    bool isDataLoaded();
    void ensureDataLoaded();

    // Utility functions
    bool testConnection();
    std::string getDatabaseVersion();
};

// Database exceptions
class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& message)
        : std::runtime_error("Database error: " + message) {}
};

class ConnectionException : public DatabaseException {
public:
    explicit ConnectionException(const std::string& message)
        : DatabaseException("Connection failed: " + message) {}
};

class QueryException : public DatabaseException {
public:
    QueryException(const std::string& query, const std::string& error)
        : DatabaseException("Query failed: " + error + "\nQuery: " + query) {}
};

#else // !ENABLE_DATABASE

// Stub implementation when database is disabled
class DatabaseManager {
public:
    static DatabaseManager& getInstance() {
        static DatabaseManager instance;
        return instance;
    }

    void initialize(const DatabaseConfig&) {}
    void shutdown() {}
    bool isInitialized() const { return false; }
    void runMigrations() {}
    int getCurrentSchemaVersion() { return 0; }
    bool testConnection() { return false; }
    std::string getDatabaseVersion() { return "Database disabled"; }
};

#endif // ENABLE_DATABASE

} // namespace db