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

```text
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

### Lookup Tables (Normalized Reference Data)

#### 1. Tags Table

```sql
CREATE TABLE tags (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL,
    category VARCHAR(50) NOT NULL, -- 'monster', 'item', 'achievement', 'effect'
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_tags_name ON tags(name);
CREATE INDEX idx_tags_category ON tags(category);

-- Seed data examples
INSERT INTO tags (name, category) VALUES
    ('undead', 'monster'),
    ('flying', 'monster'),
    ('boss', 'monster'),
    ('cursed', 'item'),
    ('magical', 'item'),
    ('quest', 'item');
```

#### 2. Colors Table

```sql
CREATE TABLE colors (
    id SERIAL PRIMARY KEY,
    name VARCHAR(20) UNIQUE NOT NULL,
    hex_code VARCHAR(7),
    rgb_r SMALLINT,
    rgb_g SMALLINT,
    rgb_b SMALLINT,
    terminal_code VARCHAR(10)
);

-- Seed data
INSERT INTO colors (name, hex_code, rgb_r, rgb_g, rgb_b, terminal_code) VALUES
    ('red', '#FF0000', 255, 0, 0, '\033[31m'),
    ('green', '#00FF00', 0, 255, 0, '\033[32m'),
    ('blue', '#0000FF', 0, 0, 255, '\033[34m');
```

#### 3. Item Categories Table

```sql
CREATE TABLE item_categories (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL,
    parent_id INTEGER REFERENCES item_categories(id),
    glyph CHAR(1),
    default_color_id INTEGER REFERENCES colors(id),
    description TEXT
);

CREATE INDEX idx_item_cat_parent ON item_categories(parent_id);

-- Hierarchical categories
INSERT INTO item_categories (name, parent_id, glyph) VALUES
    ('weapon', NULL, ')'),
    ('sword', 1, ')'),
    ('axe', 1, ')'),
    ('armor', NULL, '['),
    ('consumable', NULL, '!');
```

#### 4. Damage Types Table

```sql
CREATE TABLE damage_types (
    id SERIAL PRIMARY KEY,
    name VARCHAR(30) UNIQUE NOT NULL,
    description TEXT
);

INSERT INTO damage_types (name) VALUES
    ('physical'), ('fire'), ('ice'), ('lightning'),
    ('poison'), ('holy'), ('dark'), ('psychic');
```

#### 5. Status Effects Table

```sql
CREATE TABLE status_effects (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL,
    description TEXT,
    is_debuff BOOLEAN DEFAULT false,
    default_duration INTEGER,
    stackable BOOLEAN DEFAULT false,
    max_stacks INTEGER DEFAULT 1
);

INSERT INTO status_effects (name, is_debuff, default_duration) VALUES
    ('poisoned', true, 10),
    ('blessed', false, 50),
    ('stunned', true, 2);
```

#### 6. Equipment Slots Table

```sql
CREATE TABLE equipment_slots (
    id SERIAL PRIMARY KEY,
    name VARCHAR(30) UNIQUE NOT NULL,
    display_order INTEGER NOT NULL
);

INSERT INTO equipment_slots (name, display_order) VALUES
    ('main_hand', 1),
    ('off_hand', 2),
    ('head', 3),
    ('chest', 4),
    ('legs', 5),
    ('feet', 6),
    ('ring_left', 7),
    ('ring_right', 8);
```

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

#### 7. Abilities Table (Shared Reference)

```sql
CREATE TABLE abilities (
    id SERIAL PRIMARY KEY,
    code VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    cooldown INTEGER DEFAULT 0,
    damage_type_id INTEGER REFERENCES damage_types(id) ON DELETE RESTRICT,
    damage_min INTEGER,
    damage_max INTEGER,
    range INTEGER DEFAULT 1,
    mana_cost INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_abilities_code ON abilities(code);
```

#### 8. Monsters Table (Normalized)

```sql
CREATE TABLE monsters (
    id SERIAL PRIMARY KEY,
    code VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    glyph CHAR(1) NOT NULL,
    color_id INTEGER REFERENCES colors(id) ON DELETE RESTRICT,
    base_hp INTEGER NOT NULL,
    base_attack INTEGER NOT NULL,
    base_defense INTEGER NOT NULL,
    base_speed INTEGER NOT NULL,
    base_xp INTEGER NOT NULL,
    threat_level CHAR(1) CHECK (threat_level IN ('a','b','c','d','e','f')),
    spawn_depth_min INTEGER DEFAULT 1,
    spawn_depth_max INTEGER DEFAULT 100,
    spawn_weight INTEGER DEFAULT 100,
    gold_drop_min INTEGER DEFAULT 0,
    gold_drop_max INTEGER DEFAULT 0,
    version VARCHAR(20) NOT NULL,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_monsters_code ON monsters(code);
CREATE INDEX idx_monsters_threat ON monsters(threat_level);
CREATE INDEX idx_monsters_depth ON monsters(spawn_depth_min, spawn_depth_max);
CREATE INDEX idx_monsters_active ON monsters(is_active);
```

#### 9. Monster Tags Junction Table

```sql
-- Junction table - no CASCADE on tag_id since tags are shared
CREATE TABLE monster_tags (
    monster_id INTEGER REFERENCES monsters(id) ON DELETE CASCADE,
    tag_id INTEGER REFERENCES tags(id) ON DELETE RESTRICT,
    PRIMARY KEY (monster_id, tag_id)
);

CREATE INDEX idx_monster_tags_monster ON monster_tags(monster_id);
CREATE INDEX idx_monster_tags_tag ON monster_tags(tag_id);
```

#### 10. Monster Abilities Junction Table

```sql
-- Links monsters to shared abilities
CREATE TABLE monster_abilities (
    monster_id INTEGER REFERENCES monsters(id) ON DELETE CASCADE,
    ability_id INTEGER REFERENCES abilities(id) ON DELETE RESTRICT,
    PRIMARY KEY (monster_id, ability_id)
);

CREATE INDEX idx_monster_abilities_monster ON monster_abilities(monster_id);
CREATE INDEX idx_monster_abilities_ability ON monster_abilities(ability_id);
```

#### 11. Monster Resistances Junction Table

```sql
CREATE TABLE monster_resistances (
    monster_id INTEGER REFERENCES monsters(id) ON DELETE CASCADE,
    damage_type_id INTEGER REFERENCES damage_types(id) ON DELETE RESTRICT,
    resistance_percent INTEGER CHECK (resistance_percent BETWEEN -100 AND 100),
    PRIMARY KEY (monster_id, damage_type_id)
);

CREATE INDEX idx_monster_resist_monster ON monster_resistances(monster_id);
```

#### 12. Items Table (Normalized)

```sql
CREATE TABLE items (
    id SERIAL PRIMARY KEY,
    code VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    category_id INTEGER REFERENCES item_categories(id) ON DELETE RESTRICT,
    glyph CHAR(1) NOT NULL,
    color_id INTEGER REFERENCES colors(id) ON DELETE RESTRICT,
    base_value INTEGER DEFAULT 0,
    weight DECIMAL(5,2) DEFAULT 1.0,
    stackable BOOLEAN DEFAULT false,
    max_stack INTEGER DEFAULT 1,
    rarity VARCHAR(20) DEFAULT 'common',
    min_depth INTEGER DEFAULT 1,
    max_depth INTEGER DEFAULT 100,
    version VARCHAR(20) NOT NULL,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_items_code ON items(code);
CREATE INDEX idx_items_category ON items(category_id);
CREATE INDEX idx_items_rarity ON items(rarity);
CREATE INDEX idx_items_depth ON items(min_depth, max_depth);
CREATE INDEX idx_items_active ON items(is_active);
```

#### 13. Item Tags Junction Table

```sql
CREATE TABLE item_tags (
    item_id INTEGER REFERENCES items(id) ON DELETE CASCADE,
    tag_id INTEGER REFERENCES tags(id) ON DELETE RESTRICT,
    PRIMARY KEY (item_id, tag_id)
);

CREATE INDEX idx_item_tags_item ON item_tags(item_id);
CREATE INDEX idx_item_tags_tag ON item_tags(tag_id);
```

#### 14. Item Properties Table

```sql
-- Properties specific to each item instance
CREATE TABLE item_properties (
    id SERIAL PRIMARY KEY,
    item_id INTEGER REFERENCES items(id) ON DELETE CASCADE,
    property_type VARCHAR(50) NOT NULL,
    property_value INTEGER NOT NULL,
    damage_type_id INTEGER REFERENCES damage_types(id) ON DELETE RESTRICT,
    status_effect_id INTEGER REFERENCES status_effects(id) ON DELETE RESTRICT,
    effect_duration INTEGER,
    effect_chance DECIMAL(3,2) DEFAULT 1.0
);

CREATE INDEX idx_item_props_item ON item_properties(item_id);
CREATE INDEX idx_item_props_type ON item_properties(property_type);
```

#### 15. Item Requirements Table

```sql
CREATE TABLE item_requirements (
    id SERIAL PRIMARY KEY,
    item_id INTEGER REFERENCES items(id) ON DELETE CASCADE,
    requirement_type VARCHAR(30) NOT NULL,
    requirement_value INTEGER,
    requirement_data VARCHAR(50)
);

CREATE INDEX idx_item_reqs_item ON item_requirements(item_id);
```

#### 16. Loot Tables (Normalized)

```sql
CREATE TABLE loot_tables (
    id SERIAL PRIMARY KEY,
    code VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    min_items INTEGER DEFAULT 1,
    max_items INTEGER DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE loot_table_entries (
    id SERIAL PRIMARY KEY,
    loot_table_id INTEGER REFERENCES loot_tables(id) ON DELETE CASCADE,
    item_id INTEGER REFERENCES items(id) ON DELETE RESTRICT,
    weight INTEGER DEFAULT 100,
    quantity_min INTEGER DEFAULT 1,
    quantity_max INTEGER DEFAULT 1,
    depth_modifier DECIMAL(3,2) DEFAULT 1.0,
    level_requirement INTEGER DEFAULT 0
);

CREATE INDEX idx_loot_entries_table ON loot_table_entries(loot_table_id);
CREATE INDEX idx_loot_entries_item ON loot_table_entries(item_id);
```

#### 17. Monster Loot Assignment

```sql
CREATE TABLE monster_loot (
    monster_id INTEGER REFERENCES monsters(id) ON DELETE CASCADE,
    loot_table_id INTEGER REFERENCES loot_tables(id) ON DELETE RESTRICT,
    drop_chance DECIMAL(3,2) DEFAULT 1.0,
    PRIMARY KEY (monster_id, loot_table_id)
);

CREATE INDEX idx_monster_loot_monster ON monster_loot(monster_id);
CREATE INDEX idx_monster_loot_table ON monster_loot(loot_table_id);
```

#### 18. Character Inventory Table (Normalized)

```sql
CREATE TABLE character_inventory (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    character_id UUID REFERENCES characters(id) ON DELETE CASCADE,
    item_id INTEGER REFERENCES items(id) ON DELETE RESTRICT,
    slot_letter CHAR(1),
    quantity INTEGER DEFAULT 1,
    is_equipped BOOLEAN DEFAULT false,
    equipment_slot_id INTEGER REFERENCES equipment_slots(id) ON DELETE RESTRICT,
    durability_current INTEGER,
    durability_max INTEGER,
    is_identified BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(character_id, slot_letter)
);

CREATE INDEX idx_inventory_character ON character_inventory(character_id);
CREATE INDEX idx_inventory_equipped ON character_inventory(character_id, is_equipped);
CREATE INDEX idx_inventory_item ON character_inventory(item_id);
CREATE INDEX idx_inventory_slot ON character_inventory(equipment_slot_id);
```

#### 19. Enchantments Table (Shared Reference)

```sql
CREATE TABLE enchantments (
    id SERIAL PRIMARY KEY,
    code VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(50) NOT NULL,
    description TEXT,
    prefix VARCHAR(30),
    suffix VARCHAR(30),
    value_modifier DECIMAL(3,2) DEFAULT 1.5,
    is_active BOOLEAN DEFAULT true
);

CREATE TABLE inventory_enchantments (
    inventory_id UUID REFERENCES character_inventory(id) ON DELETE CASCADE,
    enchantment_id INTEGER REFERENCES enchantments(id) ON DELETE RESTRICT,
    power_level INTEGER DEFAULT 1,
    PRIMARY KEY (inventory_id, enchantment_id)
);
```

#### 20. Monster Encounters Table

```sql
CREATE TABLE monster_encounters (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    character_id UUID REFERENCES characters(id) ON DELETE CASCADE,
    monster_id INTEGER REFERENCES monsters(id) ON DELETE SET NULL,
    monster_code VARCHAR(50) NOT NULL, -- Store code in case monster is deleted
    monster_name VARCHAR(100), -- Store name for historical record
    encounter_depth INTEGER NOT NULL,
    damage_dealt INTEGER DEFAULT 0,
    damage_taken INTEGER DEFAULT 0,
    was_killed BOOLEAN DEFAULT false,
    killed_by_player BOOLEAN DEFAULT false,
    encounter_duration INTEGER,
    turn_count INTEGER,
    experience_gained INTEGER DEFAULT 0,
    gold_dropped INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_encounters_character ON monster_encounters(character_id);
CREATE INDEX idx_encounters_monster ON monster_encounters(monster_id);
CREATE INDEX idx_encounters_date ON monster_encounters(created_at);
```

#### 21. Encounter Drops Table

```sql
CREATE TABLE encounter_drops (
    id SERIAL PRIMARY KEY,
    encounter_id UUID REFERENCES monster_encounters(id) ON DELETE CASCADE,
    item_id INTEGER REFERENCES items(id) ON DELETE SET NULL,
    item_code VARCHAR(50) NOT NULL, -- Store code for historical record
    quantity INTEGER DEFAULT 1,
    was_picked_up BOOLEAN DEFAULT false
);

CREATE INDEX idx_drops_encounter ON encounter_drops(encounter_id);
CREATE INDEX idx_drops_item ON encounter_drops(item_id);
```

#### 22. Telemetry Table

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

### Phase 1: Foundation (Week 1-2) ✅ **COMPLETED**

- [x] Set up CMake integration with ~~libpqxx~~ **libpq** (upgraded to official PostgreSQL library)
- [x] Implement DatabaseManager with connection pooling
- [x] Create base Repository and Service classes
- [x] Set up database migrations system (simplified with automatic schema management)
- [x] Create initial schema migrations
- [x] Write unit tests for database layer

**📅 Completed**: Current session
**🎯 Status**: All requirements met and tested
**🔧 Build Integration**: Database commands added to build.sh (`./build.sh db <command>`)
**🧪 Testing**: All 107 tests passing, database layer fully tested
**📦 Deliverables**: Complete PostgreSQL integration with ECS persistence system

### Phase 2: Authentication (Week 3) ✅ **COMPLETE**

- ✅ **Database Schema**: Designed 6 authentication tables with proper security
- ✅ **PlayerRepository**: Implemented comprehensive user account CRUD operations
- ✅ **AuthenticationService**: Created authentication service with SHA-256 password hashing
- ✅ **Session Management**: Implemented secure session handling with token rotation
- ✅ **UI Components**: Created login/registration screens with FTXUI
- ✅ **Remember Me**: Added persistent login functionality with refresh tokens
- ✅ **Security**: Account lockout, input validation, and security best practices
- ✅ **Testing**: Comprehensive authentication test suite with database mocking

**📋 Detailed Plan**: See [Phase 2 Authentication Plan](./phase2-authentication-plan.md) for complete implementation details, database schema, security considerations, and UI mockups.

### Phase 3: Save System Integration (Week 4) ✅ **COMPLETE**

- ✅ Implemented SaveGameRepository with full CRUD operations
- ✅ Modified existing save system to use database
- ✅ Added cloud save sync logic with background thread
- ✅ Created save conflict resolution strategies
- ✅ Implemented auto-save to cloud (5-minute intervals)
- ✅ Added offline mode fallback with queue system

**📅 Completed**: Current session
**🎯 Status**: All requirements met and tested
**🧪 Testing**: All 118 tests passing (1317 assertions)
**📦 Deliverables**: Complete cloud save system with PostgreSQL integration

**📋 Detailed Report**: See [Phase 3 Complete Summary](./phase3-complete-summary.md) for implementation details

### Phase 4: Authentication UI Implementation (Week 5) ✅ **COMPLETE**

- ✅ Integrated LoginScreen into MainMenuScreen flow
- ✅ Added authentication state indicators throughout UI
- ✅ Created account management screen (change password, etc.)
- ✅ Added session timeout warnings and refresh
- ✅ Implemented cloud save UI indicators in SaveLoadScreen
- ✅ Added sync progress and conflict resolution dialogs
- ✅ Created profile/statistics screen
- ✅ Added logout confirmation and cleanup

**📅 Completed**: Current session
**🎯 Status**: All UI components created and integrated
**🧪 Testing**: All 118 tests passing (1314 assertions)
**📦 Deliverables**: Complete authentication UI framework

**📋 Detailed Report**: See [Phase 4 UI Implementation Summary](./phase4-ui-implementation-summary.md) for implementation details

### Phase 5: Leaderboards (Week 6)

- [ ] Implement LeaderboardRepository
- [ ] Create LeaderboardService with caching
- [ ] Add death screen score submission
- [ ] Create leaderboard UI screens
- [ ] Implement filtering (daily/weekly/all-time)
- [ ] Add friend leaderboards (future)

### Phase 6: Telemetry & Analytics (Week 7)

- [ ] Implement TelemetryRepository
- [ ] Create event tracking system
- [ ] Add gameplay event logging
- [ ] Implement batch upload system
- [ ] Create analytics dashboard (web)
- [ ] Add opt-out mechanism

### Phase 7: Achievements (Week 8)

- [ ] Design achievement system
- [ ] Implement AchievementRepository
- [ ] Create achievement checking logic
- [ ] Add achievement UI
- [ ] Implement achievement notifications
- [ ] Create achievement statistics

### Phase 8: Testing & Polish (Week 9)

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

## Phase 1 Completion Summary ✅

**🎯 Phase 1: Foundation has been successfully completed!**

### What Was Delivered

**Core Infrastructure:**

- ✅ PostgreSQL integration using official `libpq` library (more stable than libpqxx)
- ✅ Robust DatabaseManager with connection pooling (1-10 connections)
- ✅ Automatic connection validation and reconnection
- ✅ Transaction management with proper rollback handling
- ✅ RAII wrappers for safe C API usage

**Schema & Data Management:**

- ✅ Complete normalized database schema (10+ tables)
- ✅ Automatic table creation with `CREATE IF NOT EXISTS`
- ✅ Initial data loading with `ON CONFLICT DO NOTHING`
- ✅ Smart data management: "If no data exists, load it; if data exists, use it"
- ✅ Proper foreign key relationships with CASCADE/RESTRICT policies

**ECS Integration:**

- ✅ PersistenceSystem integrated with ECS World
- ✅ Component serialization (Position, Health, Stats, Renderable, AI)
- ✅ Complete save/load functionality for game state
- ✅ Monster template and leaderboard persistence
- ✅ Telemetry event logging system

**Developer Tools:**

- ✅ Build system integration with `./build.sh db <command>` commands
- ✅ Database status checking and management
- ✅ Automatic compilation and execution of database tools
- ✅ Environment variable configuration (DB_HOST, DB_PORT, etc.)

**Testing & Quality:**

- ✅ Comprehensive unit test suite (DatabaseManager, PersistenceSystem)
- ✅ Integration tests with graceful degradation when database unavailable
- ✅ All 107 existing tests continue to pass
- ✅ Build verification on multiple configurations

### Usage Examples

```bash
# Database management
./build.sh db create   # Create tables
./build.sh db status   # Check connection and data
./build.sh db reset    # Clear and reload all data

# Development workflow
./build.sh build      # Builds with PostgreSQL support
./build.sh test       # Runs all tests including database tests
./build.sh run        # Game automatically sets up database if needed
```

### Key Features Delivered

1. **Production Ready**: Connection pooling, error handling, transaction management
2. **Developer Friendly**: Simple build commands, automatic setup, clear error messages
3. **Build Server Compatible**: Uses standard PostgreSQL library, no exotic dependencies
4. **Backward Compatible**: System works with or without database available
5. **Future Proof**: Extensible architecture ready for Phase 2-7 implementations

---

## Conclusion

This PostgreSQL integration will transform Veyrm from a standalone roguelike into a connected gaming experience with persistent progression, competitive leaderboards, and rich analytics. The phased approach ensures stable implementation while maintaining the core gameplay experience.

The architecture is designed for scalability, security, and performance, with clear separation of concerns and robust error handling. By following this plan, we can deliver a professional-grade online gaming experience while maintaining the simplicity and charm of the original game.

**✅ Phase 1 Foundation: COMPLETE**
**✅ Phase 2 Authentication: COMPLETE**
**✅ Phase 3 Save System Integration: COMPLETE**
**✅ Phase 4 Authentication UI Implementation: COMPLETE**
**🚀 Ready for Phase 5: Leaderboards!**

See [Phase 2 Completion Report](./phase2-completion-report.md) for full details.
