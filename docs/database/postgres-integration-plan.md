# PostgreSQL Integration Plan

## Executive Summary

This document outlines the comprehensive plan for integrating PostgreSQL into Veyrm to enable persistent data storage, cloud saves, leaderboards, and telemetry features. The integration will be implemented in phases to ensure minimal disruption to existing gameplay while adding powerful new capabilities.

## Goals and Objectives

### Primary Goals
1. **Persistent Player Profiles** - Store player accounts and preferences
2. **Cloud Save System** - Enable cross-device gameplay continuation
3. **Global Leaderboards** - Track and display high scores and achievements
4. **Telemetry Collection** - Gather gameplay metrics for balancing and improvements

### Secondary Goals
- Daily/Weekly challenges with leaderboards
- Player statistics and progression tracking
- Death replay system
- Social features (future)

## Architecture Overview

### System Architecture

```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│  Game Client│────▶│  Connection  │────▶│  PostgreSQL │
│   (C++)     │     │     Pool     │     │   Database  │
└─────────────┘     └──────────────┘     └─────────────┘
       │                    │                     │
       ▼                    ▼                     ▼
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│   DB Layer  │     │    Cache     │     │   Backup    │
│  Interface  │     │   (Redis)    │     │   System    │
└─────────────┘     └──────────────┘     └─────────────┘
```

### Component Layers

1. **Database Access Layer** (`include/db/`)
   - Connection management
   - Query execution
   - Transaction handling
   - Error recovery

2. **Repository Layer** (`include/repositories/`)
   - Player repository
   - Save game repository
   - Leaderboard repository
   - Telemetry repository

3. **Service Layer** (`include/services/`)
   - Authentication service
   - Save/Load service
   - Leaderboard service
   - Analytics service

## Database Schema Design

### Core Tables

#### 1. Players Table
```sql
CREATE TABLE players (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    username VARCHAR(32) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP,
    total_playtime INTEGER DEFAULT 0,
    preferences JSONB,
    is_active BOOLEAN DEFAULT true
);

CREATE INDEX idx_players_username ON players(username);
CREATE INDEX idx_players_email ON players(email);
```

#### 2. Characters Table
```sql
CREATE TABLE characters (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID REFERENCES players(id) ON DELETE CASCADE,
    name VARCHAR(32) NOT NULL,
    level INTEGER DEFAULT 1,
    experience INTEGER DEFAULT 0,
    total_gold_collected INTEGER DEFAULT 0,
    deepest_level INTEGER DEFAULT 1,
    play_time INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    died_at TIMESTAMP,
    death_reason VARCHAR(255),
    is_active BOOLEAN DEFAULT true
);

CREATE INDEX idx_characters_player ON characters(player_id);
CREATE INDEX idx_characters_active ON characters(is_active);
```

#### 3. Save Games Table
```sql
CREATE TABLE save_games (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    character_id UUID REFERENCES characters(id) ON DELETE CASCADE,
    slot_number INTEGER NOT NULL,
    save_name VARCHAR(100),
    game_version VARCHAR(20) NOT NULL,
    map_seed BIGINT NOT NULL,
    current_depth INTEGER DEFAULT 1,
    turn_count INTEGER DEFAULT 0,
    game_time INTEGER DEFAULT 0,
    game_state JSONB NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(character_id, slot_number)
);

CREATE INDEX idx_saves_character ON save_games(character_id);
CREATE INDEX idx_saves_updated ON save_games(updated_at);
```

#### 4. Leaderboards Table
```sql
CREATE TABLE leaderboards (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID REFERENCES players(id),
    character_id UUID REFERENCES characters(id),
    score INTEGER NOT NULL,
    depth_reached INTEGER NOT NULL,
    monsters_killed INTEGER DEFAULT 0,
    gold_collected INTEGER DEFAULT 0,
    play_time INTEGER NOT NULL,
    death_reason VARCHAR(255),
    game_version VARCHAR(20),
    submitted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    leaderboard_type VARCHAR(20) DEFAULT 'all_time'
);

CREATE INDEX idx_leaderboard_score ON leaderboards(score DESC);
CREATE INDEX idx_leaderboard_type ON leaderboards(leaderboard_type);
CREATE INDEX idx_leaderboard_date ON leaderboards(submitted_at);
```

