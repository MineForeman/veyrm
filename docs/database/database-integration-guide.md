# Database Integration Guide

## Overview

This guide provides comprehensive documentation for Veyrm's PostgreSQL integration, including setup, usage, troubleshooting, and development guidelines.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Architecture Overview](#architecture-overview)
3. [Setup and Configuration](#setup-and-configuration)
4. [Development Usage](#development-usage)
5. [Testing](#testing)
6. [Troubleshooting](#troubleshooting)
7. [Performance Optimization](#performance-optimization)
8. [Security Considerations](#security-considerations)
9. [Migration and Maintenance](#migration-and-maintenance)

## Quick Start

### 1. Environment Setup

```bash
# Copy environment template
cp .env.example .env

# Edit .env file with your settings
vim .env
```

Required environment variables:
```bash
# PostgreSQL Configuration
POSTGRES_DB=veyrm_db
POSTGRES_USER=veyrm_admin
POSTGRES_PASSWORD=your_secure_password_here
POSTGRES_PORT=5432

# Optional: PgAdmin (for development)
PGADMIN_EMAIL=admin@veyrm.local
PGADMIN_PASSWORD=admin_password
PGADMIN_PORT=5050
```

### 2. Start Database

```bash
# Start PostgreSQL only
docker-compose up -d postgres

# Or start with PgAdmin for development
docker-compose --profile tools up -d
```

### 3. Initialize Schema

```bash
# Create all tables and indexes
./build.sh db create

# Load initial game data (monsters, items, colors, etc.)
./build.sh db load

# Verify setup
./build.sh db status
```

### 4. Run Tests

```bash
# Run all tests (148 tests should pass)
./build.sh test

# Run only database tests
./build/bin/veyrm_tests "[database]"
```

## Architecture Overview

### Database Layer Design

```
┌─────────────────────────────────────────┐
│               Application               │
├─────────────────────────────────────────┤
│            Service Layer                │
│  ┌───────────────┐ ┌─────────────────┐  │
│  │ Auth Service  │ │ Cloud Save Svc  │  │
│  └───────────────┘ └─────────────────┘  │
├─────────────────────────────────────────┤
│           Repository Layer              │
│  ┌───────────────┐ ┌─────────────────┐  │
│  │ Player Repo   │ │ SaveGame Repo   │  │
│  └───────────────┘ └─────────────────┘  │
├─────────────────────────────────────────┤
│          Database Manager               │
│        (Connection Pooling)             │
├─────────────────────────────────────────┤
│            PostgreSQL                   │
│         (Tables & Indexes)              │
└─────────────────────────────────────────┘
```

### Key Design Patterns

1. **Repository Pattern**: Clean separation between business logic and data access
2. **Connection Pooling**: Efficient database resource management
3. **Dependency Injection**: Loose coupling between components
4. **Factory Pattern**: Centralized object creation
5. **Singleton Pattern**: Single database manager instance

## Setup and Configuration

### Docker Setup (Recommended)

#### 1. Docker Compose Configuration

The project includes a complete Docker setup:

```yaml
# docker-compose.yml excerpt
services:
  postgres:
    image: postgres:16-alpine
    container_name: veyrm-postgres
    environment:
      POSTGRES_DB: ${POSTGRES_DB}
      POSTGRES_USER: ${POSTGRES_USER}
      POSTGRES_PASSWORD: ${POSTGRES_PASSWORD}
    ports:
      - "${POSTGRES_PORT:-5432}:5432"
    volumes:
      - postgres_data:/var/lib/postgresql/data
      - ./init:/docker-entrypoint-initdb.d:ro
      - ./backups:/backups
```

#### 2. Available Commands

```bash
# Database lifecycle
docker-compose up -d postgres      # Start database
docker-compose down                # Stop database
docker-compose logs -f postgres    # View logs

# Data management
./build.sh db create               # Create schema
./build.sh db load                 # Load game data
./build.sh db status               # Check connection
./build.sh db reset                # Reset database

# Development tools
docker-compose --profile tools up -d  # Start with PgAdmin
# Access PgAdmin at http://localhost:5050
```

### Manual PostgreSQL Setup

If you prefer a local PostgreSQL installation:

#### 1. Install PostgreSQL 16

```bash
# macOS (Homebrew)
brew install postgresql@16

# Ubuntu/Debian
sudo apt-get install postgresql-16

# CentOS/RHEL
sudo yum install postgresql16-server
```

#### 2. Configure Database

```bash
# Create database and user
sudo -u postgres psql
```

```sql
CREATE DATABASE veyrm_db;
CREATE USER veyrm_admin WITH PASSWORD 'your_password';
GRANT ALL PRIVILEGES ON DATABASE veyrm_db TO veyrm_admin;
```

#### 3. Update Connection Settings

```bash
# In .env file
POSTGRES_HOST=localhost
POSTGRES_PORT=5432
POSTGRES_DB=veyrm_db
POSTGRES_USER=veyrm_admin
POSTGRES_PASSWORD=your_password
```

## Development Usage

### Database Manager

The `DatabaseManager` class provides the core database interface:

```cpp
#include "db/database_manager.h"

// Get singleton instance
auto& db_manager = db::DatabaseManager::getInstance();

// Initialize with configuration
db::DatabaseConfig config;
config.host = "localhost";
config.port = 5432;
config.database = "veyrm_db";
config.username = "veyrm_admin";
config.password = "password";
config.min_connections = 2;
config.max_connections = 10;

bool success = db_manager.initialize(config);
```

### Repository Usage

#### Save Game Repository

```cpp
#include "db/save_game_repository.h"

// Create repository
auto& db_manager = db::DatabaseManager::getInstance();
db::SaveGameRepository save_repo(db_manager);

// Create a save
db::SaveGame save;
save.user_id = 123;
save.slot_number = 1;
save.character_name = "Hero";
save.character_level = 15;
save.save_data = game_state_json;

auto result = save_repo.create(save);
if (result.has_value()) {
    std::cout << "Save created with ID: " << result->id << std::endl;
}

// Load a save
auto loaded = save_repo.findByUserAndSlot(123, 1);
if (loaded.has_value()) {
    // Restore game state from loaded->save_data
}

// List user saves
auto saves = save_repo.findByUserId(123);
for (const auto& save : saves) {
    std::cout << "Slot " << save.slot_number
              << ": " << save.character_name << std::endl;
}
```

#### Player Repository

```cpp
#include "db/player_repository.h"

auto& db_manager = db::DatabaseManager::getInstance();
db::PlayerRepository player_repo(db_manager);

// Create user
db::User user;
user.username = "player123";
user.email = "player@example.com";
user.password_hash = "hashed_password";
user.salt = "random_salt";

auto user_result = player_repo.create(user);

// Find user by username
auto found_user = player_repo.findByUsername("player123");

// Create session
db::UserSession session;
session.user_id = user_result->id;
session.session_token = "random_session_token";
session.expires_at = std::chrono::system_clock::now() + std::chrono::hours(4);

auto session_id = player_repo.createSession(session);
```

### Authentication Service

```cpp
#include "auth/authentication_service.h"

auto& db_manager = db::DatabaseManager::getInstance();
db::PlayerRepository player_repo(db_manager);
auth::AuthenticationService auth_service(player_repo, db_manager);

// Register user
auto reg_result = auth_service.registerUser(
    "username",
    "email@example.com",
    "password123"
);

if (reg_result.success) {
    std::cout << "User registered with ID: "
              << reg_result.user_id.value() << std::endl;
}

// Login user
auto login_result = auth_service.login("username", "password123");
if (login_result.success) {
    std::cout << "Login successful. Session: "
              << login_result.session_token.value() << std::endl;
}

// Validate session
auto validation = auth_service.validateSession(session_token);
if (validation.valid) {
    std::cout << "Valid session for user: "
              << validation.user_id.value() << std::endl;
}
```

### Cloud Save Service

```cpp
#include "services/cloud_save_service.h"

// Initialize service with dependencies
CloudSaveService cloud_service(&save_repo, &auth_service, &ecs_world);

// The service automatically handles:
// - Save synchronization
// - Conflict detection
// - Backup creation
// - Error recovery
```

## Testing

### Test Categories

The project includes comprehensive database testing:

#### 1. Database Integration Tests

```bash
# Run all database tests
./build/bin/veyrm_tests "[database]"

# Specific test categories
./build/bin/veyrm_tests "[database][auth]"     # Authentication
./build/bin/veyrm_tests "[database][save]"     # Save/load operations
./build/bin/veyrm_tests "[database][cloud]"    # Cloud save features
```

#### 2. Performance Tests

```bash
# Performance and stress tests
./build/bin/veyrm_tests "[performance]"
```

#### 3. Test Structure

Tests use the Catch2 framework with proper setup/teardown:

```cpp
#include <catch2/catch_test_macros.hpp>
#include "db/save_game_repository.h"

class DatabaseTest {
public:
    DatabaseTest() {
        // Initialize test database
        auto& db_manager = db::DatabaseManager::getInstance();
        // ... setup
    }

    ~DatabaseTest() {
        // Clean up test data
    }
};

TEST_CASE_METHOD(DatabaseTest, "Save game operations", "[database][save]") {
    SECTION("Create and load save") {
        // Test implementation
    }
}
```

### Test Database

Tests use the same database as development but with:
- Unique usernames/emails (timestamp-based)
- Automatic cleanup in destructors
- Isolated test data to prevent conflicts

## Troubleshooting

### Common Issues

#### 1. Connection Refused

**Symptoms:**
```
ERROR: Failed to connect to database: Connection refused
```

**Solutions:**
```bash
# Check if PostgreSQL is running
docker ps | grep postgres

# Check logs
docker-compose logs postgres

# Restart database
docker-compose down && docker-compose up -d postgres

# Verify connection settings in .env
```

#### 2. Authentication Failed

**Symptoms:**
```
ERROR: FATAL: password authentication failed for user "veyrm_admin"
```

**Solutions:**
```bash
# Check password in .env file
cat .env | grep POSTGRES_PASSWORD

# Reset password
docker-compose down -v  # WARNING: This deletes data
docker-compose up -d postgres
```

#### 3. Schema/Table Errors

**Symptoms:**
```
ERROR: relation "users" does not exist
```

**Solutions:**
```bash
# Recreate schema
./build.sh db reset

# Check table creation
./build.sh db status
```

#### 4. Test Failures

**Symptoms:**
```
Test failed: Email already registered
```

**Solutions:**
- Tests use timestamp-based unique identifiers
- Clean up code should handle conflicts
- Check if database is accessible during tests

### Debugging Tools

#### 1. Database Status

```bash
# Check connection and table counts
./build.sh db status

# Expected output:
# Database connection: OK
# Tables found: 36
# Users: 5
# Save games: 12
```

#### 2. Manual Database Inspection

```bash
# Connect to database
docker exec -it veyrm-postgres psql -U veyrm_admin -d veyrm_db

# Common queries
\dt                          # List tables
SELECT COUNT(*) FROM users;  # Count users
SELECT * FROM save_games LIMIT 5;  # Recent saves
```

#### 3. Log Analysis

```bash
# Application logs
tail -f logs/veyrm_debug.log

# Database logs
docker-compose logs -f postgres
```

## Performance Optimization

### Connection Pooling

The `DatabaseManager` uses connection pooling for optimal performance:

```cpp
// Configuration
db::DatabaseConfig config;
config.min_connections = 2;    // Always keep 2 connections
config.max_connections = 10;   // Max 10 connections
config.connection_timeout = std::chrono::milliseconds(5000);
```

### Optimized Queries

#### 1. Prepared Statements

All repository methods use prepared statements:

```cpp
// Repository implementation
std::string query = R"(
    SELECT id, user_id, slot_number, character_name, save_data
    FROM save_games
    WHERE user_id = $1 AND slot_number = $2
)";

const char* params[] = { user_id_str.c_str(), slot_str.c_str() };
auto result = conn.execParams(query, 2, params);
```

#### 2. Indexes

Key indexes for performance:

```sql
-- Save games
CREATE INDEX idx_saves_user ON save_games(user_id);
CREATE INDEX idx_saves_slot ON save_games(user_id, slot_number);
CREATE INDEX idx_saves_updated ON save_games(updated_at);

-- Sessions
CREATE INDEX idx_sessions_token ON user_sessions(session_token);
CREATE INDEX idx_sessions_user ON user_sessions(user_id);
```

#### 3. JSONB Performance

Use GIN indexes for JSONB queries:

```sql
-- For save_data queries
CREATE INDEX idx_save_data_gin ON save_games USING GIN (save_data);

-- Query optimization
SELECT * FROM save_games
WHERE save_data @> '{"player": {"level": 10}}';
```

### Performance Monitoring

#### 1. Query Performance

```sql
-- Monitor slow queries
SELECT query, mean_time, calls
FROM pg_stat_statements
WHERE mean_time > 100
ORDER BY mean_time DESC;
```

#### 2. Connection Usage

```sql
-- Monitor active connections
SELECT count(*) as active_connections
FROM pg_stat_activity
WHERE state = 'active';
```

## Security Considerations

### Password Security

#### 1. Hashing Implementation

```cpp
// Authentication service uses SHA256 + salt
std::string hashPassword(const std::string& password, const std::string& salt) {
    // Implementation uses cryptographically secure hashing
    return sha256(password + salt);
}

// Generate random salt
std::string generateSalt() {
    // Generate cryptographically secure random salt
}
```

#### 2. Password Requirements

- Minimum 8 characters
- Must contain uppercase letter
- Must contain lowercase letter
- Must contain number
- Special characters recommended

### SQL Injection Prevention

All queries use prepared statements:

```cpp
// SAFE: Uses prepared statement
std::string query = "SELECT * FROM users WHERE username = $1";
const char* params[] = { username.c_str() };
auto result = conn.execParams(query, 1, params);

// NEVER DO: Direct string concatenation
// std::string query = "SELECT * FROM users WHERE username = '" + username + "'";
```

### Session Security

#### 1. Token Generation

```cpp
// Generate cryptographically secure session tokens
std::string generateToken(size_t length = 32) {
    // Uses secure random number generator
}
```

#### 2. Session Management

- Short-lived session tokens (4 hours)
- Long-lived refresh tokens (30 days)
- Automatic session cleanup
- IP address tracking
- User agent validation

### Database Access Control

#### 1. User Privileges

```sql
-- Application user has limited privileges
GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES TO veyrm_admin;
GRANT USAGE ON ALL SEQUENCES TO veyrm_admin;

-- No DDL privileges in production
REVOKE CREATE ON SCHEMA public FROM veyrm_admin;
```

#### 2. Connection Security

- Use SSL/TLS for production connections
- Firewall rules to restrict database access
- VPN access for development

## Migration and Maintenance

### Schema Migrations

#### 1. Migration System

```sql
-- Track schema versions
CREATE TABLE schema_migrations (
    version VARCHAR(20) PRIMARY KEY,
    description TEXT,
    applied_at TIMESTAMP DEFAULT NOW()
);
```

#### 2. Migration Process

```bash
# Apply new migration
./build.sh db migrate

# Check migration status
./build.sh db migrate status

# Rollback migration (if supported)
./build.sh db migrate rollback
```

### Backup and Restore

#### 1. Automated Backups

```bash
# Create backup
./build.sh db backup

# Scheduled backup (cron)
0 2 * * * /path/to/veyrm/build.sh db backup
```

#### 2. Backup Storage

```bash
# Backup location
./backups/
├── daily/
│   ├── backup_20250118_020000.sql
│   └── backup_20250117_020000.sql
├── weekly/
│   └── backup_week_03_2025.sql
└── monthly/
    └── backup_2025_01.sql
```

#### 3. Restore Process

```bash
# Restore from backup
./build.sh db restore ./backups/backup_20250118_020000.sql

# Verify restore
./build.sh db status
./build.sh test "[database]"
```

### Database Maintenance

#### 1. Regular Tasks

```bash
# Vacuum and analyze (weekly)
docker exec veyrm-postgres psql -U veyrm_admin -d veyrm_db -c "VACUUM ANALYZE;"

# Reindex (monthly)
docker exec veyrm-postgres psql -U veyrm_admin -d veyrm_db -c "REINDEX DATABASE veyrm_db;"

# Update statistics
docker exec veyrm-postgres psql -U veyrm_admin -d veyrm_db -c "ANALYZE;"
```

#### 2. Monitoring

```bash
# Database size
./build.sh db size

# Table sizes
./build.sh db tables

# Index usage
./build.sh db indexes
```

### Data Cleanup

#### 1. Session Cleanup

```sql
-- Remove expired sessions (automated)
DELETE FROM user_sessions
WHERE expires_at < NOW() - INTERVAL '1 day';

-- Remove old login history (monthly)
DELETE FROM user_login_history
WHERE created_at < NOW() - INTERVAL '90 days';
```

#### 2. Save Game Cleanup

```sql
-- Remove old save backups (configurable)
DELETE FROM save_backups
WHERE created_at < NOW() - INTERVAL '30 days';

-- Archive old telemetry data
INSERT INTO telemetry_archive
SELECT * FROM telemetry
WHERE created_at < NOW() - INTERVAL '1 year';

DELETE FROM telemetry
WHERE created_at < NOW() - INTERVAL '1 year';
```

## Environment Configuration

### Development Environment

```bash
# .env.development
POSTGRES_HOST=localhost
POSTGRES_PORT=5432
POSTGRES_DB=veyrm_db_dev
POSTGRES_USER=veyrm_admin
POSTGRES_PASSWORD=dev_password

# Enable development features
VEYRM_ENV=development
VEYRM_DEBUG=true
VEYRM_LOG_LEVEL=debug
```

### Production Environment

```bash
# .env.production
POSTGRES_HOST=prod-db-server
POSTGRES_PORT=5432
POSTGRES_DB=veyrm_db
POSTGRES_USER=veyrm_app
POSTGRES_PASSWORD=secure_production_password

# Production security
VEYRM_ENV=production
VEYRM_DEBUG=false
VEYRM_LOG_LEVEL=warning
VEYRM_SSL_MODE=require
```

### Testing Environment

```bash
# .env.test
POSTGRES_HOST=localhost
POSTGRES_PORT=5433  # Different port for test DB
POSTGRES_DB=veyrm_db_test
POSTGRES_USER=veyrm_test
POSTGRES_PASSWORD=test_password

# Test-specific settings
VEYRM_ENV=test
VEYRM_FAST_TESTS=true
VEYRM_CLEANUP_TEST_DATA=true
```

## Conclusion

This guide covers the complete PostgreSQL integration for Veyrm. The system provides:

- **Robust Authentication**: Secure user management with session handling
- **Cloud Saves**: Multi-slot save synchronization with conflict resolution
- **High Performance**: Connection pooling and optimized queries
- **Comprehensive Testing**: 148+ test cases covering all scenarios
- **Production Ready**: Security best practices and monitoring

For additional help:
- Check the [Database ERD](database-erd.md) for schema details
- See [PostgreSQL Setup](postgres-setup.md) for initial configuration
- Review test files in `tests/test_database_*.cpp` for usage examples
- Check logs in `logs/` directory for debugging information

The database integration is designed to be robust, secure, and scalable, supporting both single-player and future multiplayer features.