#### 5. Achievements Table
```sql
CREATE TABLE achievements (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    points INTEGER DEFAULT 10,
    icon VARCHAR(50),
    category VARCHAR(50),
    hidden BOOLEAN DEFAULT false
);

CREATE TABLE player_achievements (
    player_id UUID REFERENCES players(id) ON DELETE CASCADE,
    achievement_id UUID REFERENCES achievements(id),
    unlocked_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    character_id UUID REFERENCES characters(id),
    PRIMARY KEY (player_id, achievement_id)
);
```

#### 6. Telemetry Table
```sql
CREATE TABLE telemetry (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID REFERENCES players(id),
    character_id UUID REFERENCES characters(id),
    event_type VARCHAR(50) NOT NULL,
    event_data JSONB NOT NULL,
    game_version VARCHAR(20),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_telemetry_player ON telemetry(player_id);
CREATE INDEX idx_telemetry_type ON telemetry(event_type);
CREATE INDEX idx_telemetry_date ON telemetry(created_at);

-- Partition by month for performance
CREATE TABLE telemetry_y2025m01 PARTITION OF telemetry
    FOR VALUES FROM ('2025-01-01') TO ('2025-02-01');
```

### Supporting Tables

#### Session Management
```sql
CREATE TABLE sessions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    player_id UUID REFERENCES players(id) ON DELETE CASCADE,
    token VARCHAR(255) UNIQUE NOT NULL,
    ip_address INET,
    user_agent TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP NOT NULL
);

CREATE INDEX idx_sessions_token ON sessions(token);
CREATE INDEX idx_sessions_expires ON sessions(expires_at);
```

## C++ Integration

### Dependencies

1. **libpqxx** - Official PostgreSQL C++ client library
2. **Connection Pool** - r3c or custom implementation
3. **JSON Library** - nlohmann/json for JSONB handling
4. **UUID Library** - stduuid or boost::uuid

### CMake Configuration

```cmake
# Find PostgreSQL
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

# Add libpqxx
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.7.5
)
FetchContent_MakeAvailable(libpqxx)

# Link libraries
target_link_libraries(veyrm PRIVATE
    pqxx
    PostgreSQL::PostgreSQL
    Threads::Threads
)
```

### Core Classes

#### 1. Database Connection Manager
```cpp
// include/db/database_manager.h
class DatabaseManager {
private:
    std::unique_ptr<ConnectionPool> pool;
    Config dbConfig;

public:
    static DatabaseManager& getInstance();

    std::shared_ptr<pqxx::connection> getConnection();
    void releaseConnection(std::shared_ptr<pqxx::connection> conn);

    void initialize(const Config& config);
    void shutdown();

    // Transaction helpers
    template<typename F>
    auto executeTransaction(F&& func) -> decltype(func(std::declval<pqxx::work&>()));

    // Migration support
    void runMigrations();
    int getCurrentSchemaVersion();
};
```

#### 2. Player Repository
```cpp
// include/repositories/player_repository.h
class PlayerRepository {
private:
    DatabaseManager& db;

public:
    struct Player {
        std::string id;
        std::string username;
        std::string email;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point last_login;
        int total_playtime;
        nlohmann::json preferences;
    };

    std::optional<Player> findById(const std::string& id);
    std::optional<Player> findByUsername(const std::string& username);
    std::optional<Player> authenticate(const std::string& username,
                                       const std::string& password);

    std::string create(const Player& player, const std::string& password);
    void update(const Player& player);
    void updateLastLogin(const std::string& id);

    std::vector<Player> getTopPlayers(int limit = 10);
};
```

#### 3. Save Game Repository
```cpp
// include/repositories/save_game_repository.h
class SaveGameRepository {
private:
    DatabaseManager& db;

public:
    struct SaveGame {
        std::string id;
        std::string character_id;
        int slot_number;
        std::string save_name;
        int64_t map_seed;
        int current_depth;
        int turn_count;
        nlohmann::json game_state;
        std::chrono::system_clock::time_point updated_at;
    };

    std::vector<SaveGame> getCharacterSaves(const std::string& character_id);
    std::optional<SaveGame> getSaveBySlot(const std::string& character_id,
                                          int slot);

    void save(const SaveGame& save);
    void deleteSave(const std::string& save_id);

    // Cloud save sync
    std::vector<SaveGame> getRecentSaves(const std::string& player_id,
                                         int limit = 10);
};
```

#### 4. Leaderboard Service
```cpp
// include/services/leaderboard_service.h
class LeaderboardService {
private:
    LeaderboardRepository& repo;
    std::shared_ptr<Cache> cache;

public:
    enum class LeaderboardType {
        ALL_TIME,
        MONTHLY,
        WEEKLY,
        DAILY
    };

    struct Entry {
        int rank;
        std::string player_name;
        int score;
        int depth;
        int play_time;
        std::string death_reason;
    };

    std::vector<Entry> getLeaderboard(LeaderboardType type,
                                      int offset = 0,
                                      int limit = 100);

    std::optional<Entry> getPlayerRank(const std::string& player_id,
                                       LeaderboardType type);

    void submitScore(const std::string& character_id);

    // Cached operations
    std::vector<Entry> getTodaysTop10();
    void refreshCache();
};
```

### Error Handling

```cpp
// include/db/database_exceptions.h
class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& message)
        : std::runtime_error(message) {}
};

class ConnectionException : public DatabaseException {
public:
    explicit ConnectionException(const std::string& message)
        : DatabaseException("Connection error: " + message) {}
};

class QueryException : public DatabaseException {
public:
    QueryException(const std::string& query, const std::string& error)
        : DatabaseException("Query failed: " + error + "\nQuery: " + query) {}
};
```

## Implementation Phases

### Phase 1: Foundation (Week 1-2)
- [ ] Set up CMake integration with libpqxx
- [ ] Implement DatabaseManager with connection pooling
- [ ] Create base Repository and Service classes
- [ ] Set up database migrations system
- [ ] Create initial schema migrations
- [ ] Write unit tests for database layer

### Phase 2: Authentication (Week 3)
- [ ] Implement PlayerRepository
- [ ] Create authentication service
- [ ] Add password hashing (bcrypt/argon2)
- [ ] Implement session management
- [ ] Create login/registration UI
- [ ] Add "Remember Me" functionality

### Phase 3: Save System Integration (Week 4)
- [ ] Implement SaveGameRepository
- [ ] Modify existing save system to use database
- [ ] Add cloud save sync logic
- [ ] Create save conflict resolution
- [ ] Implement auto-save to cloud
- [ ] Add offline mode fallback

### Phase 4: Leaderboards (Week 5)
- [ ] Implement LeaderboardRepository
- [ ] Create LeaderboardService with caching
- [ ] Add death screen score submission
- [ ] Create leaderboard UI screens
- [ ] Implement filtering (daily/weekly/all-time)
- [ ] Add friend leaderboards (future)

### Phase 5: Telemetry & Analytics (Week 6)
- [ ] Implement TelemetryRepository
- [ ] Create event tracking system
- [ ] Add gameplay event logging
- [ ] Implement batch upload system
- [ ] Create analytics dashboard (web)
- [ ] Add opt-out mechanism

### Phase 6: Achievements (Week 7)
- [ ] Design achievement system
- [ ] Implement AchievementRepository
- [ ] Create achievement checking logic
- [ ] Add achievement UI
- [ ] Implement achievement notifications
- [ ] Create achievement statistics

### Phase 7: Testing & Polish (Week 8)
- [ ] Comprehensive integration testing
- [ ] Performance optimization
- [ ] Security audit
- [ ] Load testing
- [ ] Documentation
- [ ] Deployment preparation

## Security Considerations

### Authentication Security
- Use bcrypt or Argon2 for password hashing
- Implement rate limiting for login attempts
- Use secure session tokens (UUID v4)
- Implement session expiration
- Add two-factor authentication (future)

### Database Security
- Use prepared statements exclusively
- Implement SQL injection prevention
- Use least-privilege database users
- Enable SSL/TLS for connections
- Implement connection encryption

### Data Privacy
- GDPR compliance for EU users
- Allow data export/deletion
- Implement telemetry opt-out
- Anonymize telemetry data
- Regular security audits

## Performance Optimization

### Connection Pooling
```cpp
class ConnectionPool {
private:
    std::queue<std::shared_ptr<pqxx::connection>> available;
    std::set<std::shared_ptr<pqxx::connection>> inUse;
    std::mutex mutex;
    std::condition_variable cv;

    size_t minConnections = 2;
    size_t maxConnections = 10;

public:
    std::shared_ptr<pqxx::connection> acquire();
    void release(std::shared_ptr<pqxx::connection> conn);
};
```

### Caching Strategy
- Use Redis or in-memory cache for leaderboards
- Cache player profiles for 5 minutes
- Cache achievement definitions indefinitely
- Implement cache invalidation on updates

### Query Optimization
- Create appropriate indexes
- Use EXPLAIN ANALYZE for slow queries
- Implement query result pagination
- Batch telemetry inserts
- Use prepared statements

## Monitoring and Maintenance

### Monitoring
- Database connection pool metrics
- Query performance tracking
- Error rate monitoring
- Cache hit/miss ratios
- API response times

### Backup Strategy
- Daily automated backups
- Point-in-time recovery setup
- Regular backup testing
- Off-site backup storage
- Disaster recovery plan

### Maintenance Windows
- Schema migrations during off-peak
- Index rebuilding schedule
- Vacuum and analyze automation
- Log rotation setup
- Performance tuning reviews

## Configuration

### Environment Variables
```bash
# Database connection
VEYRM_DB_HOST=localhost
VEYRM_DB_PORT=5432
VEYRM_DB_NAME=veyrm_db
VEYRM_DB_USER=veyrm_admin
VEYRM_DB_PASSWORD=secure_password

# Connection pool
VEYRM_DB_MIN_CONNECTIONS=2
VEYRM_DB_MAX_CONNECTIONS=10
VEYRM_DB_CONNECTION_TIMEOUT=5000

# Feature flags
VEYRM_ENABLE_CLOUD_SAVES=true
VEYRM_ENABLE_LEADERBOARDS=true
VEYRM_ENABLE_TELEMETRY=false
VEYRM_ENABLE_ACHIEVEMENTS=true

# Cache settings
VEYRM_CACHE_TYPE=redis
VEYRM_CACHE_HOST=localhost
VEYRM_CACHE_PORT=6379
```

### Config File Integration
```yaml
database:
  enabled: true
  connection:
    host: ${VEYRM_DB_HOST}
    port: ${VEYRM_DB_PORT}
    database: ${VEYRM_DB_NAME}
    username: ${VEYRM_DB_USER}
    password: ${VEYRM_DB_PASSWORD}
  pool:
    min_connections: 2
    max_connections: 10
    connection_timeout: 5000
  features:
    cloud_saves: true
    leaderboards: true
    telemetry: false
    achievements: true
```

## Testing Strategy

### Unit Tests
- Mock database connections
- Test repository methods
- Validate SQL generation
- Test error handling
- Cache behavior testing

### Integration Tests
- Real database connections
- Transaction testing
- Connection pool testing
- Migration testing
- Performance benchmarks

### End-to-End Tests
- Full save/load cycle
- Leaderboard submission
- Authentication flow
- Achievement unlocking
- Telemetry collection

## Migration from Current System

### Save File Migration
1. Detect existing local saves
2. Prompt user to migrate to cloud
3. Parse existing save format
4. Convert to database schema
5. Upload to cloud storage
6. Maintain local backup

### Backwards Compatibility
- Support offline mode
- Local save fallback
- Version detection
- Gradual feature rollout
- Legacy save import

## Future Enhancements

### Phase 2 Features
- Social features (friends, guilds)
- Player messaging system
- Tournament system
- Replay system
- Spectator mode

### Advanced Analytics
- Player behavior analysis
- Difficulty balancing
- Content recommendations
- Churn prediction
- A/B testing framework

### Mobile Sync
- Cross-platform saves
- Mobile companion app
- Push notifications
- Remote play (future)

## Success Metrics

### Technical Metrics
- Database response time < 100ms
- 99.9% uptime
- Connection pool utilization < 80%
- Cache hit rate > 90%
- Error rate < 0.1%

### User Metrics
- Cloud save adoption rate
- Leaderboard participation
- Achievement completion rates
- Daily active users
- Session duration

## Risk Mitigation

### Technical Risks
- **Database outage**: Implement offline mode with sync
- **Data corruption**: Regular backups and validation
- **Performance issues**: Caching and query optimization
- **Security breach**: Encryption and regular audits

### User Experience Risks
- **Slow connections**: Async operations with UI feedback
- **Data loss**: Multiple backup strategies
- **Privacy concerns**: Clear opt-out mechanisms
- **Feature complexity**: Gradual rollout with tutorials

## Conclusion

This PostgreSQL integration will transform Veyrm from a standalone roguelike into a connected gaming experience with persistent progression, competitive leaderboards, and rich analytics. The phased approach ensures stable implementation while maintaining the core gameplay experience.

The architecture is designed for scalability, security, and performance, with clear separation of concerns and robust error handling. By following this plan, we can deliver a professional-grade online gaming experience while maintaining the simplicity and charm of the original game.